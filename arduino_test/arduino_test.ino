// Minimal-Test fuer ESP-WROOM-32
// Blinkt die Onboard-LED an GPIO 2 und schreibt in den Serial-Monitor.
// Arduino IDE: Board = "ESP32 Dev Module", Port = /dev/cu.usbserial-1120, 115200 Baud.

#define LED 2

void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("=== ESP32 Hello ===");
}

void loop() {
  digitalWrite(LED, HIGH);
  Serial.println("tick");
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
