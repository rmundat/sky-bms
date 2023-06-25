#pragma once 

#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <memory>
#include "common/constants.h"
#include "utils/safedivide.h"
#include "utils/lut.h"
#include "bms/src/bms.h"


enum class ContactorState {
    OPEN,
    CLOSE,
};

struct BatteryParameters {
    const double minVoltage, maxVoltage;
    const double dcir;
};

struct BatteryPackConfig {
    int seriesCells;
    int parallelCells;
    int batteryModules;
    int totalCells;
};

class BMS; 

class Battery {
private:
    const double dt = DT_1s;  

    bool isCrntChrgInit = false;  
    const double tempDissipateAt50 = 1.5; 
    const double tempDissipateAt100 = 3.5; 
    int batteryCapacity;
    double _eta = 0.0; 
    double factCharge = 0.0;
    double deltaVoltage = 0.0;
    double deltaTemperature = 0.0;
    double measureCurrent;
    double currentVoltage;
    double currentTemperature;
    double storedSoc = 0.0; 
    double currentCharge = 0.0;
    double totalResistance = 0.0;
    std::vector<double> socValues;
    std::vector<double> tempValues;
    std::vector<std::vector<double>> voltageValues;
    BatteryParameters params;
    BatteryPackConfig config;
    std::shared_ptr<BMS> bms;

    double calculateEfficiency();

public:
    static constexpr int BATT_WEIGHT = 68;              // Battery weight in grams
    static constexpr int MAX_CAP = 5;                   // Battery capacity in Ah
    static constexpr double BATT_CP = 0.837;            // Specfic heat capacity in J/gC

    Battery(std::vector<double> socValues, std::vector<double> tempValues, std::vector<std::vector<double>> voltageValues) 
        : socValues(socValues), tempValues(tempValues), voltageValues(voltageValues), 
          contactorState(ContactorState::OPEN),
          params({
            .minVoltage = 2.92,
            .maxVoltage = 4.17,
            .dcir = 0.0245}) {}

    ~Battery() {}

    void setBms(const std::shared_ptr<BMS>& bms);
    void init(int seriesCells, int parallelCells, int batteryModules, 
                double startSoc, double ambientTemp, int current, int selection);
    ContactorState charge(int current);
    ContactorState discharge(int current);
    ContactorState cool();
    void getVoltage(double measureVoltage);
    void getTemperature(double tempmeasureTemperature);
    void commonEnergyTransfer(int current, double eta);

    // Getters for battery state
    double getCurrent() const { return measureCurrent;}
    double getStoredSoc() { return storedSoc; }
    double getCurrentVoltage() const { return currentVoltage; }
    double getCurrentTemperature() const { return currentTemperature; }
    int getBatteryCapacity() const { return batteryCapacity; }
    ContactorState contactorState;
    ContactorState getContactorState() const {return contactorState; }
};
