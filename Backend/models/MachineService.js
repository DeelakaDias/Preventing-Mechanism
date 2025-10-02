// backend/models/MachineService.js
module.exports = (sequelize, DataTypes) => {
  const MachineService = sequelize.define("MachineService", {
    machineSerialNumber: {
      type: DataTypes.STRING,
      primaryKey: true,
      allowNull: false
    },
    lastServiceDate: {
      type: DataTypes.DATE,
      allowNull: true
    },
    brand: {
      type: DataTypes.STRING,
      allowNull: false
    },
    needleType: { 
      type: DataTypes.STRING,
      allowNull: true
    },
    fabricType: {
      type: DataTypes.STRING,
      allowNull: true
    },
    operation: {
      type: DataTypes.STRING,
      allowNull: true
    },
    styleNumber: {
      type: DataTypes.STRING,
      allowNull: true
    },
    plant: {
      type: DataTypes.STRING,
      allowNull: true
    },
    model: {
      type: DataTypes.STRING,
      allowNull: false
    }
  }, {
    tableName: "MBP_MachineData",
    timestamps: false
  });

  return MachineService;
};