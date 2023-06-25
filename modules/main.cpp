#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include "utils/readcsv.h"
#include "battery/src/battery.h"
#include "bms/src/bms.h"

int main() {
    auto ocvSurface = readCSV("modules/data/ocv-soc.csv");
    auto soc = ocvSurface.xv;
    auto temps = ocvSurface.yv;
    auto ocv = ocvSurface.zv;

    std::shared_ptr<Battery> battery = std::make_shared<Battery>(soc, temps, ocv);

    // Get the starting SOC and ambient temperature 
    double startingSoc = 50.0, ambientTemp = 25.0;
    int seriesCells = 20, parallelCells = 10, batteryModules = 6;
    std::cout << "Starting SOC (default is 50%): ";
    std::string input;
    std::getline(std::cin, input);
    if (!input.empty()) startingSoc = std::stod(input);

    std::cout << "Ambient temperature (default is 25degC): ";
    std::getline(std::cin, input);
    if (!input.empty()) ambientTemp = std::stod(input);

    std::cout << "Number of series cells (default is 20): ";
    std::getline(std::cin, input);
    if (!input.empty()) seriesCells = std::stod(input);

    std::cout << "Number of parallel cells (default is 10): ";
    std::getline(std::cin, input);
    if (!input.empty()) parallelCells = std::stod(input);

    std::cout << "Number of battery modules (default is 6): ";
    std::getline(std::cin, input);
    if (!input.empty()) batteryModules = std::stod(input);

    std::shared_ptr<BMS> bms = std::make_shared<BMS>();
    bms->setBattery(battery);

    int bmsStatus = 0;
    int option;
    std::cout << "Enter action:\n"
              << "1. Charge\n"
              << "2. Drive\n"
              << "0. Quit\n";
    std::cin >> option;

    switch(option) {
        case 1: {
            std::cout << "Enter percentage to charge to: ";
            int percentage;
            std::cin >> percentage;
            percentage = std::clamp(percentage, 0, 100);
            std::cout << "Charging to: " << percentage << std::endl;
            
            std::cout << "Enter charger output rating in kW: ";
            int chargePower;
            std::cin >> chargePower;
            chargePower = std::clamp(chargePower, 0, 300) * 1000;
            std::cout << "Setting charger to: " << chargePower << "W" << std::endl;
            battery->init(seriesCells, parallelCells, batteryModules, startingSoc, ambientTemp, chargePower, 1);

            // Code without print runs <1ms, hence using a fixed 1 second sleep
            while (bmsStatus == 0) { 
                bmsStatus = bms->chargeTo(percentage);
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }

            if (bmsStatus == 1) std::cout << "Battery limits violated. Cannot charge further.\n"; 
            if (bmsStatus == 2) std::cout << "Battery over temperature - critical. Cannot charge further.\n"; 
            if (bmsStatus == 3) std::cout << "Charge complete.\n"; 
            break;
        }
        case 2: {
            std::cout << "Enter max drive power in kW: ";
            int drivePower;
            std::cin >> drivePower;
            drivePower = std::clamp(drivePower, 0, 300) * 1000;
            std::cout << "Max drive set to: " << drivePower << "W" << std::endl;
            battery->init(seriesCells, parallelCells, batteryModules, startingSoc, ambientTemp, drivePower, 2);

            std::ifstream file("modules/data/drive-profile.csv");
            std::string line;
            std::getline(file, line);
            std::stringstream ss(line);
            std::string command;
            
            // Code without print runs <1ms, hence using a fixed 1 second sleep
            while(std::getline(ss, command, ',') && bmsStatus == 0) {
                int power = std::min((std::stoi(command.substr(0, command.size()-1))) * 1000, drivePower);
                char direction = command.back();
                switch(direction) {
                    case 'F': 
                        bmsStatus = bms->driveForward(power);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        break;
                    case 'L':
                        bmsStatus = bms->turnLeft(power);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        break;
                    case 'R':
                        bmsStatus = bms->turnRight(power);
                        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                        break;
                }

                if (bmsStatus == 1) std::cout << "Battery limits violated. Cannot drive further.\n"; 
                else if (bmsStatus == 2) std::cout << "Battery over temperature - critical. Cannot drive further.\n"; 
                else if (bmsStatus == 3) std::cout << "Charge depleted. Please charge\n";  
                else std::cout << "End of driving.\n"; 
            }
            break;
        }
        case 0: {
            return 0;
        }
        default: {
            std::cout << "Invalid option.\n";
            return 1;
        }
    }
    return 0;
}