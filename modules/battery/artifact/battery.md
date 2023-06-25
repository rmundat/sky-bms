# Battery Model Requirements Document

## Overview

This module defines the behavior of the `Battery` class. The `Battery` class simulates the behavior of a battery system and provides feedback to its associated Battery Management System (BMS). The primary functions of the BMS in this scode include battery parameter management during charging the battery, driving the vehicle forward, making turns, and monitoring the temperature of the battery.

## Assumptions

>If the battery charge level goes below the minimum charge threshold, the battery should no longer accept the `charge` command.

- Was assumed as: If the battery charge level goes below the minimum charge threshold, the battery should no longer accept the `discharge` command.

- Battery model is capable of contactor control, to prevent overheating of battery due to excess charge instead of a dedicated BMS function for contactor control during normal and emergency operations. A linear relation between efficiency and temperature was assumed, when in-fact it is very non-linear.

- Assuming charge dependeny on current and hence changed function charge(int power): to charge(int current): and discharge(int power): to discharge(int current): to avoid repetitive conversion.

- Assuming coolant pump heat dissipation and current consumption to be an arbitarty fixed numbers. Modeling pump power consumption based on pressure and flow characteristics is beyond the scope of this assignment.

## Limitations

BMS functions are limited to assignment requirements. Potential improvments are suggested below as part of method requirment.

## Parameters

| Name                  | Unit      | Type   | Description                                                      | Default value |
| :-------------------- | :---------| :----- | :----------------------------------------------------------------| :------------ |
| isCrntChrgInit        | [b]       | bool   | battery capacity actual at start intialization                   | -             |
| tempDissipateAt50     | [degC]    | double | assumed heat dissipation at 50% coolant pump duty cycle          | 1.5           |
| tempDissipateAt100    | [degC]    | double | assumed heat dissipation at 50% coolant pump duty cycle          | 3.5           |
| batteryCapacity       | [Ah]      | int    | combined total capacity all parallel cells                       | -             |
| _eta                  | [-]       | double | efficency factor                                                 | 0.0           |
| factCharge            | [-]       | double | charge factor between current and total                          | 0.0           |
| deltaVoltage          | [V]       | double | calculated voltage increment                                     | 0.0           |
| deltaTemperature      | [degC]    | double | calculated temperature increment                                 | 0.0           |
| measureCurrent        | [A]       | double | measured battery in/out current                                  | -             |
| currentVoltage        | [V]       | double | simulating voltage measure                                       | -             |
| currentTemperature    | [degC]    | double | simulating temperature measure                                   | -             |
| storedSoc             | [%]       | double | simulated SOC store                                              | 0.0           |
| currentCharge         | [Ah]      | double | simulated available battery capacity                             | 0.0           |
| totalResistance       | [mOhm]    | double | combined resistance of battery cells                             | 0.0           |
| socValues             | [%]       | double | x-axis array of SOC values                                       | -             |
| tempValues            | [degC]    | double | y-axis array of temperature values                               | -             |
| voltageValues         | [V]       | double | z-axis table data                                                | -             |
| params                | [-]       | double | BatteryParameters struct capturing battery internal parameters   | -             |
| config                | [-]       | double | BatteryPackConfig struct capturing battery cell configuration    | -             |

## Methods

### 1. Battery Initialization

#### `void Battery::init(int seriesCells, int parallelCells, int batteryModules, double startSoc, double ambientTemp, int power, int selection)`

This method initializes a `Battery` object. It sets up the battery configuration based on the number of series cells, parallel cells, and battery modules. It also sets the initial state of charge (`startSoc`), ambient temperature (`ambientTemp`), and power. The selection parameter decides whether the battery should be charged or discharged upon initialization.

### 2. Battery Charge Capacity

#### `ContactorState Battery::charge(int current)`

This method simulates the charge addition into the battery. It checks if the current charge is equal to or exceeds the battery capacity. If the charge is less, it adjusts the current charge by using the provided current and an efficiency factor calculated by the `calculateEfficiency` method. As cell charge is nearing its max capacity charging temperature increases due to excessive heat generation decreasing efficiency. This method assumes efficiency and temperature are linearly related.

This method allows for contactor control depending on current charge capacity. This method can be further improved with dedicated function for conactor control within the BMS class that can command the contactor states `open`, `precharge` and `close` of the battery taking into accout various factors along with battery capacity monotoring to prevent excess temperature increase along with timeouts.

#### `ContactorState Battery::discharge(int current)`

This method simulates the discharging of the battery. If the current charge drops to or below a certain threshold, it prevents further discharging. Otherwise, it decreases the charge using the provided current and an efficiency factor calculated by the `calculateEfficiency` method, which assumed be always efficient with no heat loss during discharging.

#### `double Battery::calculateEfficiency()`

This method calculates the efficiency of the battery based on its current charge and maximum capacity.

#### `void Battery::commonEnergyTransfer(int current, double _eta)`

This method carries out common computations for both the charging and discharging process. It adjusts the current charge, calculates the change in voltage as V = IR and change in temperature as I2R.

Temperature change is assumed to be only affected by Joule's heating and temperature rise based on heat capacity has been omitted, since would require specific heat capacity and weights of battery components to be known.

### 3. Battery Cooling

#### `ContactorState Battery::cool()`

This method simulates the cooling process of the battery. It adjusts the temperature and current depending on the thermal state retrieved from the `BMS` object.

This method assumes coolant pump consumes a fixed current of 10% at 50% duty cycle and 20% at 100% duty cycle.

### 4. Battery Measurments

#### `void Battery::getVoltage(double measureVoltage)`

This method simulates voltage sensor measurments of the battery.

#### `void Battery::getTemperature(double measureTemperature)`

This method simulates the termistor measurments of the battery.
