/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <gpioutils.hpp>

static constexpr const char* pathExport = "/sys/class/gpio/export";
static constexpr const char* pathUnexport = "/sys/class/gpio/unexport";
static constexpr const char* prefix = "/sys/class/gpio/gpio";
static constexpr const char* postfixValue = "/value";
static constexpr const char* postfixDirection = "/direction";

Gpio::Gpio(std::string number) : gpioNumber(number)
{
    gpioExport();
}

bool Gpio::gpioExist()
{
    return std::experimental::filesystem::exists(prefix + gpioNumber);
}

void Gpio::gpioExport()
{
    if (!gpioExist())
    {
        std::fstream gpioExport;
        gpioExport.open(pathExport, std::ios::out);
        if (gpioExport.good())
        {
            gpioExport << gpioNumber;
        }
    }
}

void Gpio::gpioUnexport()
{
    if (gpioExist())
    {
        std::fstream gpioExport;
        gpioExport.open(pathUnexport, std::ios::out);
        if (gpioExport.good())
        {
            gpioExport << gpioNumber;
        }
    }
}

GpioValue Gpio::getValue()
{
    std::fstream gpioFile;
    std::string gpioValue;
    std::string gpioFilePath = prefix + gpioNumber + postfixValue;

    gpioFile.open(gpioFilePath, std::ios::in);
    if (gpioFile.good())
    {
        gpioFile >> gpioValue;
    }

    return gpioValue == "0" ? GpioValue::low : GpioValue::high;
}

void Gpio::setValue(const GpioValue& value)
{
    std::fstream gpioFile;
    std::string gpioFilePath = prefix + gpioNumber + postfixValue;

    setDirection("out");

    gpioFile.open(gpioFilePath, std::ios::out);
    if (gpioFile.good())
    {
        gpioFile << std::to_string((uint8_t)value);
    }
}

std::string Gpio::getDirection()
{
    std::fstream gpioFile;
    std::string gpioFilePath = prefix + gpioNumber + postfixDirection;
    std::string direction;

    gpioFile.open(gpioFilePath, std::ios::in);
    if (gpioFile.good())
    {
        gpioFile >> direction;
    }

    return direction;
}

void Gpio::setDirection(const std::string& direction)
{
    std::fstream gpioFile;
    std::string gpioFilePath = prefix + gpioNumber + postfixDirection;

    gpioFile.open(gpioFilePath, std::ios::out);
    if (gpioFile.good())
    {
        gpioFile << direction;
    }
    gpioFile.close();
}
