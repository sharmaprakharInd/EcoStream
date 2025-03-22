const apiEndpoint = 'https://2wc34iye0j.execute-api.eu-north-1.amazonaws.com/newstage/getsensordata'; 

async function fetchSensorData() {
  try {
    const response = await fetch(apiEndpoint);
    const data = await response.json();

    // Filter out data points where all values are zero
    const filteredData = data.filter(item =>
      item.temperature !== 0 || item.humidity !== 0 || item.gasLevel !== 0
    );

    // Extract valid data for plotting
    const timestamps = filteredData.map(item => {
      return new Date(item.timestamp * 1000).toLocaleString('en-US', { timeZone: 'Asia/Kolkata' });
    });
    const temperatures = filteredData.map(item => item.temperature || null);
    const humidities = filteredData.map(item => item.humidity || null);
    const gasLevels = filteredData.map(item => item.gasLevel || null);

    // Render the charts
    renderTemperatureChart(timestamps, temperatures);
    renderHumidityChart(timestamps, humidities);
    renderGasChart(timestamps, gasLevels);

    // Display statistics (optional)
    const avgTemp = calculateAverage(temperatures.filter(value => value !== null));
    const avgHumidity = calculateAverage(humidities.filter(value => value !== null));
    const avgGas = calculateAverage(gasLevels.filter(value => value !== null));

    document.getElementById('averages').innerText =
      `Average Temperature: ${avgTemp.toFixed(2)} °C\n` +
      `Average Humidity: ${avgHumidity.toFixed(2)} %\n` +
      `Average Gas Level: ${avgGas.toFixed(2)} %\n`;
  } catch (error) {
    console.error('Error fetching sensor data:', error);
    document.getElementById('averages').innerText = 'Failed to load data!';
  }
}

function calculateAverage(data) {
  return data.length > 0 ? data.reduce((a, b) => a + b, 0) / data.length : 0;
}

function renderTemperatureChart(timestamps, temperatures) {
  new Chart(document.getElementById('temperatureChart'), {
    type: 'line',
    data: {
      labels: timestamps,
      datasets: [{
        label: 'Temperature (°C)',
        data: temperatures,
        borderColor: 'red',
        borderWidth: 2,
        fill: false
      }]
    },
    options: {
      responsive: true,
      scales: {
        x: { title: { display: true, text: 'Time' } },
        y: { title: { display: true, text: 'Temperature (°C)' } }
      }
    }
  });
}

function renderHumidityChart(timestamps, humidities) {
  new Chart(document.getElementById('humidityChart'), {
    type: 'line',
    data: {
      labels: timestamps,
      datasets: [{
        label: 'Humidity (%)',
        data: humidities,
        borderColor: 'blue',
        borderWidth: 2,
        fill: false
      }]
    },
    options: {
      responsive: true,
      scales: {
        x: { title: { display: true, text: 'Time' } },
        y: { title: { display: true, text: 'Humidity (%)' } }
      }
    }
  });
}

function renderGasChart(timestamps, gasLevels) {
  new Chart(document.getElementById('gasChart'), {
    type: 'line',
    data: {
      labels: timestamps,
      datasets: [{
        label: 'Gas Level(%)',
        data: gasLevels,
        borderColor: 'green',
        borderWidth: 2,
        fill: false
      }]
    },
    options: {
      responsive: true,
      scales: {
        x: { title: { display: true, text: 'Time' } },
        y: { title: { display: true, text: 'Gas Level' } }
      }
    }
  });
}

// Fetch and display data
fetchSensorData();


