#pragma once

#include <string>
#include <iostream>
namespace phosphor
{
namespace checkstop
{
/** @class Handler
 *  @brief Responsible for catching host checkstop
 *  condition and taking actions
 */
class Handler
{
    public:
        Handler() = delete;
        ~Handler() = default;
        Handler(const Handler&) = delete;
        Handler& operator=(const Handler&) = delete;
        Handler(Handler&&) = delete;
        Handler& operator=(Handler&&) = delete;

        /** @brief Constructs Handler object.
         *
         *  @param[in] device   - Path to gpio device device
         *  @param[in] line     - Line number in that gpio device
         */
        Handler(const std::string& device, uint32_t line)
            : device(device),
              line(line)
        {
            // Get the file descriptor for the passed in device
            getFD();
        }

    private:
        /** @brief Absolute path of GPIO device */
        std::string device;

        /** @brief Line number in the device */
        uint32_t line;

        /** @brief File descriptor for the gpio device */
        int fd = 0;

        /** @brief Opens the device and returns the descriptor */
        void getFD();
};

} // namespace checkstop
} // namespace phosphor
