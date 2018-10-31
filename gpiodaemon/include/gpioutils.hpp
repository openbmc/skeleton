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

#pragma once

#include <experimental/filesystem>
#include <fstream>

enum class GpioValue
{
    low = 0,
    high
};

enum class GpioDirection
{
    in = 0,
    out,
    both
};

class Gpio
{
    std::string gpioNumber;

    bool gpioExist();
    void gpioExport();
    void gpioUnexport();

  public:
    Gpio(std::string number);
    Gpio(const Gpio&) = default;
    Gpio& operator=(const Gpio&) = default;

    GpioValue getValue();
    void setValue(const GpioValue& value);
    std::string getDirection();
    void setDirection(const std::string& direction);
};
