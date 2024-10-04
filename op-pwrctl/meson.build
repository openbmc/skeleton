executable(
    'power_control.exe',
    'power_control_obj.c',
    include_directories: libopenbmc_intf_includes,
    link_with: libopenbmc_intf,
    dependencies: [gio_unix_dep, glib_dep],
    install: true,
)

subdir('pgood_wait')
