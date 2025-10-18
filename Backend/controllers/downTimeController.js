// src/controllers/downTimeController.js
const db = require("../models");
const downTimeEntry = db.downTimeEntry

// Get all records
exports.getAllServices = async (req, res) => {
  try {
    const services = await downTimeEntry.findAll();
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
      errorOccured,
      MBP_Controller_ID
    } = req.body; 

    const newService = await downTimeEntry.create({
      machineSerialNumber,
      dateTime,
      errorOccured,
      MBP_Controller_ID
    });

    res.status(201).json({ message: "Record added successfully", data: newService });
  } catch (err) {
    res.status(400).json({ error: err.message });
  }
};