#pragma once

#include <string>
#include <systemd/sd-event.h>
#include <sdbusplus/bus.hpp>
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
              callbackHandler(handler),
              bus(sdbusplus::bus::new_default())

        {
            // Get the file descriptor for the passed in device
            getFD();

            // And register callback handler when FD has some data
            registerCallback();
        }

        ~Handler()
        {
            eventSource = sd_event_source_unref(eventSource);
        }

        /** Callback handler when the FD has some activity on it
         *
         * @param[in] es       - Populated event source
         * @param[in] fd       - Associated File descriptor
         * @param[in] revent   - Type of event
         * @param[in] userData - User data that was passed during registration
         *
         * @return             - 0 or positive number on success and negative
         *                       errno otherwise
         */
        static int processEvents(sd_event_source* es, int fd,
                                 uint32_t revents, void* userData);

        /** @brief Returns the completion state of this handler */
        inline auto isCompleted()
        {
            return completed;
        }

    private:
        /** @brief Absolute path of device */
        const std::string& device;

        /** @brief Line number in the device */
        uint32_t line;

        /** @brief File descriptor for the gpio device */
        int fd = 0;

        /** @brief Handler to sd_event */
        sd_event *event;

        /** @brief event source */
        sd_event_source* eventSource = nullptr;

        /** @brief Callback handler when the FD has some data */
        sd_event_io_handler_t callbackHandler;

        /** @brief sdbusplus handler */
        sdbusplus::bus::bus bus;

        /** @brief Completion indicator */
        bool completed = false;

        /** @brief Opens the device and populates the descriptor */
        void getFD();

        /** @brief attaches FD to events and sets up callback handler */
        void registerCallback();

        /** @brief Given the dbus path and interface, returns service name
         *
         *  @param[in] objPath      - Dbus Object Path
         *  @param[in] interface    - Dbus interface
         *
         *  @return                 - Service name or empty string
         */
        std::string getServiceName(const std::string& objPath,
                                   const std::string& interface);

        /** @brief Analyzes the GPIO event and starts Host Quiesce target
         *  if the GPIO is asserted and Host is already in Running state.
         *
         *  @return - For now, returns zero
         */
        int analyzeEvent();
};

} // namespace checkstop
} // namespace phosphor
