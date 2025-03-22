// Initialize Firebase (Replace with your Firebase config)
const firebaseConfig = {
    apiKey: "AIzaSyCkswTVYH0Kfw4fEIYnht-BdaSNRB--LR8",
    authDomain: "irrigreat-9ab11.firebaseapp.com",
    databaseURL: "https://irrigreat-9ab11-default-rtdb.firebaseio.com",
    projectId: "irrigreat-9ab11",
    storageBucket: "irrigreat-9ab11.firebasestorage.app",
    messagingSenderId: "717688942945",
    appId: "1:717688942945:web:58597ce517adb78942d520",
  };
  
  firebase.initializeApp(firebaseConfig);
  
  const database = firebase.database();
  const temperatureRef = database.ref("sensorData/temperature");
  const humidityRef = database.ref("sensorData/humidity");
  
  const temperatureData = [];
  const humidityData = [];
  const labels = [];
  
  const ctx = document.getElementById("myChart").getContext("2d");
  const myChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: labels,
      datasets: [
        {
          label: "Temperature (°C)",
          data: temperatureData,
          borderColor: "red",
          fill: false,
        },
        {
          label: "Humidity (%)",
          data: humidityData,
          borderColor: "blue",
          fill: false,
        },
      ],
    },
    options: {
      scales: {
        x: {
          type: 'time',
          time: {
              unit: 'minute'
          }
        }
      }
    }
  });
  
  temperatureRef.on("value", (snapshot) => {
    const value = snapshot.val();
    const time = new Date();
    temperatureData.push(value);
    labels.push(time);
    myChart.update();
  });
  
  humidityRef.on("value", (snapshot) => {
    const value = snapshot.val();
    humidityData.push(value);
    myChart.update();
  });