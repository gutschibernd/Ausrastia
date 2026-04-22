// Ausrastia — ESP32 LED-Matrix Firmware
// Eine Matrix pro ESP. Web-UI per WiFi + WebSocket.
// Spaeter: UDP-Listener fuer zentrale Sync-Kommandos vom Raspberry Pi.

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <FastLED.h>

#include "config.h"
#include "web_ui.h"

CRGB leds[NUM_LEDS];
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

struct State {
  uint8_t  pattern    = 2;     // 1..8
  uint8_t  brightness = 80;
  uint8_t  speed      = 128;
  CRGB     color      = CRGB(255, 46, 136);
  bool     serpentine = true;
  bool     testMap    = false;
  uint8_t  testIdx    = 0;
  uint32_t testLastMs = 0;
} st;

// --- Koordinaten -> LED-Index (abhaengig von Verdrahtung) ---
uint16_t XY(uint8_t x, uint8_t y) {
  if (st.serpentine) {
    if (y & 1) return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
    return y * MATRIX_WIDTH + x;
  }
  return y * MATRIX_WIDTH + x;
}

// ============================================================
// Animations
// ============================================================
void pat_solid() { fill_solid(leds, NUM_LEDS, st.color); }

void pat_rainbow() {
  static uint8_t hue = 0;
  hue += (st.speed / 20) + 1;
  for (uint16_t i = 0; i < NUM_LEDS; i++) leds[i] = CHSV(hue + i * 16, 255, 255);
}

void pat_pulse() {
  uint8_t bpm = map(st.speed, 0, 255, 20, 180);
  uint8_t b   = beatsin8(bpm, 20, 255);
  CRGB c = st.color; c.nscale8(b);
  fill_solid(leds, NUM_LEDS, c);
}

void pat_strobe() {
  uint32_t period = map(st.speed, 0, 255, 400, 30);
  if ((millis() / period) & 1) fill_solid(leds, NUM_LEDS, st.color);
  else                         fill_solid(leds, NUM_LEDS, CRGB::Black);
}

void pat_fire() {
  static byte heat[NUM_LEDS];
  const uint8_t cooling = 55, sparking = 120;
  for (int i = 0; i < NUM_LEDS; i++)
    heat[i] = qsub8(heat[i], random8(0, ((cooling * 10) / NUM_LEDS) + 2));
  for (int k = NUM_LEDS - 1; k >= 2; k--)
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  if (random8() < sparking) {
    int y = random8(3);
    heat[y] = qadd8(heat[y], random8(160, 255));
  }
  for (int j = 0; j < NUM_LEDS; j++) leds[j] = HeatColor(heat[j]);
}

void pat_plasma() {
  static uint32_t t = 0;
  t += (st.speed / 10) + 1;
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t v = sin8(x * 40 + t / 4) + sin8(y * 40 + t / 6);
      leds[XY(x, y)] = CHSV(v, 255, 255);
    }
}

void pat_sparkle() {
  fadeToBlackBy(leds, NUM_LEDS, 40);
  uint8_t thr = map(st.speed, 0, 255, 20, 200);
  if (random8() < thr) leds[random16(NUM_LEDS)] = st.color;
}

// Welle, die sich ueber die gesamte Buehne erstreckt:
// DEVICE_ID verschiebt die Phase -> spaeter laufen alle 30 Matrizen synchron.
void pat_wave() {
  static uint16_t t = 0;
  t += (st.speed / 8) + 1;
  uint16_t stageOffset = (DEVICE_ID - 1) * MATRIX_WIDTH * 32;
  for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++) {
      uint8_t v = sin8(t + stageOffset + x * 32 + y * 16);
      leds[XY(x, y)] = CHSV(v, 255, v);
    }
}

// Diagnose: Pixel fuer Pixel nach Index durchschalten.
void runTestMap() {
  if (millis() - st.testLastMs > 400) {
    st.testLastMs = millis();
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    leds[st.testIdx] = CRGB::White;
    st.testIdx = (st.testIdx + 1) % NUM_LEDS;
  }
  // Waehrend des Laufs auch Index 0 schwach rot halten, als Anker
  leds[0] += CRGB(40, 0, 0);
}

