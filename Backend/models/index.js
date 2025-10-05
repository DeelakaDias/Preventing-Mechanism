// src/models/index.js
const Sequelize = require("sequelize");
const sequelize = require("../config/db");

const db = {};
db.Sequelize = Sequelize;
db.sequelize = sequelize;

// Load models
db.MachineService = require("./MachineService")(sequelize, Sequelize);
db.downTimeEntry = require("./downTimeEntry")(sequelize, Sequelize);
db.overviewModel = require("./overviewModel")(sequelize, Sequelize);
db.overviewMachineDataModel = require("./overviewMachineDataModel")(sequelize, Sequelize);

module.exports = db;