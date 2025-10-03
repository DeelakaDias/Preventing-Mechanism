const express = require("express");
const router = express.Router();
const {
  getAllServices,
  createService
} = require("../controllers/overviewController");

// GET all service records
router.get("/get2", getAllServices);

// POST a new service record (form submission)
router.post("/post2", createService);

module.exports = router;