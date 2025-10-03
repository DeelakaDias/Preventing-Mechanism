const express = require("express");
const db = require("./models");

const app = express();
app.use(express.json()); // parse JSON form data
app.use(express.urlencoded({ extended: true })); // parse URL-encoded form data

// Sync database
db.sequelize.sync()
  .then(() => console.log("✅ Database synced"))
  .catch((err) => console.error("❌ DB sync error:", err));

// Routes
const machineServiceRoutes = require("./routes/machineServiceRoutes");
const downTimeRoutes = require("./routes/downTimeRoutes");
app.use("/api/machine-service", machineServiceRoutes);
app.use("/api/abc", downTimeRoutes);

module.exports = app;
