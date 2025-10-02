// src/controllers/machineServiceController.js
const db = require("../models");
const MachineService = db.MachineService;

// Get all records
exports.getAllServices = async (req, res) => {
  try {
    const services = await MachineService.findAll();
    res.json(services);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
};

// Create new record (insert form data)
exports.createService = async (req, res) => {
  try {
    const {
      machineSerialNumber,
      needleType,
      fabricType,
      operation,
      styleNumber,
      lastServiceDate,
      plant,
      brand,
      model
    } = req.body; 

    const newService = await MachineService.create({
      machineSerialNumber,
      lastServiceDate,
      needleType,
      fabricType,
      operation,
      styleNumber,
      plant,
      brand,
      model
    });

    res.status(201).json({ message: "Record added successfully", data: newService });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
};