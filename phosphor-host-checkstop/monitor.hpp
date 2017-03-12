#pragma once

#include <string>
#include <iostream>
namespace phosphor
{
namespace gpio
{
/** @class Monitor
 *  @brief Responsible for catching GPIO state change
 *  condition and taking actions
 */
class Monitor
{
    public:
        Monitor() = delete;
        ~Monitor() = default;
        Monitor(const Monitor&) = delete;
        Monitor& operator=(const Monitor&) = delete;
        Monitor(Monitor&&) = delete;
        Monitor& operator=(Monitor&&) = delete;

        /** @brief Constructs Monitor object.
         *
         *  @param[in] path     - Path to gpio input device
         *  @param[in] line     - GPIO state to monitor
         */
        Monitor(const std::string& path, uint32_t state)
            : path(path),
              state(state)
        {
            // Populate the file descriptor for the passed in device
            openDevice();
        }

    private:
        /** @brief Absolute path of GPIO input device */
        const std::string& path;

        /** @brief GPIO state that is of interest */
        uint32_t state;

        /** @brief File descriptor for the gpio input device */
        int fd = 0;

        /** @brief Opens the device and populates the descriptor */
        void openDevice();
};

} // namespace gpio
} // namespace phosphor
