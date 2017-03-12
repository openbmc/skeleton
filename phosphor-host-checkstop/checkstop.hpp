#pragma once

#include <string>
#include <systemd/sd-event.h>
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
         * @param[in] device   - Path to gpio device
         * @param[in] line     - Line number in that gpio device
         * @param[in] event    - sd_event handler
         * @param[in] handler  - IO callback handler
         */
        Handler(const std::string& device, uint32_t line,
                sd_event* event, sd_event_io_handler_t handler)
            : device(device),
              line(line),
              event(event),
              callbackHandler(handler)

        {
            // Get the file descriptor for the passed in device
            getFD();

            // And register callback handler when FD has some data
            registerCallback();
        }

        /** Callback handler when the FD has some activity on it
         *
         * @param[in] es       - Populated event source
         * @param[in] fd       - Associated File descriptor
         * @param[in] revent   - Type of event
         * @param[in] userData - User data that was passed during registration
         */
        static int processEvents(sd_event_source* es, int fd,
                                 uint32_t revents, void* userData);

    private:
        /** @brief Absolute path of device */
        const std::string& device;

        /** @brief Line number in the device */
        uint32_t line;

        /** @brief File descriptor for the device */
        int fd;

        /** @brief Handler to sd_event */
        sd_event *event;

        /** @brief event source */
        sd_event_source* eventSource = nullptr;

        /** @brief Callback handler when the FD has some data */
        sd_event_io_handler_t callbackHandler;

        /** @brief Opens the device and returns the descriptor */
        void getFD();

        /** @brief attaches FD to events and sets up callback handler */
        void registerCallback();
};

} // namespace checkstop
} // namespace phosphor
