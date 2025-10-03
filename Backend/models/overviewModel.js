module.exports = (sequelize, DataTypes) => {
  const overviewController = sequelize.define("overviewController", {
    Id: {
      type: DataTypes.INTEGER,
      primaryKey: true,
      autoIncrement: true,
      allowNull: false
    },
    machineSerialNumber: {
      type: DataTypes.STRING,
      allowNull: false
    },
    machineRPM: {
      type: DataTypes.STRING,
      allowNull: false
    },
    machineVibration: {
      type: DataTypes.FLOAT,
      allowNull: true
    },
    machineCurrent: {
      type: DataTypes.FLOAT,
      allowNull: true
    },
    needleRuntime: {
      type: DataTypes.FLOAT,
      allowNull: true
    }
  }, {
    tableName: "MBP_ControllerData",
    timestamps: false
  });

  return overviewController;
};
