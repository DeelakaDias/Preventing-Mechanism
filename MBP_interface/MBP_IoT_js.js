document.addEventListener('DOMContentLoaded', () => {
    // Initialize metrics to zero
    document.querySelector('.metric-value.rpm').textContent = 0;
    document.querySelector('.metric-value.vibration').textContent = 0;
    document.querySelector('.metric-value.current').textContent = 0;
    document.querySelector('.metric-value.runtime').textContent = 0;

    // Select all rows in the Machine Status table
    const machineRows = document.querySelectorAll('#machineStatusTable tr');

    machineRows.forEach(row => {
        row.addEventListener('click', () => {
            // Remove highlight from all rows
            machineRows.forEach(r => r.classList.remove('selected'));

            // Highlight clicked row
            row.classList.add('selected');

            // Update Overview metrics from clicked row's data attributes
            const rpm = row.getAttribute('data-rpm');
            const vibration = row.getAttribute('data-vibration');
            const current = row.getAttribute('data-current');
            const runtime = row.getAttribute('data-runtime');

            document.querySelector('.metric-value.rpm').textContent = rpm;
            document.querySelector('.metric-value.vibration').textContent = vibration;
            document.querySelector('.metric-value.current').textContent = current;
            document.querySelector('.metric-value.runtime').textContent = runtime;
        });
    });
});


// Tab switching functionality
function showTab(tabName) {
    // Hide all tab contents
    const tabContents = document.querySelectorAll('.tab-content');
    tabContents.forEach(tab => tab.classList.remove('active'));

    // Remove active class from all buttons
    const tabButtons = document.querySelectorAll('.tab-button');
    tabButtons.forEach(btn => btn.classList.remove('active'));

    // Show selected tab and mark button as active
    document.getElementById(tabName).classList.add('active');
    event.target.classList.add('active');
}

// Machine form submission
document.getElementById('machineForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const formData = new FormData(e.target);
    const machineData = Object.fromEntries(formData.entries());
    
    // Add timestamp
    machineData.dateTime = new Date().toISOString();
    
    // Show success message
    document.getElementById('machineAlert').innerHTML = 
        '<div class="alert alert-success">Machine information added successfully!</div>';
    
    // Reset form
    e.target.reset();
    
    // Clear alert after 3 seconds
    setTimeout(() => {
        document.getElementById('machineAlert').innerHTML = '';
    }, 3000);
});

// Downtime form submission
document.getElementById('downtimeForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const formData = new FormData(e.target);
    const downtimeData = Object.fromEntries(formData.entries());
    
    // Add to downtime table
    const tbody = document.getElementById('downtimeTable');
    const newRow = tbody.insertRow(0);
    newRow.innerHTML = `
        <td>${downtimeData.machineSerialNumber}</td>
        <td>${new Date(downtimeData.dateTime).toLocaleString()}</td>
        <td>${downtimeData.downTimeReasonCode}</td>
        <td>${downtimeData.downtimeReasonDescription}</td>
    `;
    
    // Show success message
    document.getElementById('downtimeAlert').innerHTML = 
        '<div class="alert alert-success">Downtime record added successfully!</div>';
    
    // Reset form
    e.target.reset();
    
    // Clear alert after 3 seconds
    setTimeout(() => {
        document.getElementById('downtimeAlert').innerHTML = '';
    }, 3000);
});

// IoT data simulation
// const machines = ['MSN001', 'MSN002', 'MSN003', 'MSN004'];
// let iotDataCounter = 0;

// function generateIoTData() {
//     const machine = machines[Math.floor(Math.random() * machines.length)];
//     const rpm = Math.floor(Math.random() * 300) + 700; // 700-1000 RPM
//     const vibration = (Math.random() * 5 + 1).toFixed(1); // 1-6 vibration
//     const current = (Math.random() * 10 + 10).toFixed(1); // 10-20 A
//     const runtime = Math.floor(Math.random() * 500) + 100; // 100-600 hours
    
//     return {
//         machineSerialNumber: machine,
//         machineRPM: rpm,
//         machineVibration: vibration,
//         current: current,
//         needleRuntime: runtime,
//         dateTime: new Date().toISOString()
//     };
// }

// function updateLiveMetrics() {
//     const data = generateIoTData();
//     document.getElementById('liveRPM').textContent = data.machineRPM;
//     document.getElementById('liveVibration').textContent = data.machineVibration;
//     document.getElementById('liveCurrent').textContent = data.current;
//     document.getElementById('liveRuntime').textContent = data.needleRuntime;
// }

// function addIoTDataRow() {
//     const data = generateIoTData();
//     const tbody = document.getElementById('iotDataTable');
    
//     // Add new row at the top
//     const newRow = tbody.insertRow(0);
//     newRow.innerHTML = `
//         <td>${data.machineSerialNumber}</td>
//         <td>${data.machineRPM}</td>
//         <td>${data.machineVibration}</td>
//         <td>${data.current}</td>
//         <td>${data.needleRuntime}</td>
//         <td>${new Date(data.dateTime).toLocaleString()}</td>
//     `;
    
//     // Keep only last 10 rows
//     while (tbody.rows.length > 10) {
//         tbody.deleteRow(-1);
//     }
    
//     iotDataCounter++;
// }

// // Initialize DOM content when loaded
// document.addEventListener('DOMContentLoaded', function() {
//     // Initialize with some dummy IoT data
//     for (let i = 0; i < 5; i++) {
//         addIoTDataRow();
//     }

//     // Update live metrics every 2 seconds
//     setInterval(updateLiveMetrics, 2000);

//     // Add new IoT data every 5 seconds
//     setInterval(addIoTDataRow, 5000);

//     // Initial metric update
//     updateLiveMetrics();
// });