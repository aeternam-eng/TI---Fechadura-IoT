#include <Arduino.h>
#include "BLE.h"


void setup() {
  Serial.begin(115200);

  pinMode(LOCK_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);

  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, HIGH);

  setupBT();
}

void loop() {
  if(hasToLock && ((millis() - lockMillis) > 5000)){
    setLockState(false);
    lockMillis = 0;
    hasToLock = false;

    BLEDevice::startAdvertising();
  }

  if (deviceConnected) {
      delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
  }
  if (!deviceConnected && oldDeviceConnected) {
      delay(500); // give the bluetooth stack the chance to get things ready
      pBLEServer->startAdvertising(); // restart advertising
      Serial.println("start advertising");
      oldDeviceConnected = deviceConnected;
  }
  if (deviceConnected && !oldDeviceConnected) {
      oldDeviceConnected = deviceConnected;
  }

  delay(50);
}