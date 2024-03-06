#include <Arduino.h>
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include "Frisbee_1_inferencing.h" // Include the machine learning model header

Adafruit_MPU6050 mpu;

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool isAnalyzing = false; // Flag to control the analysis state

#define SERVICE_UUID        "b0baad1d-7659-47ca-9e6e-e6558c186240"
#define CHARACTERISTIC_UUID "8d28bf86-3102-4978-8693-47651082cade"

// Structure to hold the best prediction
struct Prediction {
  String label;
  float value;
} bestPrediction;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override {
        deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) override {
        deviceConnected = false;
    }
};

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for the serial port to connect
    Serial.println("MPU6050 starting...");

    if (!mpu.begin()) {
        Serial.println("Failed to find MPU6050 chip");
        while (1) {
            delay(10);
        }
    }   

    Serial.println("MPU6050 Found!");
    Serial.println("Press SPACE to start/stop analysis.");

    BLEDevice::init("XIAO_ESP32S3");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
    pCharacteristic->addDescriptor(new BLE2902());

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("BLE ready, waiting for connections...");
}

void loop() {
    // Check for spacebar press to toggle analysis state
    if (Serial.available() > 0) {
        char receivedChar = Serial.read();
        if (receivedChar == ' ') {
            isAnalyzing = !isAnalyzing; // Toggle analysis state
            if (isAnalyzing) {
                Serial.println("Analysis started. Press SPACE to stop.");
                bestPrediction.label = "";
                bestPrediction.value = 0;
            } else {
                Serial.println("Analysis stopped.");
                // Print the best prediction after stopping
                if (!bestPrediction.label.isEmpty()) {
                    String message =  bestPrediction.label + " : " + String(bestPrediction.value, 6);
                    Serial.println(message);

                    // Send result over BLE if a device is connected
                    if (deviceConnected) {
                        pCharacteristic->setValue(message.c_str());
                        pCharacteristic->notify();
                    }
                } else {
                    Serial.println("No valid prediction made during this period.");
                }
            }
        }
    
        // Perform analysis if the isAnalyzing flag is true
        if (isAnalyzing) {
            sensors_event_t a, g, temp;
            mpu.getEvent(&a, &g, &temp);

            float features[] = {a.acceleration.x, a.acceleration.y, a.acceleration.z, g.gyro.x, g.gyro.y, g.gyro.z};
            ei::signal_t signal;
            signal.total_length = sizeof(features) / sizeof(features[0]);
            signal.get_data = [&](size_t offset, size_t length, float *out_ptr) -> int {
                memcpy(out_ptr, features + offset, length * sizeof(float));
                return 0;
            };

            ei_impulse_result_t result;
            EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);
            if (res == EI_IMPULSE_OK) {
                for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
                    if (result.classification[ix].value > bestPrediction.value) {
                        bestPrediction.label = result.classification[ix].label;
                        bestPrediction.value = result.classification[ix].value;
                    }
                }
            } else {
                Serial.println("Inference failed");
            }
        }
    }

    // Handle the connection state changes
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("Start advertising");
        oldDeviceConnected = deviceConnected;
    }

    if (deviceConnected && !oldDeviceConnected) {
        // Here, you can do tasks that should happen once upon reconnection
        oldDeviceConnected = deviceConnected;
    }

    delay(1000); // Small delay to keep the loop manageable
}


