const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService,
  getLatestRecordsByMachine
} = require("../controllers/overviewController");

// GET all service records
router.get("/getOverview", getAllServices);

// POST a new service record (form submission)
router.post("/postOverview", createService);

// ✅ GET last 10 records for a given machine serial number
router.get("/getLatest/:machineSerialNumber", getLatestRecordsByMachine);

module.exports = router;