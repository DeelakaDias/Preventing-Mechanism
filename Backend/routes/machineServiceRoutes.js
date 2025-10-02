// src/routes/machineServiceRoutes.js
const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService
} = require("../controllers/machineServiceController");

// GET all service records
router.get("/", getAllServices);

// POST a new service record (form submission)
router.post("/", createService);

module.exports = router;