#include "GirominController.h"
#include <Arduino.h>

GirominController::GirominController(int serialNumber, const IPAddress& outIp, unsigned int outPort, 
                                     const std::string& ssid, const std::string& pass, unsigned int localPort)
    : serialNumber_(serialNumber), outIp_(outIp), outPort_(outPort), 
      ssid_(ssid), pass_(pass), localPort_(localPort),
      basicTag_("/giromin/" + std::to_string(serialNumber)) 
{
}

void GirominController::setup() {
    Serial.begin(115200);
    // configurePWM();
    configureGiromin();
}

void GirominController::loop() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastLoopTime_ >= loopInterval_) {
        lastLoopTime_ = currentMillis;

        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("WiFi disconnected. Restarting...");
            esp_restart();
        }

        receiveOSCMessage();
        giromin.updateIMU();
        handleButton(giromin.botao1, "/b1");
        handleButton(giromin.botao2, "/b2");
        giromin.updateButton();
        giromin.updateOrientation();
        giromin.sendOSCIMUData(outIp_, outPort_, basicTag_);

        if (axp_.GetBtnPress() == 0x02) {
            Serial.println("AXP Reset Button pressed");
            esp_restart();
        }
    }
}

void GirominController::configurePWM() {
    constexpr int ledPin = 10;
    constexpr int freq = 5000;
    constexpr int ledChannel = 0;
    constexpr int resolution = 8;

    // ledcSetup(ledChannel, freq, resolution);
    // ledcAttachPin(ledPin, ledChannel);
    // ledcWrite(ledChannel, 511);
}

void GirominController::configureGiromin() {
    giromin.begin();
    giromin.setSerial(serialNumber_);
    axp_.begin();
    giromin.initEEPROM();
    giromin.initIMU();
    giromin.initButton();
    giromin.checkButtonAndCalibrateIMU();
    giromin.initWifi(ssid_, pass_, localPort_);
}

void GirominController::handleButton(Bounce& button, const std::string& suffix) {
    if (button.fell()) {
        Serial.println("Botão apertado");
        giromin.sendOSCButton(outIp_, outPort_, basicTag_, suffix, 1.0f);
    }
    if (button.rose()) {
        Serial.println("Botão solto");
        giromin.sendOSCButton(outIp_, outPort_, basicTag_, suffix, 0.0f);
    }
}

void GirominController::receiveOSCMessage() {
    OSCMessage inmsg;
    int size = giromin.Udp.parsePacket();
    if (size > 0) {
        while (size--) {
            inmsg.fill(giromin.Udp.read());
        }
        if (!inmsg.hasError()) {
            // inmsg.dispatch("/reset", [this](OSCMessage &msg) { this->oscReset(msg); });
            // inmsg.dispatch("/reset", oscReset);


        }
    }
}

// static void GirominController::oscReset(OSCMessage &msg) {
//     if (msg.getInt(0)) {
//         Serial.println("OSC Reset requested");
//         esp_restart();
//     }
// }
