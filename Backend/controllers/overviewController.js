const db = require("../models");
const overviewController = db.overviewController

// Get all records
exports.getAllServices = async (req, res) => {
  try {
    const services = await overviewController.findAll();
    res.json(services);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
};

// Create new record (insert form data)
exports.createService = async (req, res) => {
  try {
    const {
      machineRPM,
      machineVibration,
      machineCurrent,
      needleRuntime
    } = req.body; 

    const newService = await overviewController.create({
      machineRPM,
      machineVibration,
      machineCurrent,
      needleRuntime
    });

    res.status(201).json({ message: "Record added successfully", data: newService });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
};