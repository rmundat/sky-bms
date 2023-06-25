#include "bms/src/bms.h"

void BMS::setBattery(const std::shared_ptr<Battery>& battery) {
    this->battery = battery;
    this->battery->setBms(shared_from_this());
}

int BMS::chargeTo(int percentage) {
    if (!battery) return 0;

    voltage = battery->getCurrentVoltage();
    chargeCurrent = battery->getCurrent();
    currentSoc = estimateSOC(chargeCurrent);

    if (battery->getContactorState() == ContactorState::OPEN) return 1;

    monitorTemperature();
    if (thermalFaultLevel != ThermalFaultLevel::NOFAULT) {
        battery->cool();
        if (thermalFaultLevel == ThermalFaultLevel::CRITICAL) return 2;
    }

    if (currentSoc >= percentage) return 3;

    battery->getVoltage(voltage);
    battery->charge(chargeCurrent);
    std::cout << "SOC: " << static_cast<int>(currentSoc) << "%\n"
        << "Charging at: " << std::abs(chargeCurrent * voltage * 0.001) << " kW\n"
        << "Battery is at: " << voltage << " V\n"
        << "Battery is charging at: " << chargeCurrent << " A\n"
        << "Battery Temperature: " << temperature << " degC\n";

    return 0;
}

int BMS::driveForward(int power) {
    return drive(power);
}

int BMS::turnLeft(int power) {
    power -= streeringEffort;
    return drive(power);
}

int BMS::turnRight(int power) {
    power -= streeringEffort;
    return drive(power);
}

int BMS::drive(int power) {
    if (!battery) return 0;

    voltage = battery->getCurrentVoltage();
    current = battery->getCurrent();
    power = power * 1000;
    int reqMotorCurrent = safeDivide(power, voltage);

    int driveCurrent = -(std::min(current, reqMotorCurrent));

    currentSoc = estimateSOC(driveCurrent);

    if (battery->getContactorState() == ContactorState::OPEN) return 1;

    monitorTemperature();
    if (thermalFaultLevel != ThermalFaultLevel::NOFAULT) {
        battery->cool();
        if (thermalFaultLevel == ThermalFaultLevel::CRITICAL) return 2;
    }

    if (currentSoc <= 0.1) return 3;

    battery->getVoltage(voltage);
    battery->discharge(driveCurrent);
    std::cout << "SOC: " << static_cast<int>(currentSoc) << "%\n"
        << "Driving at: " << std::abs(current * voltage * 0.001) << " kW\n"
        << "Battery is at: " << voltage << " V\n"
        << "Battery is discharging at: " << abs(driveCurrent) << " A\n"
        << "Battery Temperature: " << temperature << " degC\n";
        
    return 0; 
}

void BMS::monitorTemperature() {
    if (!battery) return;

    temperature = battery->getCurrentTemperature();
    battery->getTemperature(temperature);

    if (temperature > 55) {
        thermalFaultLevel = ThermalFaultLevel::CRITICAL;
    } else if (temperature > 45) {
        coolantPumpDuty = 100;
        thermalFaultLevel = ThermalFaultLevel::HIGH;
    } else if (temperature > 35) {
        coolantPumpDuty = 50;
        thermalFaultLevel = ThermalFaultLevel::LOW;
    }

    if (temperature < 28 && coolantPumpDuty > 0) {
        coolantPumpDuty = 0;
        thermalFaultLevel =ThermalFaultLevel::NOFAULT;
    }
}

double BMS::estimateSOC(int current) {
    if (!isSocInit && battery) {
        currentSoc = battery->getStoredSoc();
        isSocInit = true;
    }
    deltaCharge = current * (dt / 3600.0);
    deltaSoc = (deltaCharge / (battery->getBatteryCapacity() * 0.98)) * 100.0;
    currentSoc += deltaSoc;
    if(currentSoc > 100.0) currentSoc = 100.0; 
    else if(currentSoc < 0.0) currentSoc = 0.0;
    return currentSoc;
}