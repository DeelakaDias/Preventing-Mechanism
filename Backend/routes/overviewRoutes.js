const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService
} = require("../controllers/overviewController");

// GET all service records
router.get("/getOverview", getAllServices);

// POST a new service record (form submission)
router.post("/postOverview", createService);

module.exports = router;