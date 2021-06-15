#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLEAdvertisedDevice.h>

#define LOCK_PIN GPIO_NUM_2
#define GREEN_LED_PIN GPIO_NUM_16
#define RED_LED_PIN GPIO_NUM_4

#define SERVICE_UUID "9e98d7aF-d2f9-42f5-acd2-bcb5a5cdc7df"

#define LOCK_STATUS_CHARACTERISTIC_UUID "9e98d7ae-d2f9-42f5-acd2-bcb5a5cdc7df"
#define LOCK_STATUS_DESCRIPTOR_UUID "70d709c2-8f3d-11eb-8dcd-0242ac130003"

BLEServer* pBLEServer;
BLEService* pBLEService;
BLECharacteristic* pBLECharacteristic;

bool hasToLock = false;
unsigned long  lockMillis = 0;

bool deviceConnected = false;
bool oldDeviceConnected = false;

void setupBT();
void setLockState(bool state);

class MyBLEServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        log_d("Client Connected! Peer MTU: %d | Conn ID: %d", pServer->getPeerMTU(pServer->getConnId()), pServer->getConnId());
        BLEDevice::startAdvertising();
        deviceConnected = true;
    }

    void onDisconnect(BLEServer* pServer) {
        log_d("Client Disconnected! Peer MTU: %d | Conn ID: %d", pServer->getPeerMTU(pServer->getConnId()), pServer->getConnId());
        deviceConnected = false;
    }
};

class LockStatusCharacteristicCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string rxValue = pCharacteristic->getValue();

        if(rxValue.length() != 1){
            return;
        }

        uint8_t receivedByte = pCharacteristic->getData()[0];

        if(receivedByte == 0x7F) {
            setLockState(true);
        } else if(receivedByte == 0xFF) {
            setLockState(false);
        } 
    }
};

void setupBT() {
    BLEDevice::init("IoT Lock");

    pBLEServer = BLEDevice::createServer();
    pBLEServer->setCallbacks(new MyBLEServerCallbacks());
    pBLEService = pBLEServer->createService(SERVICE_UUID);

    BLEDescriptor* pBLELockStatusDescriptor = new BLEDescriptor(LOCK_STATUS_DESCRIPTOR_UUID);
    pBLELockStatusDescriptor->setValue("Lock Status");

    pBLECharacteristic = pBLEService->createCharacteristic(LOCK_STATUS_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_WRITE);
    pBLECharacteristic->addDescriptor(pBLELockStatusDescriptor);
    pBLECharacteristic->setCallbacks(new LockStatusCharacteristicCallbacks());
    uint16_t a = 0xFF;
    pBLECharacteristic->setValue(a);

    pBLEService->start();

    BLEAdvertising* pBLEAdvertising = BLEDevice::getAdvertising();
    pBLEAdvertising->addServiceUUID(SERVICE_UUID);
    pBLEAdvertising->setScanResponse(false);
    pBLEAdvertising->setMinPreferred(0x00);
    //pBLEAdvertising->setMinPreferred(0x06);
    //pBLEAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();

    log_d("BLE configured.");
}

void setLockState(bool state){
    //TRUE == OPEN
    if(state) {
        digitalWrite(LOCK_PIN, HIGH);

        digitalWrite(GREEN_LED_PIN, HIGH);
        digitalWrite(RED_LED_PIN, LOW);

        uint16_t a = 0x7F;
        pBLECharacteristic->setValue(a);
        pBLECharacteristic->notify();
        log_w("Lock has been opened.");

        hasToLock = true;
        lockMillis = millis();
    } else {
        digitalWrite(LOCK_PIN, LOW);

        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, HIGH);

        uint16_t a = 0xFF;
        pBLECharacteristic->setValue(a);
        pBLECharacteristic->notify();
        log_w("Lock has been closed.");
    }
}

