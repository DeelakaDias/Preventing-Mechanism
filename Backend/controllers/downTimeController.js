// src/controllers/downTimeController.js
const db = require("../models");
const dd = db.downTimeEntry

// Get all records
exports.getAllServices = async (req, res) => {
  try {
    const services = await dd.findAll();
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
      dateTime,
      errorOccured
    } = req.body; 

    const newService = await dd.create({
      machineSerialNumber,
      dateTime,
      errorOccured
    });

    res.status(201).json({ message: "Record added successfully", data: newService });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
};