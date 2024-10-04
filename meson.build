project(
    'skeleton',
    'c',
    version: '1.0',
    default_options: [
        'buildtype=debugoptimized',
        'warning_level=2',
        'werror=true',
    ],
)

cjson_dep = dependency('libcjson', fallback: 'cjson')
gio_unix_dep = dependency('gio-unix-2.0')
glib_dep = dependency('glib-2.0')
libsystemd_dep = dependency('libsystemd')

subdir('libopenbmc_intf')

subdir('op-pwrctl')
subdir('op-hostctl')
