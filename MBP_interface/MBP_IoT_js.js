// -------------------- TAB SWITCHING --------------------
function showTab(tabName) {
  document.querySelectorAll(".tab-content").forEach((tab) => tab.classList.remove("active"));
  document.querySelectorAll(".tab-button").forEach((btn) => btn.classList.remove("active"));

  document.getElementById(tabName).classList.add("active");

  const activeBtn = Array.from(document.querySelectorAll(".tab-button")).find((btn) =>
    btn.textContent.toLowerCase().includes(tabName.split("-")[0])
  );
  if (activeBtn) activeBtn.classList.add("active");

  localStorage.setItem("activeTab", tabName);
}

// -------------------- MACHINE FORM --------------------
document.getElementById("machineForm").addEventListener("submit", async (e) => {
  e.preventDefault();

  const form = e.target;
  const formData = new FormData(form);
  const machineData = Object.fromEntries(formData.entries());
  machineData.dateTime = new Date().toISOString();

  try {
    const res = await fetch("http://localhost:5000/api/machine-service", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(machineData),
    });

    const result = await res.json();
    const alertDiv = document.getElementById("machineAlert");

    if (res.ok) {
      alertDiv.innerHTML = `<div class="alert alert-success">${result.message || "Machine added successfully!"}</div>`;
      form.reset();
    } else {
      alertDiv.innerHTML = `<div class="alert alert-danger">Error: ${result.error || "Failed to add machine"}</div>`;
    }
  } catch (err) {
    document.getElementById("machineAlert").innerHTML = `<div class="alert alert-danger">Network error: ${err.message}</div>`;
  }

  setTimeout(() => (document.getElementById("machineAlert").innerHTML = ""), 3000);
});

// -------------------- DOWNTIME FORM --------------------
document.getElementById("downtimeForm").addEventListener("submit", async (e) => {
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

    const alertDiv = document.getElementById("downtimeAlert");
    if (response.ok) {
      alertDiv.innerHTML = `<div class="alert alert-success">${result.message || "Downtime recorded successfully!"}</div>`;
      form.reset();
    } else {
      alertDiv.innerHTML = `<div class="alert alert-danger">Error: ${result.error || "Failed to record downtime"}</div>`;
    }
  } catch (err) {
    document.getElementById("downtimeAlert").innerHTML = `<div class="alert alert-danger">Network error: ${err.message}</div>`;
  }

  setTimeout(() => (document.getElementById("downtimeAlert").innerHTML = ""), 3000);
});

// -------------------- FETCH RECORDS BUTTON --------------------
document.getElementById("fetchRecordsBtn").addEventListener("click", async () => {
  const serialNumber = document.getElementById("downtimeMachineSerial").value.trim();
  const dropdown = document.getElementById("controllerRecordId");

  if (!serialNumber) return alert("Please enter a Machine Serial Number first.");

  try {
    const res = await fetch(`http://localhost:5000/api/overview/getLatest/${serialNumber}`);
    if (!res.ok) throw new Error("Failed to fetch records");

    const records = await res.json();
    dropdown.innerHTML = `<option value="">Select Controller Record</option>`;

    if (records.length === 0) {
      dropdown.innerHTML += `<option disabled>No records found</option>`;
      return;
    }

    records.forEach((r) => {
      const option = document.createElement("option");
      option.value = r.Id;
      option.textContent = `ID: ${r.Id}`;
      dropdown.appendChild(option);
    });

    localStorage.setItem("controllerRecords", JSON.stringify(records));
    alert("Records fetched successfully!");
  } catch (err) {
    console.error(err);
    alert("Error fetching records. Check console for details.");
  }
});

// -------------------- CONTROLLER RECORDS PAGE NAVIGATION --------------------
function goToControllerRecords() {
  const form = document.getElementById("downtimeForm");
  const data = Object.fromEntries(new FormData(form).entries());
  localStorage.setItem("downtimeFormData", JSON.stringify(data));
  localStorage.setItem("activeTab", "downtime-entry");
  window.location.href = "controller_records.html";
}

// -------------------- ON LOAD --------------------
document.addEventListener("DOMContentLoaded", () => {
  const activeTab = localStorage.getItem("activeTab") || "machine-entry";
  showTab(activeTab);

  const savedData = localStorage.getItem("downtimeFormData");
  if (savedData) {
    const data = JSON.parse(savedData);
    for (const [key, value] of Object.entries(data)) {
      const input = document.querySelector(`[name='${key}']`);
      if (input) input.value = value;
    }
  }
});