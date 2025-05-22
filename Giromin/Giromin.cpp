#include "Arduino.h"
#include "Giromin.h"

using namespace GirominConstants;

Giromin::Giromin() : 
    LCD(14, 27, 33) // Inicialização direta do LCD
{
}

void Giromin::begin(bool SerialEnable, bool I2CEnable) {
    if (isInited) return;
    isInited = true;

    if (SerialEnable) {
        Serial.begin(115200);
        Serial.flush();
        delay(50);
        Serial.println("Giromin initializing...");
    }

    pinMode(LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(BUTTON_1_PIN, INPUT_PULLUP);
    pinMode(BUTTON_2_PIN, INPUT_PULLUP);

    if (I2CEnable) {
        Wire.begin(21, 22);
    }

    if (SerialEnable) {
        Serial.println("OK");
    }
}

void Giromin::initLCD() {
    LCD.begin();
    LCD.setFont(u8x8_font_chroma48medium8_r);
    LCD.drawString(0, 0, "Giromin");
    LCD.drawString(0, 1, "0.1.1#");
    LCD.setCursor(6, 1);
    LCD.print(serialNumber_);

    unsigned long start = millis();
    while (millis() - start < 1500) {
        // Aguarda 1,5 segundos sem bloquear outros processos
    }
}

void Giromin::initEEPROM() {
    EEPROM.begin(EEPROM_SIZE);
    int addr = 0;
    EEPROM.get(addr, gxOffset_); addr += sizeof(float);
    EEPROM.get(addr, gyOffset_); addr += sizeof(float);
    EEPROM.get(addr, gzOffset_); addr += sizeof(float);
    EEPROM.get(addr, axOffset_); addr += sizeof(float);
    EEPROM.get(addr, ayOffset_); addr += sizeof(float);
    EEPROM.get(addr, azOffset_);

    Serial.println("Calibration data loaded from EEPROM:");
    Serial.printf("(gx, gy, gz, ax, ay, az) = (%.5f, %.5f, %.5f, %.5f, %.5f, %.5f)\n",
                  gxOffset_, gyOffset_, gzOffset_, axOffset_, ayOffset_, azOffset_);
}

void Giromin::initWifi(const std::string& ssid, const std::string& pass, unsigned int localPort) {
    Serial.printf("\nConnecting to %s\n", ssid.c_str());
    WiFi.begin(ssid.c_str(), pass.c_str());

    const unsigned long timeout = 10000; // 10s
    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - start > timeout) {
            Serial.println("\nFailed to connect to WiFi.");
            esp_restart();
            return;
        }
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected. IP address: ");
    Serial.println(WiFi.localIP());

    Udp.begin(localPort);
    Serial.printf("UDP local port: %u\n", localPort);
}

void Giromin::initIMU() {
    if (IMU.Init() == 0) {
        Serial.println("IMU MPU6886 initialized successfully");
    }
}

void Giromin::initButton() {
    botao1.attach(BUTTON_1_PIN);
    botao1.interval(5);
    botao2.attach(BUTTON_2_PIN);
    botao2.interval(5);
}

void Giromin::checkButtonAndCalibrateIMU() {
    botao1.update();
    if (botao1.read() == LOW) {
        calibrateIMU();
    }
}

void Giromin::updateIMU() {
    IMU.getAccelData(&ax_, &ay_, &az_);
    IMU.getGyroData(&gx_, &gy_, &gz_);

    ax = ax_ - axOffset_;
    ay = ay_ - ayOffset_;
    az = az_ - azOffset_;
    gx = (gx_ - gxOffset_) * PI / 180.0f;
    gy = (gy_ - gyOffset_) * PI / 180.0f;
    gz = (gz_ - gzOffset_) * PI / 180.0f;
}

void Giromin::updateButton() {
    botao1.update();
    botao2.update();
}

void Giromin::longButtonPress() {
    Serial.println("Long press detected.");
}

