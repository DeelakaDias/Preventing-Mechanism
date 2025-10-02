// src/config/db.js
const { Sequelize } = require("sequelize");

// Replace with your Azure SQL credentials
const sequelize = new Sequelize("KRESEAKREIOTPRD", "IOTAdmin", "oKuvodump5JNG7dM", {
  host: "kreseakreiotprdsrv.database.windows.net",
  dialect: "mssql",
  dialectOptions: {
    options: {
      encrypt: true, // Required for Azure
      trustServerCertificate: false
    }
  },
  logging: false
});

module.exports = sequelize;