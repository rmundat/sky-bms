#include "battery/src/battery.h"

void Battery::setBms(const std::shared_ptr<BMS>& bms) {
    this->bms = bms;
}

void Battery::init(int seriesCells, int parallelCells, int batteryModules, 
                    double startSoc, double ambientTemp, int power, int selection) {
    if (!isCrntChrgInit) {
        currentCharge = startSoc * 0.01;
        isCrntChrgInit = true;
    }  

    config.seriesCells = seriesCells;
    config.parallelCells = parallelCells;
    config.batteryModules = batteryModules;
    config.totalCells = seriesCells * parallelCells * batteryModules;
    
    storedSoc = startSoc;
    LookUpTable lutInterp(this->socValues, this->tempValues, this->voltageValues);
    double measureVoltage = lutInterp.lookup2d(startSoc, ambientTemp);
    measureVoltage = measureVoltage * seriesCells * batteryModules;

    measureCurrent = safeDivide(power, measureVoltage);

    batteryCapacity = MAX_CAP * parallelCells;

    if (selection == 1) charge(measureCurrent);
    else if (selection == 2) discharge(measureCurrent);  
    cool();   
    getVoltage(measureVoltage); 
    getTemperature(ambientTemp);   
}

ContactorState Battery::charge(int current) {                      
    if (currentCharge >= batteryCapacity) {
        currentCharge = batteryCapacity;
        return ContactorState::OPEN;
    }
    
    current = measureCurrent;
    _eta = calculateEfficiency();
    commonEnergyTransfer(current, _eta);

    currentTemperature *= 1 + (1 - _eta);
    // ...

    contactorState = ContactorState::CLOSE;
    return contactorState;
}

ContactorState Battery::discharge(int current) {
    if (currentCharge <= 0.01) {
        currentCharge = 0;
        return ContactorState::OPEN;
    }

    if (current > 0) current = -measureCurrent;
    _eta = 1.0;
    commonEnergyTransfer(current, _eta);

    contactorState = ContactorState::CLOSE;
    return contactorState;
}

ContactorState Battery::cool() {
    if (!bms) return  contactorState;
    ThermalFaultLevel thermalState = bms->getThermalState();

    if (thermalState == ThermalFaultLevel::CRITICAL) {
        return ContactorState::OPEN;
    } else if (thermalState == ThermalFaultLevel::HIGH) {
        currentTemperature -= tempDissipateAt100;
        measureCurrent *= 0.80;
    } else if (thermalState == ThermalFaultLevel::LOW) {
        currentTemperature -= tempDissipateAt50;
        measureCurrent *= 0.90;
    } 

    return ContactorState::CLOSE;
}

void Battery::getVoltage(double measureVoltage) {
    currentVoltage =  measureVoltage + deltaVoltage; // Vt = V0(SOC,T) − iR0 − V1
}

void Battery::getTemperature(double measureTemperature) {
    currentTemperature = measureTemperature + deltaTemperature;
}

double Battery::calculateEfficiency() {
    // Try: efficiency based on heat loss model
    factCharge = (currentCharge / batteryCapacity); 
    return (factCharge < 0.9) ? 1 : 1 - (factCharge - 0.9);
}

void Battery::commonEnergyTransfer(int current, double _eta) {
    double totalResistance = ((params.dcir * config.seriesCells * config.batteryModules) / config.parallelCells) * 0.001;
    currentCharge += current * (dt / 3600.0) * _eta; 
    deltaVoltage = current * totalResistance;
    // int totalCells = config.seriesCells * config.parallelCells * config.batteryModules;
    // deltaTemperature = ((std::pow(current, 2) * totalResistance * dt * 9.81) / (BATT_WEIGHT * totalCells * BATT_CP));
    deltaTemperature = (std::pow(current, 2) * totalResistance * dt);
}