void Giromin::shortButtonPress() {
    Serial.println("Short press detected.");
#ifdef SOUND_ON
    for (int i = 0; i < 10; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(1);
        digitalWrite(BUZZER_PIN, LOW);
        delay(1);
    }
#endif
}

void Giromin::updateOrientation() {
    deltat_ = Fusion.deltatUpdate();
    Fusion.MadgwickUpdate(gx, gy, gz, ax, ay, az, deltat_);
    q0 = Fusion.getQ0();
    q1 = Fusion.getQ1();
    q2 = Fusion.getQ2();
    q3 = Fusion.getQ3();
}

void Giromin::rotateQuaternions() {
    q0_ = q0;
    q1_ = q2;
    q2_ = q3;
    q3_ = q1;
}

void Giromin::sendOSCIMUData(const IPAddress outIp, unsigned int outPort, const std::string& basicTag) {
    OSCBundle imu;

    imu.add((basicTag + "/a/x").c_str()).add(ax);
    imu.add((basicTag + "/a/y").c_str()).add(ay);
    imu.add((basicTag + "/a/z").c_str()).add(az);

    imu.add((basicTag + "/g/x").c_str()).add(gx);
    imu.add((basicTag + "/g/y").c_str()).add(gy);
    imu.add((basicTag + "/g/z").c_str()).add(gz);

    imu.add((basicTag + "/q").c_str()).add(q1).add(q2).add(q3).add(q0);

    Udp.beginPacket(outIp, outPort);
    imu.send(Udp);
    Udp.endPacket();
    imu.empty();
}

void Giromin::sendOSCButton(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTag, float oscButVal) {
    std::string oscTag = basicTag + specificTag;

    OSCMessage but(oscTag.c_str());
    but.add(oscButVal);

    Udp.beginPacket(outIp, outPort);
    but.send(Udp);
    Udp.endPacket();
    but.empty();
}

void Giromin::calibrateIMU() {
    Serial.println("Calibrating IMU, keep device still...");

    const unsigned long calibrationDuration = 5000;
    unsigned long startTime = millis();

    while (millis() - startTime < calibrationDuration) {
        // Esperando estabilização
    }

    gxOffset_ = gyOffset_ = gzOffset_ = 0.0f;
    axOffset_ = ayOffset_ = azOffset_ = 0.0f;

    for (int i = 0; i < CALIB_SAMPLES; ++i) {
        IMU.getAccelData(&ax_, &ay_, &az_);
        IMU.getGyroData(&gx_, &gy_, &gz_);

        gxOffset_ += gx_;
        gyOffset_ += gy_;
        gzOffset_ += gz_;

        axOffset_ += ax_;
        ayOffset_ += ay_;
        azOffset_ += az_;
    }

    gxOffset_ /= CALIB_SAMPLES;
    gyOffset_ /= CALIB_SAMPLES;
    gzOffset_ /= CALIB_SAMPLES;
    axOffset_ /= CALIB_SAMPLES;
    ayOffset_ /= CALIB_SAMPLES;
    azOffset_ = (azOffset_ / CALIB_SAMPLES) - 1.0f;

    int addr = 0;
    EEPROM.put(addr, gxOffset_); addr += sizeof(float);
    EEPROM.put(addr, gyOffset_); addr += sizeof(float);
    EEPROM.put(addr, gzOffset_); addr += sizeof(float);
    EEPROM.put(addr, axOffset_); addr += sizeof(float);
    EEPROM.put(addr, ayOffset_); addr += sizeof(float);
    EEPROM.put(addr, azOffset_);
    EEPROM.commit();

    Serial.println("Calibration data saved to EEPROM.");
}

void Giromin::setIMUOffsetValues(float gxOffset, float gyOffset, float gzOffset, float axOffset, float ayOffset, float azOffset) {
    gxOffset_ = gxOffset;
    gyOffset_ = gyOffset;
    gzOffset_ = gzOffset;
    axOffset_ = axOffset;
    ayOffset_ = ayOffset;
    azOffset_ = azOffset;
}

// Instância global
Giromin giromin;
