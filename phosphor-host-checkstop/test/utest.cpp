#include <iostream>
#include <sys/types.h>
#include <chrono>
#include <string>
#include <gtest/gtest.h>
#include "monitor.hpp"

using namespace phosphor::gpio;

// Exit helper. Ideally should be class but need
// this to be used inside a static method.
bool completed {};

class GpioTest : public ::testing::Test
{
    public:
        static constexpr auto DEVICE = "/tmp/test_fifo";

        // systemd event handler
        sd_event* events;

        // Really needed just for the constructor
        int line = 10;

        // Need this so that events can be initialized.
        int rc;

        // Source of event
        sd_event_source* eventSource = nullptr;

        // Gets called as part of each TEST_F construction
        GpioTest()
              : rc(sd_event_default(&events))
        {
            // Check for successful creation of event handler
            EXPECT_GE(rc, 0);

            // FIFO created to simulate data available
            EXPECT_EQ(0, mknod(DEVICE, S_IFIFO|0666, 0));
        }

        // Gets called as part of each TEST_F destruction
        ~GpioTest()
        {
            EXPECT_EQ(0, remove(DEVICE));

            events = sd_event_unref(events);
            EXPECT_EQ(events, nullptr);
        }

        // Callback handler on data
        static int callbackHandler(sd_event_source* es, int fd,
                                 uint32_t revents, void* userData)
        {
            std::cout <<"Event fired" << std::endl;
            completed = true;
            return 0;
        }
};

/** @brief Makes sure that event never comes for 3 seconds
 */
TEST_F(GpioTest, noEventIn3Seconds)
{
    using namespace std::chrono;

    Monitor gpio(DEVICE, line, events, callbackHandler);

    // Waiting 3 seconds and check if the completion status is set
    int count = 0;
    while(count < 3)
    {
        // Returns -0- on timeout and positive number on dispatch
        auto sleepTime = duration_cast<microseconds>(seconds(1));
        if(!sd_event_run(events, sleepTime.count()))
        {
            count++;
        }
    }
    EXPECT_EQ(false, completed);

    // 3 to cater to another uptick that happens prior to breaking.
    EXPECT_EQ(3, count);
}

/** @brief Pump data in the middle and expect the callback to be invoked */
TEST_F(GpioTest, pumpDataAndExpectCallBack)
{
    using namespace std::chrono;

    Monitor gpio(DEVICE, line, events, callbackHandler);

    // Pump the data in the middle
    int count = 0;
    while(count < 2 && !completed)
    {
        if (count == 1)
        {
            auto pumpData = std::string("echo 'foo' > ") + DEVICE;
            EXPECT_GE(0, system(pumpData.c_str()));
        }

        // Returns -0- on timeout
        auto sleepTime = duration_cast<microseconds>(seconds(1));
        if(!sd_event_run(events, sleepTime.count()))
        {
            count++;
        }
    }
    EXPECT_EQ(true, completed);
    EXPECT_EQ(1, count);
}
