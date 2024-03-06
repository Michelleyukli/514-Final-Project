#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <AccelStepper.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Stepper motor pin definitions
#define MOTOR_PIN1 0
#define MOTOR_PIN2 1
#define MOTOR_PIN3 2
#define MOTOR_PIN4 3
// Initialize AccelStepper for X27 stepper motor (4 pin connection)
AccelStepper stepper(AccelStepper::FULL4WIRE, MOTOR_PIN1, MOTOR_PIN3, MOTOR_PIN2, MOTOR_PIN4);

// The remote service and characteristic we wish to connect to.
static BLEUUID serviceUUID("b0baad1d-7659-47ca-9e6e-e6558c186240");
static BLEUUID charUUID("8d28bf86-3102-4978-8693-47651082cade");

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static BLERemoteCharacteristic* pRemoteCharacteristic;
static BLEAdvertisedDevice* myDevice;

// Notification callback function to handle data received from the server
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    display.clearDisplay(); // Clear the display
    display.setCursor(0,0); // Set cursor to top-left

    String dataStr = "";
    for(int i = 0; i < length; i++) {
        dataStr += (char)pData[i];
    }

    display.println("Best Prediction:");
    display.println(dataStr);
    display.display(); // Refresh the display to show new data

    // Rotate stepper motor a small angle and reset to 0
    stepper.move(100); // Example small move
    stepper.runToPosition(); // Move the motor
    stepper.moveTo(0); // Move back to position 0
    stepper.runToPosition(); // Reset the motor position
}

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("Disconnected");
  }
};

bool connectToServer() {
    Serial.print("Connecting to ");
    Serial.println(myDevice->getAddress().toString().c_str());

    BLEClient* pClient = BLEDevice::createClient();
    Serial.println(" - Created client");

    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");

    // Obtain a reference to the service in the remote BLE server.
    BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService == nullptr) {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(serviceUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our service");

    // Obtain a reference to the characteristic in the service of the remote BLE server.
    pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
    if (pRemoteCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(charUUID.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Serial.println(" - Found our characteristic");

    if(pRemoteCharacteristic->canNotify())
      pRemoteCharacteristic->registerForNotify(notifyCallback);

    connected = true;
    return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("BLE Advertised Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      myDevice = new BLEAdvertisedDevice(advertisedDevice);
      doConnect = true;
      doScan = false;
    }
  }
};

void setup() {
  Serial.begin(115200);
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();

  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1349);
  pBLEScan->setWindow(449);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);
  stepper.setMaxSpeed(1000); // Set max speed
  stepper.setAcceleration(500); // Set acceleration
} 

void loop() {
  if (doConnect) {
    if (connectToServer()) {
      Serial.println("Connected to the BLE Server.");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Awaiting predictions...");
      display.display();
    } else {
      Serial.println("Failed to connect.");
      display.clearDisplay();
      display.setCursor(0,0);
      display.println("Failed to connect to BLE Server");
      display.display();
    }
    doConnect = false;
  }

  if (doScan && !connected) {
    BLEDevice::getScan()->start(0); // Restart scan
  }

  delay(1000); // Delay between loops
}
