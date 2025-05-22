#include "GirominController.h"

constexpr int SERIAL_NUMBER = 29;
const IPAddress outIp(192, 168, 0, 140);
constexpr unsigned int outPort = 1333;
constexpr unsigned int localPort = 9000;

const std::string ssid = "giromin0";
const std::string pass = "dervishmaria";

GirominController controller(SERIAL_NUMBER, outIp, outPort, ssid, pass, localPort);

void setup() {
  controller.setup();
}

void loop() {
  controller.loop();
}
