// Dummy controller data
const dummyControllerData = [
    { Id: 1, machineSerialNumber: 'MSN001', machineRPM: 850, machineVibration: 2.3, machineCurrent: 15.2, needleRuntime: 247 },
    { Id: 2, machineSerialNumber: 'MSN002', machineRPM: 920, machineVibration: 1.8, machineCurrent: 14.5, needleRuntime: 300 },
    { Id: 3, machineSerialNumber: 'MSN003', machineRPM: 1500, machineVibration: 3.5, machineCurrent: 12.5, needleRuntime: 500 },
    { Id: 4, machineSerialNumber: 'MSN004', machineRPM: 1200, machineVibration: 2.0, machineCurrent: 14.0, needleRuntime: 300 },
    { Id: 5, machineSerialNumber: 'MSN001', machineRPM: 870, machineVibration: 2.5, machineCurrent: 15.5, needleRuntime: 260 },
    { Id: 6, machineSerialNumber: 'MSN002', machineRPM: 940, machineVibration: 1.9, machineCurrent: 14.7, needleRuntime: 320 },
    { Id: 7, machineSerialNumber: 'MSN003', machineRPM: 1480, machineVibration: 3.3, machineCurrent: 12.3, needleRuntime: 480 },
    { Id: 8, machineSerialNumber: 'MSN004', machineRPM: 1220, machineVibration: 2.1, machineCurrent: 14.2, needleRuntime: 310 },
    { Id: 9, machineSerialNumber: 'MSN001', machineRPM: 860, machineVibration: 2.4, machineCurrent: 15.3, needleRuntime: 255 },
    { Id: 10, machineSerialNumber: 'MSN002', machineRPM: 930, machineVibration: 1.85, machineCurrent: 14.6, needleRuntime: 310 }
];

// Navigate back to dashboard (preserve downtime entry state)
function goBackToDashboard() {
    console.log('Going back to dashboard, preserving downtime entry state');
    // Ensure the active tab is set to downtime-entry
    localStorage.setItem('activeTab', 'downtime-entry');
    // Navigate back
    window.location.href = 'MBP_IoT_dashboard.html';
}

// Load controller data and render charts
function loadControllerData() {
    const data = dummyControllerData;

    // Extract data for charts
    const ids = data.map(record => record.Id);
    const rpms = data.map(record => record.machineRPM);
    const vibrations = data.map(record => record.machineVibration);
    const currents = data.map(record => record.machineCurrent);

    // Create RPM Chart
    createChart('rpmChart', 'Machine RPM', ids, rpms, 'rgba(76, 175, 80, 0.6)', 'rgba(76, 175, 80, 1)');

    // Create Vibration Chart
    createChart('vibrationChart', 'Machine Vibration', ids, vibrations, 'rgba(255, 152, 0, 0.6)', 'rgba(255, 152, 0, 1)');

    // Create Current Chart
    createChart('currentChart', 'Machine Current (A)', ids, currents, 'rgba(33, 150, 243, 0.6)', 'rgba(33, 150, 243, 1)');
}

// Create chart function
function createChart(canvasId, label, xData, yData, backgroundColor, borderColor) {
    const ctx = document.getElementById(canvasId).getContext('2d');
    
    new Chart(ctx, {
        type: 'line',
        data: {
            labels: xData,
            datasets: [{
                label: label,
                data: yData,
                backgroundColor: backgroundColor,
                borderColor: borderColor,
                borderWidth: 3,
                tension: 0.4,
                fill: true,
                pointRadius: 6,
                pointHoverRadius: 8,
                pointBackgroundColor: borderColor,
                pointBorderColor: '#fff',
                pointBorderWidth: 2
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: true,
            plugins: {
                legend: {
                    display: true,
                    position: 'top',
                    labels: {
                        font: {
                            size: 14,
                            weight: 'bold'
                        },
                        padding: 20
                    }
                },
                tooltip: {
                    backgroundColor: 'rgba(0, 0, 0, 0.8)',
                    padding: 12,
                    titleFont: {
                        size: 14
                    },
                    bodyFont: {
                        size: 13
                    }
                }
            },
            scales: {
                x: {
                    title: {
                        display: true,
                        text: 'Record ID',
                        font: {
                            size: 14,
                            weight: 'bold'
                        }
                    },
                    grid: {
                        display: true,
                        color: 'rgba(0, 0, 0, 0.05)'
                    }
                },
                y: {
                    title: {
                        display: true,
                        text: label,
                        font: {
                            size: 14,
                            weight: 'bold'
                        }
                    },
                    beginAtZero: true,
                    grid: {
                        display: true,
                        color: 'rgba(0, 0, 0, 0.05)'
                    }
                }
            }
        }
    });
}

// Load data when page loads
document.addEventListener("DOMContentLoaded", loadControllerData);