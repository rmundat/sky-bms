# Battery Management System (BMS) Requirements Document

## Overview

BMS provides control and monitoring functionalities for an electric vehicle's battery. The primary functions of the BMS in this scode include battery parameter management during charging the battery, driving the vehicle forward, making turns, and monitoring the temperature of the battery.

## Assumptions

Steering motor is assumed to consume a constant 0.2kW of power during turn.

## Limitations

BMS functions are limited to assessment requirements. Potential improvments are suggested below as part of method requirment.

## Parameters

| Name                  | Unit      | Type   | Description                                           | Default value |
| :-------------------- | :---------| :----- | :---------------------------------------------------- | :------------ |
| current               | [A]       | int    | measured current                                      | -             |
| isSocInit             | [b]       | bool   | SOC read at initilization enable                      | false         |
| streeringEffort       | [kW]      | double | power consumed by steering                            | 0.2           |
| voltage               | [V]       | double | measured voltage                                      | -             |
| chargeCurrent         | [A]       | double | usable charge current                                 | -             |
| currentSoc            | [%]       | double | estimated SoC                                         | 0.0           |
| temperature           | [degC]    | double | measured temperature                                  | -             |
| deltaCharge           | [As]      | double | battery charge delta (internal)                       | -             |
| deltaSoc              | [%]       | double | battery SOC delta (internal)                          | -             |
| coolantPumpDuty       | [-]       | int    | commanded coolant pump duty cycle                     | 0             |
| thermalFaultLevel     | [-]       | struct | ThermalFaultLevel struct to capture fault state       | NOFAULT       |

## Methods

### 1. Battery Setting

Battery object is used to monitor and manipulate the battery's state. This is done through the setBattery function, which takes in a battery object as an argument.

```c++
void BMS::setBattery(const std::shared_ptr<Battery>& battery) {  
  this->battery = battery;  
  this->battery->setBms(shared_from_this());  
}
```

### 2. Battery Charging

#### `int BMS::chargeTo(int percentage)`

This method allows BMS to charge the battery to a specific state of charge (SOC) provided as a percentage, which takes in the desired SOC as an argument. The BMS also monitors the temperature during the charging process and cools the battery if a thermal fault is detected. The function returns different codes based on the result of the charging process indicating contactor, thermal and SOC states of the battery. This method simulates voltage, shunt/hall and termistor sensor measurements of battery.

This method can be further improved to aribrate the charge current based on battery internal states such as cell min, max and delta voltages, battery temperature, capacity/soc, I2T excess energy availability, etc. This method can also include the PID control lopp to control the requested current to the arbitrated setpoint during constant current/constant voltage to minimize oscillations due to battery state changes. This method can also inculde a extensive diagnostics stratergy, charging type determination, state transtions and communication protcols to establish charger handshake and energy transfer.

### 3. Vehicle Driving

#### `int BMS::drive(int power)`

This method allows BMS to discharge the battery to meet the power demands to drive the vehicle forward, turn left, and turn right. These are done through the driveForward, turnLeft, and turnRight functions, respectively. Each function takes in a power argument. The method includes usable current arbitration, battery state monitors including contactor states, temperature and charge level. Each function returns different codes based on the result of the driving process.This method simulates voltage, shunt/hall and termistor sensor measurements of battery.

This method can be further improved to aribrate the discharge long and short term current as stated in charge method above, to precicely maintain the motor torque or speed control demands at any given vehicle state based on accelerator pedal maps, vehicle speed and transmission setpoint maps (if powertrain supports multi-speed motor), inverter state change control during forward, reverse and regeneration braking, active discharge and shutdown phases, HV and LV component interactions and diagnostics to meet driving demands.

### 4. Temperature Monitoring

#### `void BMS::monitorTemperature()`

This method allows BMS to monitor the temperature of the battery. This is done through the monitorTemperature function. The BMS adjusts the duty of the coolant pump to control the flow rate and pressure for heat dissipation from the battery.

Temperature fault diagnostics sets the thermal fault level based on the current temperature and a hysteresis is provided to heal the any matured fault. The default temperature values are towards the conservative side since opening contactor at 55 degC is unecessary and is should be fine since it is typical for a battery to reach that temperature during I2T short term limiting.

### 5. SOC Estimation

#### `double BMS::estimateSOC(int current)`

This method provides the capability of estimating the current state of charge (SOC) of the battery based on the current draw. This is done through the estimateSOC function, which takes in the current as an argument for columb counting (intergral of accumulated charge over time). The function only initializes the SOC from the battery's stored SOC, simulating a NV store of the previous drive/charge cycle SOC.

A more advanced method of SOC estimation may involve a Kalman Filter with 1st or 2nd order battery model to estimate the battery SOC based on a combination of OCV-temperature maps and columb counting by calibrating the weights to account for diviations at various battery operating sates with columb counting taking precedence during steady state operation and OCV-temperature maps at initial and terminal states.

## Dependencies

The BMS depends on a Battery class with the following methods:

- setBms
- getCurrentVoltage
- getCurrent
- getContactorState
- cool
- getVoltage
- charge
- discharge
- getCurrentTemperature
- getTemperature
- getStoredSoc

The BMS also depends on the `ContactorState` and `ThermalFaultLevel` enumerations.

## Diagnotics and Fault Monitoring

### Error Codes

The `chargeTo` and `drive` functions return the following error codes:

`1`: Operation successful

`2`: Contactor state is open.

`3`: Critical thermal fault detected.

`4`: SOC is above the desired percentage (for charging) or SOC is below 10% (for driving).

Method does not include extensive rationality/plausbility checks, fault active, maturation, debounce, heal creteria.
