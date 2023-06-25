#pragma once

#include <iostream>
#include <algorithm>
#include <memory>  
#include "battery/src/battery.h"

enum class ThermalFaultLevel {
    NOFAULT, LOW, HIGH, CRITICAL
};

class Battery;

class BMS : public std::enable_shared_from_this<BMS> {
private:
    const double dt = DT_1s;
    int current;
    bool isSocInit = false;
    double streeringEffort = 0.2;
    double voltage;
    double chargeCurrent;
    double currentSoc = 0.0;
    double temperature;
    double deltaCharge;
    double deltaSoc;
    int coolantPumpDuty = 0;
    ThermalFaultLevel thermalFaultLevel;
    std::shared_ptr<Battery> battery;

    int drive(int power);
    void monitorTemperature();
    double estimateSOC(int current);

public:
    BMS() : thermalFaultLevel(ThermalFaultLevel::NOFAULT) {
        this->battery = nullptr;
    }
    ~BMS() {}

    void setBattery(const std::shared_ptr<Battery>& battery);
    int chargeTo(int percentage);
    int driveForward(int power);
    int turnLeft(int power);
    int turnRight(int power);
    ThermalFaultLevel getThermalState() const { return thermalFaultLevel; }
};

