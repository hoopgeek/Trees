// ===== ws_portal.ino  (drop-in portal for viewing mesh messages) =====
// Note: WebSocket and WiFi headers are included in foc.ino

extern painlessMesh mesh;   // your sketch already defines this somewhere

// ---- Config ----
#ifndef WS_PORTAL_WS_PORT
#define WS_PORTAL_WS_PORT 81
#endif

// Serve a tiny HTML viewer at http://<esp-ip>/
static const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><meta charset="utf-8"/>
<title>painlessMesh Viewer</title>
<style>
body{font-family:system-ui,Segoe UI,Arial;margin:0;background:#111;color:#ddd}
#top{position:sticky;top:0;background:#181818;padding:8px 12px;border-bottom:1px solid #333}
#status{font-weight:600} button{margin-left:8px}
pre{white-space:pre-wrap;word-break:break-word;margin:0;padding:12px}
.time{color:#9cf} .from{color:#fc9} .dir{color:#9f9} .bad{color:#f66}
</style>
<div id="top">
  <span id="status">🔌 connecting…</span>
  <button onclick="log.innerText=''">Clear</button>
</div>
<pre id="log"></pre>
<script>
const log  = document.getElementById('log');
const stat = document.getElementById('status');
let ws;
function add(line, bad){ const el=document.createElement('div'); el.innerHTML=line; if(bad) el.className='bad'; log.appendChild(el); window.scrollTo(0,document.body.scrollHeight); }
function connect(){
  const proto = location.protocol === 'https:' ? 'wss':'ws';
  ws = new WebSocket(proto + '://' + location.host.replace(/:\\d+$/,'') + ':__WS_PORT__/');
  ws.onopen  = () => { stat.textContent = '✅ connected'; };
  ws.onclose = () => { stat.textContent = '⚠️ disconnected, retrying…'; setTimeout(connect, 1200); };
  ws.onerror = () => { add('ws error', true); };
  ws.onmessage = (ev) => {
    try {
      const m = JSON.parse(ev.data);
      if (m.type === 'hello') { add(`hello from node ${m.nodeId} (root:${m.root})`); return; }
      if (m.type === 'mesh') {
        const when = (m.at_us/1000).toFixed(3)+'ms';
        add(`<span class="time">${when}</span> <span class="dir">${m.dir}</span> from <span class="from">${m.peer}</span> → ${m.msg}`);
        return;
      }
      add(ev.data);
    } catch { add(ev.data); }
  };
}
connect();
</script>
)HTML";

// ---- Server instances ----

WebServer http(80);
WebSocketsServer ws(WS_PORTAL_WS_PORT);


// Call this from wsPortalBegin() after Wi-Fi is up:
void wsPortalStartMDNS() {
  const char* host = "mesh-portal";     // change if you like
  if (MDNS.begin(host)) {
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("ws",   "tcp", WS_PORTAL_WS_PORT);
    Serial.printf("[ws_portal] mDNS: http://%s.local  ws://%s.local:%d/\n",
                  host, host, WS_PORTAL_WS_PORT);
  } else {
    Serial.println("[ws_portal] mDNS failed to start");
  }
}

// ---- Tiny ring buffer so we never block when broadcasting ----
struct WsItem { uint16_t len; char data[240]; };
static WsItem wsQueue[32];
static volatile uint8_t wsQHead=0, wsQTail=0;

static bool wsEnqueue(const char* s) {
  uint8_t next = (wsQHead + 1) & 31;
  if (next == wsQTail) return false;            // drop if full
  size_t n = strlen(s); if (n > 239) n = 239; // truncate
  memcpy(wsQueue[wsQHead].data, s, n); wsQueue[wsQHead].data[n] = 0; wsQueue[wsQHead].len = (uint16_t)n;
  wsQHead = next; return true;
}
static void wsDrain() {
  if (ws.connectedClients() == 0) { wsQTail = wsQHead; return; } // discard if nobody listening
  while (wsQTail != wsQHead) {
    ws.broadcastTXT(wsQueue[wsQTail].data, wsQueue[wsQTail].len);
    wsQTail = (wsQTail + 1) & 31;
  }
}

// ---- JSON helpers ----
static void jsonEscape(const String& in, String& out) {
  out.reserve(in.length()+8);
  for (size_t i=0;i<in.length();++i) {
    char c = in[i];
    if (c=='"' || c=='\\') { out += '\\'; out += c; }
    else if (c=='\n') out += "\\n";
    else if (c=='\r') {}       // skip
    else out += c;
  }
}
static void sendHello(uint8_t num) {
  char buf[128];
  snprintf(buf, sizeof(buf),
    "{\"type\":\"hello\",\"nodeId\":%lu,\"root\":%s,\"clients\":%u}",
    (unsigned long)mesh.getNodeId(), mesh.isRoot() ? "true":"false", num);
  wsEnqueue(buf);
}

// ---- WebSocket event ----
static void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t len) {
  if (type == WStype_CONNECTED) {
    sendHello(num);
  }
}

// ---- Public API ----
void wsPortalBegin(const char* title) {
  // 1) WebSocket
  ws.begin();
  ws.enableHeartbeat(
    3000,  // ping every 3s (more aggressive for iOS)
    2000,  // expect a pong within 2s
    2      // disconnect after 2 missed pongs (client will auto-retry)
  );
  ws.onEvent(onWsEvent);

  // 2) Minimal HTTP UI
  http.on("/", HTTP_GET, [title](){
    String page(reinterpret_cast<const __FlashStringHelper*>(INDEX_HTML));
    page.replace("__WS_PORT__", String(WS_PORTAL_WS_PORT));
    http.send(200, "text/html; charset=utf-8", page);
  });
  http.on("/ping", HTTP_GET, [](){ http.send(200, "text/plain", "ok"); });
  http.begin();
  wsPortalStartMDNS();

  // 3) Serial hint so you know where to browse
  IPAddress ip = WiFi.localIP();
  Serial.printf("[ws_portal] HTTP http://%s/  WS ws://%s:%d/\n",
                ip.toString().c_str(), ip.toString().c_str(), WS_PORTAL_WS_PORT);
}

// Call this every loop()
void wsPortalUpdate() {
  ws.loop();          // non-blocking
  http.handleClient();// non-blocking
  wsDrain();          // non-blocking
  // Note: ESP32 mDNS runs in background, no update() needed
}

// Call this inside your mesh onReceive()
void wsPortalForwardMesh(uint32_t from, const String& msg) {
  String esc; jsonEscape(msg, esc);
  // Note: mesh.getNodeTime() is in microseconds and rolls over; fine for display
  String line;
  line.reserve(esc.length() + 96);
  line += "{\"type\":\"mesh\",\"dir\":\"in\",\"peer\":";
  line += String(from);
  line += ",\"at_us\":";
  line += String(mesh.getNodeTime());
  line += ",\"msg\":\""; line += esc; line += "\"}";
  wsEnqueue(line.c_str());
}

// Optional: call this wherever you send (so you can see your own outgoing app msgs)
void wsPortalTeeOutgoing(uint32_t to, const String& msg) {
  String esc; jsonEscape(msg, esc);
  String line;
  line.reserve(esc.length() + 96);
  line += "{\"type\":\"mesh\",\"dir\":\"out\",\"peer\":";
  line += String(to);
  line += ",\"at_us\":";
  line += String(mesh.getNodeTime());
  line += ",\"msg\":\""; line += esc; line += "\"}";
  wsEnqueue(line.c_str());
}
