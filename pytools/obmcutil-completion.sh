_obmcutil() {
    COMPREPLY=()
    cur=${COMP_WORDS[COMP_CWORD]}

    opts="bmcstate bootprogress chassiskill chassisoff chassison chassisstate hoststate power poweroff poweron state -h --help -v --verbose -w --wait"

    # complete -* with long options.
    COMPREPLY=($(compgen -W "$opts" -- $cur))
}
