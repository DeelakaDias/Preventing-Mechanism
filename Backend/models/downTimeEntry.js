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

    solutionGiven: {
      type: DataTypes.STRING,
      allowNull: false
    },
    dateTime: {
      type: DataTypes.DATE,
      allowNull: false
    },

    MBP_Controller_ID: {
      type: DataTypes.STRING,
      allowNull: false
    },

    errorCode: {
      type: DataTypes.STRING,
      allowNull: false
    } 

  }, {
    tableName: "MBP_ErrorLogger",
    timestamps: false
  });

  return downTimeEntry;
};