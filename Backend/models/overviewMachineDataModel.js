module.exports = (sequelize, DataTypes) => {
  const overviewMachineData = sequelize.define("overviewMachineData", {
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
    plant: {
      type: DataTypes.STRING,
      allowNull: false
    }
  }, {
    tableName: "MBP_MachineData",
    timestamps: false
  });

  return overviewMachineData;
};