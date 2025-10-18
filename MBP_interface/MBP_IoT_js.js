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

// 🚀 Machine form submission
document.getElementById('machineForm').addEventListener('submit', async function (e) {
    e.preventDefault();

    const form = e.target;
    const formData = new FormData(form);
    const machineData = Object.fromEntries(formData.entries());
    machineData.dateTime = new Date().toISOString(); // add timestamp

    try {
        const response = await fetch("http://localhost:5000/api/machine-service", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(machineData),
        });

        const result = await response.json();

        if (response.ok) {
            document.getElementById("machineAlert").innerHTML =
                `<div class="alert alert-success">${result.message || "Machine information added successfully!"}</div>`;

            // ✅ Clear form fields after successful submission
            form.reset();
        } else {
            document.getElementById("machineAlert").innerHTML =
                `<div class="alert alert-danger">Error: ${result.error || "Failed to add machine"}</div>`;
        }
    } catch (err) {
        document.getElementById("machineAlert").innerHTML =
            `<div class="alert alert-danger">Network error: ${err.message}</div>`;
    }

    // ⏳ Clear alert message after 3 seconds
    setTimeout(() => {
        document.getElementById("machineAlert").innerHTML = "";
    }, 3000);
});


// 🚀 Downtime form submission
document.getElementById('downtimeForm').addEventListener('submit', async function (e) {
    e.preventDefault();

    const form = e.target;
    const formData = new FormData(form);
    const downtimeData = Object.fromEntries(formData.entries());

    try {
        const response = await fetch("http://localhost:5000/api/downTimeRoutes/postDownTimeRoutes", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify(downtimeData),
        });

        const result = await response.json();

        if (response.ok) {
            // ✅ Add a new row dynamically (optional)
            const tbody = document.getElementById("downtimeTable");
            if (tbody) {
                const newRow = tbody.insertRow(0);
                newRow.innerHTML = `
                    <td>${downtimeData.machineSerialNumber}</td>
                    <td>${new Date(downtimeData.dateTime).toLocaleString()}</td>
                    <td>${downtimeData.errorOccured}</td>
                    <td>—</td>
                `;
            }

            document.getElementById("downtimeAlert").innerHTML =
                `<div class="alert alert-success">${result.message || "Downtime record added successfully!"}</div>`;

            // ✅ Clear form fields after successful submission
            form.reset();
        } else {
            document.getElementById("downtimeAlert").innerHTML =
                `<div class="alert alert-danger">Error: ${result.error || "Failed to record downtime"}</div>`;
        }
    } catch (err) {
        document.getElementById("downtimeAlert").innerHTML =
            `<div class="alert alert-danger">Network error: ${err.message}</div>`;
    }

    // ⏳ Clear alert message after 3 seconds
    setTimeout(() => {
        document.getElementById("downtimeAlert").innerHTML = "";
    }, 3000);
});


// Fetch and populate downtime records
async function loadDowntimeRecords() {
    try {
        const response = await fetch("http://localhost:5000/api/downTimeRoutes/getDownTimeRoutes");
        const data = await response.json();

        const tbody = document.getElementById("downtimeTable");
        tbody.innerHTML = ""; // clear old static rows

        data.forEach(record => {
            const row = document.createElement("tr");
            row.innerHTML = `
                <td>${record.machineSerialNumber}</td>
                <td>${new Date(record.dateTime).toLocaleDateString()}</td>
                <td>${record.errorOccured}</td>
            `;
            tbody.appendChild(row);
        });

    } catch (error) {
        console.error("Error loading downtime records:", error);
    }
}

// Call it when page loads
document.addEventListener("DOMContentLoaded", loadDowntimeRecords);

// Dummy data for testing
let dummyDowntimeRecords = [
    {
        machineSerialNumber: 'MSN001',
        dateTime: '2025-01-10',
        controllerRecordId: 3,
        errorOccured: 'Belt replacement required'
    },
    {
        machineSerialNumber: 'MSN003',
        dateTime: '2025-01-09',
        controllerRecordId: 7,
        errorOccured: 'Motor overheating'
    },
    {
        machineSerialNumber: 'MSN002',
        dateTime: '2025-01-08',
        controllerRecordId: 2,
        errorOccured: 'Needle broke during operation'
    }
];

