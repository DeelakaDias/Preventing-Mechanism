// backend/models/MachineService.js
module.exports = (sequelize, DataTypes) => {
  const MachineService = sequelize.define("MachineService", {
    machineSerialNumber: {
      type: DataTypes.STRING,
      primaryKey: true,
      allowNull: false
    },

    errorOccured: {
      type: DataTypes.STRING,
      allowNull: false
    },
    dateTime: {
      type: DataTypes.dateTime,
      allowNull: true
    },


  }, {
    tableName: "MBP_ErrorLogger",
    timestamps: false
  });

  return MachineService;
};