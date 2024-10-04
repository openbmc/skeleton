gnome = import('gnome')

openbmc_intf_src = gnome.gdbus_codegen(
    'openbmc_intf',
    sources: 'openbmc_intf.xml',
    interface_prefix: 'org.openbmc',
    object_manager: true,
)

libopenbmc_intf = library(
    'libopenbmc_intf',
    'gpio.c',
    'gpio_configs.c',
    'gpio_json.c',
    openbmc_intf_src,
    # The gdbus generated code cannot be compiled with -Wpedantic.
    c_args : '-Wno-pedantic',
    dependencies: [cjson_dep, gio_unix_dep, glib_dep],
    version: meson.project_version(),
    install: true,
)

libopenbmc_intf_includes = include_directories('.')

import('pkgconfig').generate(
    libopenbmc_intf,
    name: 'libopenbmc_intf',
    version: meson.project_version(),
    requires: [gio_unix_dep, glib_dep],
    description: '[deprecated] OpenBMC interface library from skeleton',
)
