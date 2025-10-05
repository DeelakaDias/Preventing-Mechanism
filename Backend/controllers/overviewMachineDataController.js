// src/controllers/downTimeController.js
const db = require("../models");
const overviewMachineData = db.overviewMachineDataModel

// Get all records
exports.getAllServices = async (req, res) => {
  try {
    const services = await overviewMachineData.findAll();
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
      brand,
      plant,
      lastServiceDate
    } = req.body; 

    const newService = await overviewMachineData.create({
      machineSerialNumber,
      brand,
      plant,
      lastServiceDate
    });

    res.status(201).json({ message: "Record added successfully", data: newService });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
};