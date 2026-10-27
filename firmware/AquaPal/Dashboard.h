#pragma once

// Served directly by the ESP32. No CDN, external assets, internet, or tracking.
static const char DASHBOARD_HTML[] PROGMEM = R"AQUAPALHTML(
<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>AquaPal | Water Bottle</title>
<style>
:root{color-scheme:dark;font-family:system-ui,-apple-system,Segoe UI,sans-serif;background:#10151d;color:#ebf6ff}
*{box-sizing:border-box}body{margin:0;padding:34px 18px;background:radial-gradient(ellipse at top,#173b47 0%,#10151d 62%);min-height:100vh}
main{max-width:820px;margin:auto}header{display:flex;justify-content:space-between;align-items:center;gap:12px;margin-bottom:25px}
h1{margin:0;letter-spacing:-1.4px;font-size:34px}p{color:#a6bac5;margin:5px 0 0}.pill{background:#183f42;color:#9debd5;border:1px solid #2d7569;border-radius:100px;padding:7px 12px;font-size:12px}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:13px}.card{background:#1d2733;border:1px solid #34414d;border-radius:17px;padding:20px}
.card.wide{grid-column:1/-1}.tag{font-size:13px;color:#a6bac5}.number{font-size:32px;font-weight:730;letter-spacing:-1px;margin-top:9px}.unit{font-size:14px;font-weight:400;color:#a6bac5}.track{margin:16px 0 8px;background:#344552;border-radius:20px;height:11px;overflow:hidden}.fill{width:0%;height:100%;background:linear-gradient(90deg,#16b5b7,#79dec2);transition:width .3s}
.row{display:flex;justify-content:space-between;align-items:center;gap:12px}.hint{font-size:12px;color:#91a4b2}label{display:block;color:#a8bbc5;font-size:13px;margin-top:12px}input{width:100%;padding:12px;background:#101923;border:1px solid #486071;color:#eef;border-radius:9px;margin-top:5px}button{background:#69d6be;color:#092a2e;font-weight:700;padding:12px 20px;border:0;border-radius:10px;margin-top:16px;cursor:pointer}.notice{font-size:13px;color:#b8cad3;margin-top:12px}a{color:#80d9cf}@media(max-width:560px){.grid{grid-template-columns:1fr}.card.wide{grid-column:auto}h1{font-size:30px}body{padding:20px 13px}}
</style>
</head>
<body><main>
<header><div><h1>💧 AquaPal</h1><p>Smart water bottle · Local dashboard</p></div><span class="pill" id="status">Connecting…</span></header>
<div class="grid">
<section class="card wide"><div class="row"><span class="tag">Daily water intake (estimated)</span><span class="tag" id="clock">--:--</span></div>
<div class="number"><span id="consumed">--</span> <span class="unit">mL / <span id="goal">--</span> mL</span></div><div class="track"><div id="progress" class="fill"></div></div><div class="hint" id="progressText">Waiting for sensor…</div></section>
<section class="card"><div class="tag">Water remaining</div><div class="number"><span id="level">--</span> <span class="unit">mL</span></div><div class="hint">Estimated from ultrasonic distance</div></section>
<section class="card"><div class="tag">Next reminder</div><div class="number" id="reminder">--</div><div class="hint" id="reminderHint">Since last recorded intake</div></section>
<section class="card"><div class="tag">Room temperature</div><div class="number"><span id="temp">--</span> <span class="unit">°C</span></div></section>
<section class="card"><div class="tag">Humidity</div><div class="number"><span id="humidity">--</span> <span class="unit">%</span></div></section>
<section class="card wide"><div class="tag">Settings</div><form id="settings"><label>Daily goal (mL)<input id="goalInput" type="number" min="250" max="5000" required></label>
<label>Reminder interval (minutes)<input id="intervalInput" type="number" min="5" max="180" required></label>
<button type="submit">Save settings</button><span class="notice" id="feedback"></span></form></section>
</div><p class="notice">Approximate sensor readings, not a medical hydration device. Keep bottle upright during measurements.</p>
</main>
<script>
let formDirty=false;const byId=id=>document.getElementById(id);
function put(id,value){byId(id).textContent=value}
async function refresh(){try{const r=await fetch('/api/status',{cache:'no-store'});if(!r.ok)throw Error('Offline');const v=await r.json();
put('status',v.sensorReady?'Sensor connected':'Waiting for sensor');put('consumed',v.consumedMl);put('goal',v.goalMl);
put('level',v.sensorReady?v.waterMl:'--');put('temp',v.temperatureC===null?'--':v.temperatureC.toFixed(1));put('humidity',v.humidity===null?'--':v.humidity.toFixed(0));
put('clock',v.time||'RTC unavailable');put('reminder',v.reminderDue?'Drink water':v.nextReminderMin+' min');
put('reminderHint',v.reminderDue?'A reminder is due':'Until the next reminder');
const p=Math.min(100,Math.round(100*v.consumedMl/Math.max(v.goalMl,1)));byId('progress').style.width=p+'%';put('progressText',p+'% of daily goal');
if(!formDirty){byId('goalInput').value=v.goalMl;byId('intervalInput').value=v.reminderIntervalMin}
}catch(e){put('status','Device offline')}}
byId('settings').addEventListener('input',()=>formDirty=true);
byId('settings').addEventListener('submit',async e=>{e.preventDefault();try{const body=new URLSearchParams({goalMl:byId('goalInput').value,reminderMinutes:byId('intervalInput').value});const r=await fetch('/api/settings',{method:'POST',body});put('feedback',r.ok?' Saved.':' Please check the values.');formDirty=false;refresh()}catch(e){put('feedback',' Could not save settings.')}});
refresh();setInterval(refresh,3000);
</script></body></html>
)AQUAPALHTML";
