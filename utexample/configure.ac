AC_PREREQ([2.69])
AC_INIT([unit-tests], [0.1],
        [http://www.github.com/openbmc])
AC_LANG([C++])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([subdir-objects -Wall -Werror foreign dist-xz])
AM_SILENT_RULES([yes])

# Checks for programs.
AC_PROG_CXX
AM_PROG_AR
AC_PROG_INSTALL

# Checks for typedefs, structures, and compiler characteristics.
AX_CXX_COMPILE_STDCXX_14([noext])
AX_APPEND_COMPILE_FLAGS([-Wall -Werror], [CXXFLAGS])

# For linking
LT_INIT

# Check for needed modules
PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 221], [], [AC_MSG_ERROR(["systemd required and not found"])])
PKG_CHECK_MODULES([SDBUSPLUS], [sdbusplus], ,[AC_MSG_ERROR([The openbmc/sdbusplus package is required])])
PKG_CHECK_MODULES([PHOSPHOR_DBUS_INTERFACES], [phosphor-dbus-interfaces], [], [AC_MSG_ERROR(["phosphor-dbus-interfaces required and not found."])])

# Check/set gtest specific functions.
PKG_CHECK_MODULES([GTEST], [gtest], [], [AC_MSG_NOTICE([gtest not found, tests will not build])])
AX_PTHREAD([GTEST_CFLAGS+=" -DGTEST_HAS_PTHREAD=1 "],[GTEST_CFLAGS+=" -DGTEST_HAS_PTHREAD=0 "])

# Add --enable-oe-sdk flag to configure script
AC_ARG_ENABLE([oe-sdk],
    AS_HELP_STRING([--enable-oe-sdk], [Link testcases absolutely against OE SDK so they can be ran within it.])
)

# Check for OECORE_TARGET_SYSROOT in the environment.
AC_ARG_VAR(OECORE_TARGET_SYSROOT,
    [Path to the OE SDK SYSROOT])

# Configure OESDK_TESTCASE_FLAGS environment variable, which will be later
# used in test/Makefile.am
AS_IF([test "x$enable_oe_sdk" == "xyes"],
    AS_IF([test "x$OECORE_TARGET_SYSROOT" == "x"],
          AC_MSG_ERROR([OECORE_TARGET_SYSROOT must be set with --enable-oe-sdk])
    )
    AC_MSG_NOTICE([Enabling OE-SDK at $OECORE_TARGET_SYSROOT])
    [
        testcase_flags="-Wl,-rpath,\${OECORE_TARGET_SYSROOT}/lib"
        testcase_flags="${testcase_flags} -Wl,-rpath,\${OECORE_TARGET_SYSROOT}/usr/lib"
        testcase_flags="${testcase_flags} -Wl,-dynamic-linker,`find \${OECORE_TARGET_SYSROOT}/lib/ld-*.so | sort -r -n | head -n1`"
    ]
    AC_SUBST([OESDK_TESTCASE_FLAGS], [$testcase_flags])
)

# Create configured output
AC_CONFIG_FILES([
  Makefile
  test/Makefile])
AC_OUTPUT
