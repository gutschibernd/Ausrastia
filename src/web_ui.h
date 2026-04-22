#pragma once
#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"HTML(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,user-scalable=no">
<title>Ausrastia</title>
<style>
  :root{--bg:#0b0b12;--fg:#eaeaf2;--mut:#8a8aa0;--acc:#ff2e88;--acc2:#00e1ff;--card:#15151f;--bd:#24243a}
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent}
  html,body{margin:0;padding:0;background:var(--bg);color:var(--fg);font:15px/1.4 -apple-system,Segoe UI,Roboto,sans-serif;overscroll-behavior:none}
  header{padding:18px 16px 8px;display:flex;align-items:center;justify-content:space-between}
  header h1{margin:0;font-size:20px;letter-spacing:.4px;background:linear-gradient(90deg,var(--acc),var(--acc2));-webkit-background-clip:text;background-clip:text;color:transparent}
  .dot{width:8px;height:8px;border-radius:50%;background:#f33;display:inline-block;margin-right:6px;vertical-align:middle}
  .dot.on{background:#2ecc71;box-shadow:0 0 8px #2ecc71}
  .status{font-size:12px;color:var(--mut)}
  main{padding:8px 14px 120px;max-width:720px;margin:0 auto}
  .card{background:var(--card);border:1px solid var(--bd);border-radius:14px;padding:14px 14px 16px;margin:10px 0}
  .card h2{margin:0 0 10px;font-size:13px;text-transform:uppercase;letter-spacing:1.2px;color:var(--mut);font-weight:600}
  .grid{display:grid;grid-template-columns:repeat(4,1fr);gap:8px}
  @media(max-width:420px){.grid{grid-template-columns:repeat(2,1fr)}}
  .pat{background:#1e1e2e;border:1px solid var(--bd);border-radius:10px;padding:14px 8px;text-align:center;font-weight:600;cursor:pointer;transition:transform .08s, border-color .15s, background .15s}
  .pat:active{transform:scale(.96)}
  .pat.sel{border-color:var(--acc);background:linear-gradient(135deg,rgba(255,46,136,.18),rgba(0,225,255,.12));box-shadow:0 0 0 1px var(--acc) inset}
  .row{display:flex;align-items:center;gap:12px;margin:10px 0}
  .row label{flex:0 0 90px;color:var(--mut);font-size:12px;text-transform:uppercase;letter-spacing:1px}
  input[type=range]{flex:1;-webkit-appearance:none;appearance:none;height:28px;background:transparent}
  input[type=range]::-webkit-slider-runnable-track{height:6px;background:#2a2a3d;border-radius:3px}
  input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;appearance:none;width:22px;height:22px;border-radius:50%;background:linear-gradient(135deg,var(--acc),var(--acc2));margin-top:-8px;border:none;box-shadow:0 2px 8px rgba(0,0,0,.5)}
  .val{width:42px;text-align:right;color:var(--fg);font-variant-numeric:tabular-nums}
  .colors{display:flex;flex-wrap:wrap;gap:8px}
  .sw{width:36px;height:36px;border-radius:50%;cursor:pointer;border:2px solid transparent;transition:transform .08s}
  .sw:active{transform:scale(.9)}
  .sw.sel{border-color:#fff}
  input[type=color]{width:44px;height:44px;padding:0;border:none;background:transparent;cursor:pointer}
  .toggles{display:flex;gap:8px;flex-wrap:wrap}
  .btn{background:#1e1e2e;border:1px solid var(--bd);color:var(--fg);border-radius:10px;padding:10px 14px;font-weight:600;cursor:pointer;font-size:13px}
  .btn:active{transform:scale(.97)}
  .btn.sel{border-color:var(--acc2);background:rgba(0,225,255,.12)}
  .btn.warn{border-color:#ffb020;color:#ffb020}
  .info{font-size:11px;color:var(--mut);margin-top:10px;line-height:1.6}
  .info code{background:#0000004d;padding:1px 6px;border-radius:4px;color:#eaeaf2}
</style>
</head>
<body>
<header>
  <h1>AUSRASTIA</h1>
  <div class="status"><span id="dot" class="dot"></span><span id="dev">connecting…</span></div>
</header>
<main>

  <div class="card">
    <h2>Pattern</h2>
    <div class="grid" id="patterns"></div>
  </div>

  <div class="card">
    <h2>Regler</h2>
    <div class="row"><label>Helligkeit</label><input type="range" id="brightness" min="0" max="255"><span class="val" id="vBrightness">0</span></div>
    <div class="row"><label>Speed</label><input type="range" id="speed" min="0" max="255"><span class="val" id="vSpeed">0</span></div>
  </div>

  <div class="card">
    <h2>Farbe</h2>
    <div class="colors">
      <input type="color" id="colorPicker" value="#ff2e88">
      <div class="sw" style="background:#ff2e88" data-c="#ff2e88"></div>
      <div class="sw" style="background:#ff0000" data-c="#ff0000"></div>
      <div class="sw" style="background:#ff8800" data-c="#ff8800"></div>
      <div class="sw" style="background:#ffee00" data-c="#ffee00"></div>
      <div class="sw" style="background:#00ff55" data-c="#00ff55"></div>
      <div class="sw" style="background:#00e1ff" data-c="#00e1ff"></div>
      <div class="sw" style="background:#3355ff" data-c="#3355ff"></div>
      <div class="sw" style="background:#bb00ff" data-c="#bb00ff"></div>
      <div class="sw" style="background:#ffffff" data-c="#ffffff"></div>
    </div>
  </div>

  <div class="card">
    <h2>Verdrahtung (Wiring)</h2>
    <div class="toggles">
      <button class="btn" data-wiring="serpentine" id="wSerp">Serpentine</button>
      <button class="btn" data-wiring="rowmajor" id="wRow">Row-Major</button>
      <button class="btn warn" id="testMap">Pixel-Test</button>
    </div>
    <div class="info">
      Pixel-Test wandert 1 Pixel nach dem anderen über <b>Index 0 → <span id="nLeds">16</span></b>.
      Schau welcher LED zuerst aufleuchtet und in welche Richtung es läuft — dann entscheidest du
      zwischen Serpentine (Zickzack) und Row-Major (immer links→rechts).
    </div>
  </div>

</main>

<script>
const PATTERNS = [
  {id:1,name:"Solid"},{id:2,name:"Rainbow"},{id:3,name:"Pulse"},{id:4,name:"Strobe"},
  {id:5,name:"Fire"},{id:6,name:"Plasma"},{id:7,name:"Sparkle"},{id:8,name:"Wave"}
];
const $ = s => document.querySelector(s);
const grid = $("#patterns");
PATTERNS.forEach(p=>{
  const el=document.createElement("div");
  el.className="pat"; el.textContent=p.name; el.dataset.id=p.id;
  el.onclick=()=>send({cmd:"pattern",id:p.id});
  grid.appendChild(el);
});

let ws, reconnectT;
function connect(){
  ws = new WebSocket(`ws://${location.host}/ws`);
  ws.onopen = ()=>{ $("#dot").classList.add("on"); };
  ws.onclose = ()=>{ $("#dot").classList.remove("on"); $("#dev").textContent="reconnecting…"; clearTimeout(reconnectT); reconnectT=setTimeout(connect,1200); };
  ws.onmessage = ev => {
    const s = JSON.parse(ev.data);
    $("#dev").textContent = `${s.device}  ·  ${s.numLeds} LEDs`;
    $("#nLeds").textContent = s.numLeds;
    document.querySelectorAll(".pat").forEach(e=>e.classList.toggle("sel", +e.dataset.id===s.pattern));
    $("#brightness").value=s.brightness; $("#vBrightness").textContent=s.brightness;
    $("#speed").value=s.speed; $("#vSpeed").textContent=s.speed;
    $("#colorPicker").value=s.color;
    document.querySelectorAll(".sw").forEach(e=>e.classList.toggle("sel", e.dataset.c.toLowerCase()===s.color.toLowerCase()));
    $("#wSerp").classList.toggle("sel", s.serpentine);
    $("#wRow").classList.toggle("sel", !s.serpentine);
    $("#testMap").classList.toggle("sel", s.testMap);
    $("#testMap").textContent = s.testMap ? "Pixel-Test aus" : "Pixel-Test";
  };
}
function send(o){ if(ws && ws.readyState===1) ws.send(JSON.stringify(o)); }

$("#brightness").oninput=e=>{ $("#vBrightness").textContent=e.target.value; send({cmd:"brightness",v:+e.target.value}); };
$("#speed").oninput=e=>{ $("#vSpeed").textContent=e.target.value; send({cmd:"speed",v:+e.target.value}); };
function hexToRgb(h){ h=h.replace("#",""); return {r:parseInt(h.slice(0,2),16), g:parseInt(h.slice(2,4),16), b:parseInt(h.slice(4,6),16)}; }
$("#colorPicker").oninput=e=>{ const c=hexToRgb(e.target.value); send({cmd:"color",...c}); };
document.querySelectorAll(".sw").forEach(el=>el.onclick=()=>{ const c=hexToRgb(el.dataset.c); send({cmd:"color",...c}); });
$("#wSerp").onclick=()=>send({cmd:"wiring",mode:"serpentine"});
$("#wRow").onclick =()=>send({cmd:"wiring",mode:"rowmajor"});
$("#testMap").onclick=()=>{ const on=!$("#testMap").classList.contains("sel"); send({cmd:"testMap",on}); };
connect();
</script>
</body>
</html>
)HTML";
