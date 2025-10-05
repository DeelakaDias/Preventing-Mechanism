// src/routes/machineServiceRoutes.js
const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService
} = require("../controllers/downTimeController");

// GET all service records
router.get("/getDownTimeRoutes", getAllServices);

// POST a new service record (form submission)
router.post("/postDownTimeRoutes", createService);

module.exports = router;