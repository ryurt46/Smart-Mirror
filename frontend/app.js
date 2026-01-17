async function fetchClock() {
  const res = await fetch("/clock");
  const data = await res.json();

  const now = new Date();
  const h = now.getHours().toString().padStart(2, "0");
  const m = now.getMinutes().toString().padStart(2, "0");
  const s = now.getSeconds().toString().padStart(2, "0");

  const year = now.getFullYear();
  const month = (now.getMonth() + 1).toString().padStart(2, "0"); // 0-baserad
  const day = now.getDate().toString().padStart(2, "0");

  document.getElementById("current-day").textContent =
    `${data.current_day} ${year}-${month}-${day} ─ Vecka: ${data.week_number}`;

  document.getElementById("current-time").textContent = `${h}:${m}:${s}`;
}



async function fetchWeather() {
  const res = await fetch("/weather");
  const data = await res.json();

  const today = data.forecast?.[0];

  const iconEl = document.getElementById("weather-icon");

  const hour = new Date().getHours();
  const isDay = hour >= 6 && hour < 20;

  const codeMap = {
    1: isDay ? "wi-day-sunny.svg" : "wi-night-clear.svg",
    2: isDay ? "wi-day-cloudy.svg" : "wi-night-alt-partly-cloudy.svg",
    3: isDay ? "wi-cloudy.svg" : "wi-night-cloudy.svg",
    4: isDay ? "wi-snow.svg" : "wi-night-snow.svg",
    5: isDay ? "wi-rain.svg" : "wi-night-rain.svg",
    6: isDay ? "wi-sleet.svg" : "wi-night-sleet.svg",
    7: isDay ? "wi-thunderstorm.svg" : "wi-night-thunderstorm.svg",
    8: isDay ? "wi-fog.svg" : "wi-night-fog.svg",
    9: isDay ? "wi-day-sprinkle.svg" : "wi-night-sprinkle.svg",
  };

  if (today) {
    const code = today.weather_code;
    const iconFile =
      codeMap[code] || (isDay ? "wi-day-sunny.svg" : "wi-night-clear.svg");
    iconEl.src = `icons/${iconFile}`;

    const minTemp = today.min_temp.toFixed(1);
    const maxTemp = today.max_temp.toFixed(1);
    const wind = today.avg_wind?.toFixed(1);

    document.getElementById("today-weather").textContent =
      `Idag: ${minTemp}°C – ${maxTemp}°C${wind ? `, Vind ${wind} m/s` : ""}`;
  } else {
    document.getElementById("today-weather").textContent = "";
    iconEl.src = isDay ? "icons/wi-day-sunny.svg" : "icons/wi-night-clear.svg";
  }

  const forecastDiv = document.getElementById("forecast-week");
  forecastDiv.innerHTML = "";

  const weekdays = ["Söndag", "Måndag", "Tisdag", "Onsdag", "Torsdag", "Fredag", "Lördag"];

  data.forecast.slice(1).forEach((day) => {
    const [year, month, date] = day.date.split("-").map(Number);
    const dateObj = new Date(year, month - 1, date);
    const weekday = weekdays[dateObj.getDay()];
    const minTemp = day.min_temp.toFixed(1);
    const maxTemp = day.max_temp.toFixed(1);
    const d = document.createElement("div");
    d.className = "forecast-day";
    d.textContent = `${weekday} ${minTemp}°C – ${maxTemp}°C`;
    forecastDiv.appendChild(d);
  });
}

async function fetchDepartures() {
  const res = await fetch("/departures");
  const data = await res.json();

  const departuresDiv = document.getElementById("departures");
  departuresDiv.innerHTML = "";

  data.forEach((group) => {
    if (!group.departures || group.departures.length === 0) return;

    const title = document.createElement("div");
    title.className = "departure-title";
    title.textContent = group.name;
    departuresDiv.appendChild(title);
    departuresDiv.appendChild(document.createElement("hr"));

    group.departures.forEach((dep) => {
      const d = document.createElement("div");
      d.className = "departure-line";
      d.textContent = dep;
      if (
        dep.toLowerCase().includes("försenad") ||
        dep.toLowerCase().includes("delayed")
      ) {
        d.classList.add("delayed");
      }
      departuresDiv.appendChild(d);
    });
  });
}

async function initialize() {
  await fetchClock();
  await fetchWeather();
  await fetchDepartures();

  setInterval(fetchClock, 1000);
  setInterval(fetchWeather, 60000);
  setInterval(fetchDepartures, 30000);
}

initialize();
