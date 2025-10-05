const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService
} = require("../controllers/overviewMachineDataController");

// GET all service records
router.get("/get-overview-machineData", getAllServices);

// // POST a new service record (form submission)
// router.post("/post3", createService);

module.exports = router;