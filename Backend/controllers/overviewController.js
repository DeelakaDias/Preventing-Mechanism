const db = require("../models");
const overviewController = db.overviewModel

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

// Get last 10 records for a specific machineSerialNumber
exports.getLatestRecordsByMachine = async (req, res) => {
  try {
    const { machineSerialNumber } = req.params;

    if (!machineSerialNumber) {
      return res.status(400).json({ error: "machineSerialNumber is required" });
    }

    const records = await overviewController.findAll({
      where: { machineSerialNumber },
      order: [["dateTime", "DESC"]], // latest first
      limit: 10 // get only last 10
    });

    res.status(200).json(records);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
};
