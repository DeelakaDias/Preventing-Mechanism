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
