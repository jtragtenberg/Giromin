#ifndef GirominController_h
#define GirominController_h

#include "Giromin.h"
#include "AXP192.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <string>

class GirominController {
public:
    GirominController(int serialNumber, const IPAddress& outIp, unsigned int outPort, 
                      const std::string& ssid, const std::string& pass, unsigned int localPort);

    void setup();
    void loop();

private:
    void configurePWM();
    void configureGiromin();
    void handleButton(Bounce& button, const std::string& suffix);
    void receiveOSCMessage();
    // void oscReset(OSCMessage &msg);

    int serialNumber_;
    IPAddress outIp_;
    unsigned int outPort_;
    std::string ssid_;
    std::string pass_;
    unsigned int localPort_;
    std::string basicTag_;
    AXP192 axp_;
    unsigned long lastLoopTime_ = 0;
    static constexpr unsigned long loopInterval_ = 10; // ms
};

#endif
