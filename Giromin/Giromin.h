/*
 Giromin.h - Library for the Giromin Gestural Controller
 Created by João Tragtenberg, December 2, 2019.
*/

#ifndef GIROMIN_H
#define GIROMIN_H

#if defined(ESP32)

#include <Arduino.h>
#include <Wire.h>
#include <string>
#include <array>

#include "CommUtil.h"
#include "MPU6886.h"
#include "Madgwick.h"

#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>

#include <U8x8lib.h>
#include <Bounce2.h>
#include <EEPROM.h>

//Constantes de hardware
namespace GirominConstants {
constexpr int CALIB_SAMPLES = 1000;
constexpr int LED_PIN = 11;
constexpr int BUZZER_PIN = 2;
constexpr int BUTTON_1_PIN = 39;
constexpr int BUTTON_2_PIN = 37;
constexpr unsigned long LONG_PRESS = 20000;  // ms
constexpr int EEPROM_SIZE = 30;
}

//Classe principal
class Giromin {
public:
  Giromin();

  // === Inicialização ===
  void begin(bool SerialEnable = true, bool I2CEnable = true);
  void initLCD();
  void initWifi(const std::string& ssid, const std::string& pass, unsigned int localPort);
  void initIMU();
  void initEEPROM();
  void initButton();

  void setSerial(int serial) {
    serialNumber_ = serial;
  }
  void useMagnetometer() {
    useMagnetometer_ = true;
    Serial.println("Using magnetometer");
  }

  // === Botões ===
  void longButtonPress();
  void shortButtonPress();
  void checkButtonAndCalibrateIMU();

  // === Sensores ===
  void updateIMU();
  void updateButton();
  void updateOrientation();
  void rotateQuaternions();

  // === Comunicação OSC ===
  void sendOSCRawIMUData(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTagA, const std::string& specificTagG);
  void sendOSCRawAccData(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTag);
  void sendOSCRawGyroData(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTag);
  void sendOSCQuaternions(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTag);
  void sendOSCIMUData(const IPAddress outIp, unsigned int outPort, const std::string& basicTag);
  void sendOSCButton(const IPAddress outIp, unsigned int outPort, const std::string& basicTag, const std::string& specificTag, float oscButVal);

  // === Calibração ===
  void calibrateIMU();
  void setIMUOffsetValues(float gxOffset, float gyOffset, float gzOffset,
                          float axOffset, float ayOffset, float azOffset);

  // === Componentes ===
  Bounce botao1{};
  Bounce botao2{};
  MPU6886 IMU{};
  U8X8_SH1107_64X128_4W_HW_SPI LCD{ 14, 27, 33 };
  WiFiUDP Udp{};
  Madgwick Fusion{};

  // === Variáveis públicas ===
  float ax = 0.0f, ay = 0.0f, az = 0.0f;
  float gx = 0.0f, gy = 0.0f, gz = 0.0f;
  float mx = 0.0f, my = 0.0f, mz = 0.0f;
  float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;

  bool longPressFlag = false;
  unsigned long buttonPressTimeStamp = 0;

private:
  // === Flags e Estados ===
  bool isInited = false;
  bool useMagnetometer_ = false;

  // === Offsets e Internos ===
  float ax_ = 0.0f, ay_ = 0.0f, az_ = 0.0f;
  float gx_ = 0.0f, gy_ = 0.0f, gz_ = 0.0f;
  float mx_ = 0.0f, my_ = 0.0f, mz_ = 0.0f;

  float q0_ = 1.0f, q1_ = 0.0f, q2_ = 0.0f, q3_ = 0.0f;

  float gxOffset_ = 0.0f, gyOffset_ = 0.0f, gzOffset_ = 0.0f;
  float axOffset_ = 0.0f, ayOffset_ = 0.0f, azOffset_ = 0.0f;

  float deltat_ = 0.0f;

  std::array<float, 3> m_{ 0.0f, 0.0f, 0.0f };
  std::array<float, 3> m_tmp{ 0.0f, 0.0f, 0.0f };
  std::array<float, 3> magnEllipsoidCenter_{ 0.0f, 0.0f, 0.0f };
  std::array<std::array<float, 3>, 3> magnEllipsoidTransform_{};

  int buttonValue_ = 0;
  int eepromTest = 0;
  int serialNumber_ = 0;
};

//Instância global
extern Giromin giromin;

#else
#error "This library only supports boards with ESP32 processor."
#endif

#endif
