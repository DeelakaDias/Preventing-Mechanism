module.exports = (sequelize, DataTypes) => {
  const overviewController = sequelize.define("overviewController", {
    Id: {
      type: DataTypes.INTEGER,
      primaryKey: true,
      autoIncrement: true,
      allowNull: false
    },
    machineSerialNumber: {
      type: DataTypes.STRING(100),
      allowNull: false
    },
    machineRPM: {
      type: DataTypes.DECIMAL(10, 2),
      allowNull: false
    },
    machineVibration: {
      type: DataTypes.STRING(1000),
      allowNull: false
    },
    machineCurrent: {
      type: DataTypes.DECIMAL(10, 2),
      allowNull: false
    },
    needleRuntime: {
      type: DataTypes.DECIMAL(10, 2),
      allowNull: false
    },
    dateTime: {
      type: DataTypes.DATE, // Sequelize will handle datetime2 as DATE
      allowNull: false
    },
    status: {
      type: DataTypes.STRING(100),
      allowNull: true
    }
  }, {
    tableName: "MBP_ControllerData",
    timestamps: false
  });

  return overviewController;
};