// ============================================================
// WebSocket
// ============================================================
void broadcastState() {
  JsonDocument doc;
  doc["pattern"]    = st.pattern;
  doc["brightness"] = st.brightness;
  doc["speed"]      = st.speed;
  char col[8];
  snprintf(col, sizeof(col), "#%02X%02X%02X", st.color.r, st.color.g, st.color.b);
  doc["color"]      = col;
  doc["serpentine"] = st.serpentine;
  doc["testMap"]    = st.testMap;
  doc["device"]     = DEVICE_NAME;
  doc["deviceId"]   = DEVICE_ID;
  doc["numLeds"]    = NUM_LEDS;
  String s; serializeJson(doc, s);
  ws.textAll(s);
}

void onWsEvent(AsyncWebSocket*, AsyncWebSocketClient* c, AwsEventType type,
               void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    broadcastState();
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (!info->final || info->opcode != WS_TEXT) return;
    JsonDocument doc;
    if (deserializeJson(doc, data, len)) return;
    const char* cmd = doc["cmd"] | "";
    if      (!strcmp(cmd, "pattern"))    st.pattern    = doc["id"] | 1;
    else if (!strcmp(cmd, "brightness")) { st.brightness = doc["v"] | 80; FastLED.setBrightness(st.brightness); }
    else if (!strcmp(cmd, "speed"))      st.speed      = doc["v"] | 128;
    else if (!strcmp(cmd, "color"))      st.color      = CRGB(doc["r"] | 255, doc["g"] | 0, doc["b"] | 0);
    else if (!strcmp(cmd, "wiring"))     st.serpentine = !strcmp(doc["mode"] | "serpentine", "serpentine");
    else if (!strcmp(cmd, "testMap"))    { st.testMap = doc["on"] | false; st.testIdx = 0; }
    broadcastState();
  }
}

// ============================================================
// Setup / Loop
// ============================================================
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== Ausrastia boot ===");

  FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalLEDStrip);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, LED_MAX_MA);
  FastLED.setBrightness(st.brightness);
  FastLED.clear(); FastLED.show();

  // Boot-Blink damit man sieht dass das Teil lebt
  for (int i = 0; i < 3; i++) {
    fill_solid(leds, NUM_LEDS, CRGB(20, 0, 40)); FastLED.show(); delay(120);
    FastLED.clear(); FastLED.show(); delay(120);
  }

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(DEVICE_NAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("Connecting to %s", WIFI_SSID);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(300); Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWiFi OK  IP=%s  host=%s.local\n",
                  WiFi.localIP().toString().c_str(), DEVICE_NAME);
    // gruener Bestaetigungs-Flash
    fill_solid(leds, NUM_LEDS, CRGB(0, 40, 0)); FastLED.show(); delay(300);
  } else {
    Serial.println("\nWiFi FAIL — starting AP");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(DEVICE_NAME, "ausrastia");
    Serial.printf("AP IP=%s  SSID=%s  pw=ausrastia\n",
                  WiFi.softAPIP().toString().c_str(), DEVICE_NAME);
    fill_solid(leds, NUM_LEDS, CRGB(40, 20, 0)); FastLED.show(); delay(300);
  }

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send_P(200, "text/html; charset=utf-8", INDEX_HTML);
  });
  server.onNotFound([](AsyncWebServerRequest* r) { r->send(404, "text/plain", "404"); });
  server.begin();
  Serial.println("HTTP+WS server up on :80");
}

void loop() {
  ws.cleanupClients();

  if (st.testMap) {
    runTestMap();
  } else {
    switch (st.pattern) {
      case 1: pat_solid();   break;
      case 2: pat_rainbow(); break;
      case 3: pat_pulse();   break;
      case 4: pat_strobe();  break;
      case 5: pat_fire();    break;
      case 6: pat_plasma();  break;
      case 7: pat_sparkle(); break;
      case 8: pat_wave();    break;
      default: pat_rainbow();
    }
  }
  FastLED.show();
  delay(1000 / 60); // ~60 FPS
}
