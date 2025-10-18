// backend/models/MachineService.js
module.exports = (sequelize, DataTypes) => {
  const downTimeEntry = sequelize.define("downTimeEntry", {
    Id: {
      type: DataTypes.STRING,
      primaryKey: true,
      autoIncrement: true,
      allowNull: true
    },

    machineSerialNumber: {
      type: DataTypes.STRING,
      allowNull: false
    },

    errorOccured: {
      type: DataTypes.STRING,
      allowNull: false
    },
    dateTime: {
      type: DataTypes.DATE,
      allowNull: true
    },

    MBP_Controller_ID: {
      type: DataTypes.STRING,
      allowNull: true
    },
    

  }, {
    tableName: "MBP_ErrorLogger",
    timestamps: false
  });

  return downTimeEntry;
};