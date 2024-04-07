const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<html lang="en">
<head>
  <meta charset="UTF-8" />
  <title>Ember Alert</title>
  <style>
    body {
  display: flex;
  flex-direction: column;
  font-family: sans-serif;
  text-align: center;
  align-items: center;
  margin: 0;
  min-width: 400px;
  background-color: #9c2a33;
  font-size: 1em;
}

div {
  background-color: #212226;
  color: white;
  opacity: 80%;
  padding: 20px 20px 20px 20px;
  border-radius: 3px;
  margin-top: 4vh;
  box-shadow: 0 1px 4px black;
  width: 60%;
  align-items: center;
}

p {
  font-family: monospace;
  font-size: 1.5em;
}

#branding {
  height: 8vh;
  width: 100%;
  display: flex;
  justify-content: center;
  align-items: center;
  background-color: orange;
  margin: 0;
  font-size: 30px;
}

#hotspots {
  display: flex;
  flex-direction: column;
  justify-content: center;
  text-align: center;
  position: relative;
}

#hotspottable {
  display: flex;
  flex-direction: column;
  text-align: center;
  overflow-y: auto;
  height: 20vh;
  font-family: monospace;
  font-size: 1.5em;
  padding: 10px;
  width: 100%;
}

table {
  width: 100%;
}

th {
  border-bottom: 2px solid white;
  width: 50%;
}

td {
  width: 50%;
}

.button {
  display: inline-block;
  padding: 10px 20px;
  margin-top: 2vh;
  font-size: 16px;
  font-weight: bold;
  text-align: center;
  text-decoration: none;
  cursor: pointer;
  border: none;
  border-radius: 4px;
  background-color: orange;
}

.button:hover {
  background-color: #ff8c00;
}

.button:active,
.button:focus {
  outline: none;
  background-color: #cc5500;
}

  </style>
</head>
<body>
    <header id="branding">
        <h2>EMBER ALERT</h2>
      </header>
      
      <div>
        <h3> Live Temperature </h3>
        <p><span id="temperature">24.2</span> C</p>
      </div>
      
      <div>
        <h3> Current Location</h3>
        <p><span id="gps">49°56'21.0"N 119°23'42.3"W</span></p>
      </div>
      
      <div id="hotspots">
        <h3> Recorded Hotspots</h3>
        <table>
          <tr>
            <th>Location</th>
            <th>Temperature</th>
          </tr>
        </table>
        <div id="hotspottable">
          <table id="recordedData">
          </table>
          <p style="opacity:25%; text-align:center; width:100%; font-size:0.8em;">time go collect more data! ᕕ( ᐛ )ᕗ</p>
        </div>
        <button class="button" onclick="exportToExcel()">Export to Excel</button>
        <button class="button" onclick="clearTable()">Clear Table </button>
      </div>

      <script>
var currentLocation = 0;
var currentTemperature = 0;
var currentlat = 0;
var currentlng = 0;

var updateTab = 0;

function updateTemperature() {
  const temperatureText = document.getElementById("temperature");
  temperatureText.textContent = currentTemperature;
}

function updateLocation() {
  const locationText = document.getElementById("gps");
  locationText.textContent = currentLocation;
}

function updateTable() {
  const tableBody = document.getElementById("recordedData");
  const row = document.createElement("tr");
  row.innerHTML = `
    <td>${currentLocation}</td>
    <td>${currentTemperature} C</td>
  `;
  const firstRow = tableBody.firstChild;
  tableBody.insertBefore(row, firstRow);
}

function clearTable() {
  const tableBody = document.getElementById("recordedData");
  // Remove all rows from the table
  while (tableBody.firstChild) {
    tableBody.removeChild(tableBody.firstChild);
  }
}

function exportToExcel() {
  const table = document.getElementById("recordedData");
  const rows = table.querySelectorAll("tr");
  let csv = [];
  csv.push("Location, Temperature");
  for (let i = 0; i < rows.length; i++) {
    let row = [],
      cols = rows[i].querySelectorAll("td, th");
    for (let j = 0; j < cols.length; j++) row.push(cols[j].innerText);
    csv.push(row.join(","));
  }
  const csvData = csv.join("\n");
  // Create a download link
  const link = document.createElement("a");
  link.href = "data:text/csv;charset=utf-8," + encodeURIComponent(csvData);
  link.download = "Recorded-Hotspots.csv";
  link.style.display = "none";
  // Add the link to the DOM and trigger the click event
  document.body.appendChild(link);
  link.click();
  document.body.removeChild(link);
}

if (!!window.EventSource) {
  var source = new EventSource("/events");

  source.addEventListener(
    "open",
    function (e) {
      console.log("Events Connected");
    },
    false
  );
  source.addEventListener(
    "error",
    function (e) {
      if (e.target.readyState != EventSource.OPEN) {
        console.log("Events Disconnected");
      }
    },
    false
  );

  source.addEventListener(
    "message",
    function (e) {
      console.log("message", e.data);
    },
    false
  );

  source.addEventListener(
    "new_readings",
    function (e) {
      console.log("new_readings", e.data);
      var obj = JSON.parse(e.data);
      console.log("Parsed data:", obj); // Debugging
      currentTemperature = obj.temp;
      currentLocation = obj.coords;
      currentLat = obj.lat;
      currentLng = obj.lng;
      updateTab = obj.recordData;
      updateTemperature();
      updateLocation();
      if(updateTab == "1"){
        updateTable();
      }
    },
    false
  );
}
      </script>
</body>
</html>)rawliteral";