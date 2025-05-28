const express = require("express");
const http = require("http");
const { Server } = require("socket.io");
const { SerialPort } = require("serialport"); // Updated import for v10+
const { ReadlineParser } = require("@serialport/parser-readline"); // Updated package name

const app = express();
const server = http.createServer(app);
const io = new Server(server, {
  cors: {
    origin: "*", // Adjust this to your frontend URL in production
    methods: ["GET", "POST"]
  }
});

const SERIAL_PORT = "COM12"; // Verify this is correct in Device Manager
let port; // Declare port variable outside try-catch

try {
  // Initialize serial port
  port = new SerialPort({
    path: SERIAL_PORT,
    baudRate: 9600,
    autoOpen: true // This is default, but explicit is better
  });

  const parser = port.pipe(new ReadlineParser({ delimiter: '\r\n' }));

  // Handle serial port errors
  port.on('error', (err) => {
    console.error('SerialPort error:', err.message);
  });

  io.on("connection", (socket) => {
    console.log("New client connected");

    parser.on("data", (data) => {
      try {
        // First try to parse as JSON
        try {
          const jsonData = JSON.parse(data);
          socket.emit("sensorData", jsonData);
        } catch (jsonError) {
          // If not JSON, send raw data with timestamp
          socket.emit("sensorData", {
            raw: data.trim(),
            timestamp: new Date().toISOString()
          });
        }
      } catch (error) {
        console.error("Data handling error:", error);
      }
    });

    socket.on("disconnect", () => {
      console.log("Client disconnected");
    });
  });

  const PORT = 3001;
  server.listen(PORT, () => {
    console.log(`Server running on http://localhost:${PORT}`);
  });

} catch (portError) {
  console.error("Failed to initialize SerialPort:", portError.message);
}