// Tab switching functionality
function showTab(tabName) {
    const tabContents = document.querySelectorAll('.tab-content');
    tabContents.forEach(tab => tab.classList.remove('active'));

    const tabButtons = document.querySelectorAll('.tab-button');
    tabButtons.forEach(btn => btn.classList.remove('active'));

    document.getElementById(tabName).classList.add('active');
    
    // Find and activate the corresponding button
    const buttons = document.querySelectorAll('.tab-button');
    buttons.forEach(btn => {
        if (btn.textContent.toLowerCase().includes(tabName.replace('-', ' '))) {
            btn.classList.add('active');
        }
    });
    
    // Save active tab to localStorage
    localStorage.setItem('activeTab', tabName);
}

// Save form data to localStorage
function saveDowntimeFormData() {
    const machineSerial = document.getElementById('downtimeMachineSerial');
    const dateTime = document.getElementById('downTimeDateTime');
    const controllerId = document.getElementById('controllerRecordId');
    const errorDesc = document.getElementById('errorOccured');
    
    if (machineSerial && dateTime && controllerId && errorDesc) {
        const formData = {
            machineSerialNumber: machineSerial.value,
            dateTime: dateTime.value,
            controllerRecordId: controllerId.value,
            errorOccured: errorDesc.value
        };
        localStorage.setItem('downtimeFormData', JSON.stringify(formData));
        console.log('Form data saved:', formData);
    }
}

// Restore form data from localStorage
function restoreDowntimeFormData() {
    const savedData = localStorage.getItem('downtimeFormData');
    console.log('Restoring form data:', savedData);
    
    if (savedData) {
        try {
            const formData = JSON.parse(savedData);
            
            const machineSerial = document.getElementById('downtimeMachineSerial');
            const dateTime = document.getElementById('downTimeDateTime');
            const controllerId = document.getElementById('controllerRecordId');
            const errorDesc = document.getElementById('errorOccured');
            
            if (machineSerial) machineSerial.value = formData.machineSerialNumber || '';
            if (dateTime) dateTime.value = formData.dateTime || '';
            if (controllerId) controllerId.value = formData.controllerRecordId || '';
            if (errorDesc) errorDesc.value = formData.errorOccured || '';
            
            console.log('Form data restored successfully');
        } catch (e) {
            console.error('Error restoring form data:', e);
        }
    }
}

// Clear form data from localStorage
function clearDowntimeFormData() {
    localStorage.removeItem('downtimeFormData');
    console.log('Form data cleared');
}

// Navigate to Controller Records page
function goToControllerRecords() {
    // Save form data before leaving
    saveDowntimeFormData();
    // Save that we're coming from downtime entry
    localStorage.setItem('activeTab', 'downtime-entry');
    // Navigate to controller records page
    window.location.href = 'controller_records.html';
}

// Add event listeners to save form data on input change
function addDowntimeFormAutoSave() {
    const form = document.getElementById('downtimeForm');
    if (form) {
        const inputs = form.querySelectorAll('input, select, textarea');
        inputs.forEach(input => {
            input.addEventListener('change', saveDowntimeFormData);
            input.addEventListener('input', saveDowntimeFormData);
            input.addEventListener('blur', saveDowntimeFormData);
        });
        console.log('Auto-save listeners added to', inputs.length, 'inputs');
    }
}

// Machine form submission (with dummy data)
if (document.getElementById('machineForm')) {
    document.getElementById('machineForm').addEventListener('submit', function (e) {
        e.preventDefault();

        const form = e.target;
        const formData = new FormData(form);
        const machineData = Object.fromEntries(formData.entries());
        
        console.log('Machine Data Submitted:', machineData);

        document.getElementById("machineAlert").innerHTML =
            '<div class="alert alert-success">Machine information added successfully! (Dummy mode)</div>';
        
        form.reset();

        setTimeout(() => {
            document.getElementById("machineAlert").innerHTML = "";
        }, 3000);
    });
}

