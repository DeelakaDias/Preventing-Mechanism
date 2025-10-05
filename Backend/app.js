const express = require("express");
const db = require("./models");
const cors = require("cors");

const app = express();
app.use(cors());
app.use(express.json()); // parse JSON form data
app.use(express.urlencoded({ extended: true })); // parse URL-encoded form data

// Sync database
db.sequelize.sync()
  .then(() => console.log("✅ Database synced"))
  .catch((err) => console.error("❌ DB sync error:", err));

// Routes
const machineServiceRoutes = require("./routes/machineServiceRoutes");
const downTimeRoutes = require("./routes/downTimeRoutes");
const overviewRoutes = require("./routes/overviewRoutes");
const overviewMachineDataRoutes = require("./routes/overviewMachineDataRoutes");
app.use("/api/machine-service", machineServiceRoutes);
app.use("/api/downTimeRoutes", downTimeRoutes);
app.use("/api/overview", overviewRoutes);
app.use("/api/overview-machineData", overviewMachineDataRoutes);

module.exports = app;