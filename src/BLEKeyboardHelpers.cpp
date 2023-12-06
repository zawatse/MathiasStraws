#include "BLEKeyboardHelpers.h"

void BleKeyboardCallbacks::onConnect(BLEServer* server) {
    _isBleConnected = true;
    // Allow notifications for characteristics
    BLE2902* cccDesc = (BLE2902*)(*_input)->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDesc->setNotifications(true);
    Serial.println("Client has connected");
}
void BleKeyboardCallbacks::onDisconnect(BLEServer* server) {
    _isBleConnected = false;
    // Disallow notifications for characteristics
    BLE2902* cccDesc = (BLE2902*)(*_input)->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
    cccDesc->setNotifications(false);
    Serial.println("Client has disconnected");
}

 void OutputCallbacks::onWrite(BLECharacteristic* characteristic) {
    OutputReport* report = (OutputReport*) characteristic->getData();
    //Serial.print("LED state: ");
    //Serial.print((int) report->leds);
    //Serial.println();
}

void bluetoothKeyboardSetup(BLEHIDDevice** hid, BLECharacteristic** input, BLECharacteristic** output){
    // initialize the device
    delay(1000);
    Serial.println("BLUETOOTH SETUP STARTED");
    BLEDevice::init(DEVICE_NAME);
    BLEServer* server = BLEDevice::createServer();
    server->setCallbacks(new BleKeyboardCallbacks(input));
    // create an HID device
    (*hid) = new BLEHIDDevice(server);
    (*input) = (*hid)->inputReport(1); // report ID
    (*output) = (*hid)->outputReport(1); // report ID
    (*output)->setCallbacks(new OutputCallbacks());
    // set manufacturer name
    (*hid)->manufacturer()->setValue("TOM Vanderbilt");
    // set USB vendor and product ID
    (*hid)->pnp(0x02, 0xe502, 0xa111, 0x0210);
    // information about HID device: device is not localized, device can be connected
    (*hid)->hidInfo(0x00, 0x02);
    // Security: device requires bonding
    BLESecurity* security = new BLESecurity();
    security->setAuthenticationMode(ESP_LE_AUTH_BOND);
    // set report map
    (*hid)->reportMap((uint8_t*)REPORT_MAP, sizeof(REPORT_MAP));
    (*hid)->startServices();
    // set battery level to 100%
    (*hid)->setBatteryLevel(100);
    // advertise the services
    BLEAdvertising* advertising = server->getAdvertising();
    advertising->setAppearance(HID_KEYBOARD);
    advertising->addServiceUUID((*hid)->hidService()->getUUID());
    advertising->addServiceUUID((*hid)->deviceInfo()->getUUID());
    advertising->addServiceUUID((*hid)->batteryService()->getUUID());
    advertising->start();
    //Serial.println("BLE ready");
    delay(portMAX_DELAY);
}