// Downtime form submission (with dummy data)
if (document.getElementById('downtimeForm')) {
    document.getElementById('downtimeForm').addEventListener('submit', function (e) {
        e.preventDefault();

        const form = e.target;
        const formData = new FormData(form);
        const downtimeData = Object.fromEntries(formData.entries());
        
        console.log('Downtime Data Submitted:', downtimeData);

        // Add to dummy data array
        dummyDowntimeRecords.unshift({
            machineSerialNumber: downtimeData.machineSerialNumber,
            dateTime: downtimeData.dateTime,
            controllerRecordId: parseInt(downtimeData.controllerRecordId),
            errorOccured: downtimeData.errorOccured
        });

        // Save to localStorage
        localStorage.setItem('dummyDowntimeRecords', JSON.stringify(dummyDowntimeRecords));

        // Reload the table
        loadDowntimeRecords();

        document.getElementById("downtimeAlert").innerHTML =
            '<div class="alert alert-success">Downtime record added successfully!</div>';
        
        // Clear form and localStorage ONLY after successful submission
        form.reset();
        clearDowntimeFormData();

        setTimeout(() => {
            document.getElementById("downtimeAlert").innerHTML = "";
        }, 3000);
    });
}

// Load downtime records from localStorage or dummy data
function loadDowntimeRecords() {
    const tbody = document.getElementById("downtimeTable");
    if (!tbody) return;
    
    // Load from localStorage if available
    const savedRecords = localStorage.getItem('dummyDowntimeRecords');
    if (savedRecords) {
        dummyDowntimeRecords = JSON.parse(savedRecords);
    }
    
    tbody.innerHTML = "";

    if (dummyDowntimeRecords.length === 0) {
        tbody.innerHTML = '<tr><td colspan="4" style="text-align:center;">No downtime records</td></tr>';
        return;
    }

    dummyDowntimeRecords.forEach(record => {
        const row = document.createElement("tr");
        row.innerHTML = `
            <td>${record.machineSerialNumber}</td>
            <td>${record.dateTime}</td>
            <td>${record.controllerRecordId}</td>
            <td>${record.errorOccured}</td>
        `;
        tbody.appendChild(row);
    });
}

// Initialize on page load
document.addEventListener("DOMContentLoaded", function() {
    console.log('Page loaded, initializing...');
    
    // Load downtime records
    loadDowntimeRecords();
    
    // Check if we should show the downtime entry tab (default or coming back from controller records)
    const activeTab = localStorage.getItem('activeTab');
    console.log('Active tab from storage:', activeTab);
    
    if (activeTab === 'downtime-entry') {
        // Show downtime entry tab
        showTab('downtime-entry');
        // Restore form data after a short delay to ensure DOM is ready
        setTimeout(() => {
            restoreDowntimeFormData();
        }, 100);
    } else if (activeTab === 'machine-entry') {
        showTab('machine-entry');
    } else {
        // Default to machine-entry tab on first load
        showTab('machine-entry');
    }
    
    // Add auto-save functionality to downtime form
    addDowntimeFormAutoSave();
    
    console.log('Initialization complete');
});


document.getElementById("fetchRecordsBtn").addEventListener("click", async () => {
  const serialNumberInput = document.getElementById("downtimeMachineSerial");
  const dropdown = document.getElementById("controllerRecordId");
  const machineSerial = serialNumberInput.value.trim();

  if (!machineSerial) {
    alert("Please enter a Machine Serial Number first.");
    return;
  }

  try {
    const response = await fetch(`http://localhost:5000/api/overview/getLatest/${machineSerial}`);
    if (!response.ok) throw new Error("Failed to fetch records");

    const records = await response.json();

    // Reset dropdown
    dropdown.innerHTML = `<option value="">Select Controller Record</option>`;

    if (records.length === 0) {
      const option = document.createElement("option");
      option.textContent = "No records found";
      option.disabled = true;
      dropdown.appendChild(option);
      return;
    }

    // Populate dropdown with IDs
    records.forEach(record => {
      const option = document.createElement("option");
      option.value = record.Id;
      option.textContent = `ID: ${record.Id}`;
      dropdown.appendChild(option);
    });

    // ✅ Store records in localStorage for graph page
    localStorage.setItem("controllerRecords", JSON.stringify(records));
    console.log("Stored last 10 records in localStorage:", records);

    alert("Records fetched successfully! You can now view them in Controller Records.");

  } catch (err) {
    console.error(err);
    alert("Error fetching records. Check console for details.");
  }
});