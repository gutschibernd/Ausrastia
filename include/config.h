#pragma once

// --- WiFi ---
#define WIFI_SSID "GutschiLAN"
#define WIFI_PASS "87494621735234247554"

// --- Device identity (change per ESP when you flash the others, 1..30) ---
#define DEVICE_ID   1
#define DEVICE_NAME "ausrastia-01"

// --- LED matrix ---
// WS2812B data line. GPIO 5 is safe on ESP-WROOM-32 dev boards.
// If nothing lights up, try GPIO 4 or 13.
#define LED_PIN        5
#define MATRIX_WIDTH   4
#define MATRIX_HEIGHT  4
#define NUM_LEDS       (MATRIX_WIDTH * MATRIX_HEIGHT)

// Max current limit per matrix (mA). 4x4 on USB is safe, keep headroom.
#define LED_MAX_MA     800
