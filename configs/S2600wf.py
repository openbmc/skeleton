## System states
##   state can change to next state in 2 ways:
##   - a process emits a GotoSystemState signal with state name to goto
##   - objects specified in EXIT_STATE_DEPEND have started
SYSTEM_STATES = [
]

FRU_INSTANCES = {
        '<inventory_root>/system/chassis/motherboard/bmc' : { 'fru_type' : 'BMC','is_fru' : False, 'manufacturer' : 'ASPEED' },
}

GPIO_CONFIG = {}
GPIO_CONFIG['PGOOD']         = {'gpio_pin': 'AB3', 'direction': 'in'}
GPIO_CONFIG['POWER_BUTTON']  = {'gpio_pin': 'E2', 'direction': 'both'}
GPIO_CONFIG['POWER_UP_PIN']  = {'gpio_pin': 'E3', 'direction': 'out'}
GPIO_CONFIG['RESET_BUTTON']  = {'gpio_pin': 'E0', 'direction': 'both'}
GPIO_CONFIG['RESET_OUT']     = {'gpio_pin': 'E1', 'direction': 'out'}
GPIO_CONFIG['ID_BUTTON']     = {'gpio_pin': 'S6', 'direction': 'out'}


GPIO_CONFIGS = {
    'power_config' : {
        'power_good_in' : 'PGOOD',
        'power_up_outs' : [
            ('POWER_UP_PIN', True),
        ],
        'reset_outs' : [
            ('RESET_OUT', False),
        ],
        'pci_reset_outs': [
        ],
    },

    'hostctl_config' : {
        'fsi_data' : '',
        'fsi_clk' : '',
        'fsi_enable' : '',
        'cronus_sel' : '',
        'optionals' : [
        ],
    },
}
