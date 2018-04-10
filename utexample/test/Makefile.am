AM_CPPFLAGS = -I$(top_srcdir)/ $(GTEST_CFLAGS)
AM_CXXFLAGS = $(PTHREAD_CFLAGS) $(GTEST_CFLAGS) $(SDBUSPLUS_CFLAGS)
AM_LDFLAGS = $(GTEST_LIBS) $(PTHREAD_LIBS) -lgtest -lgmock $(OESDK_TESTCASE_FLAGS) $(SDBUSPLUS_LIBS)

# Run all 'check' test programs
check_PROGRAMS = reboot_unittest mac_unittest
TESTS = $(check_PROGRAMS)

# Build/add unit-tests.
reboot_unittest_SOURCES = reboot_unittest.cpp
reboot_unittest_LDADD = $(top_builddir)/reboot.o

mac_unittest_SOURCES = mac_unittest.cpp
mac_unittest_LDADD = $(top_builddir)/mac.o
