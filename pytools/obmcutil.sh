#!/bin/bash

USAGE="Usage: openbmc [command]

Available commands:
	bmcstate
	chassisstate
	hoststate
	bootprogress
	state
	power"

SERVICE_ROOT=xyz.openbmc_project
SERVICE_CONTROL=$SERVICE_ROOT.State

OBJECT_ROOT=/xyz/openbmc_project
OBJECT_CONTROL=$OBJECT_ROOT/state

call ()
{
	busctl $1 | cut -d '"' -f2
}

get_property ()
{
	call "get-property $1 $2 $3 $4"
}

set_property ()
{
	call "set-property $1 $2 $3 $4 $5"
}

state_query ()
{
	STATE=$(get_property $1 $2 $3 $4)
	printf "%-20s: %s\n" $PROPERTY $STATE
}

parse_arg ()
{
	case "$1" in
		bmcstate)
			OBJECT=$OBJECT_CONTROL/bmc0
			SERVICE=$(mapper get-service $OBJECT)
			INTERFACE=$SERVICE
			PROPERTY=CurrentBMCState
			state_query $SERVICE $OBJECT $INTERFACE $PROPERTY
			;;
		chassisstate)
			OBJECT=$OBJECT_CONTROL/chassis0
			SERVICE=$(mapper get-service $OBJECT)
			INTERFACE=$SERVICE
			PROPERTY=CurrentPowerState
			state_query $SERVICE $OBJECT $INTERFACE $PROPERTY
			;;
		hoststate)
			OBJECT=$OBJECT_CONTROL/host0
			SERVICE=$(mapper get-service $OBJECT)
			INTERFACE=$SERVICE
			PROPERTY=CurrentHostState
			state_query $SERVICE $OBJECT $INTERFACE $PROPERTY
			;;
		bootprogress)
			OBJECT=$OBJECT_CONTROL/host0
			SERVICE=$(mapper get-service $OBJECT)
			INTERFACE=$SERVICE_CONTROL.Boot.Progress
			PROPERTY=BootProgress
			state_query $SERVICE $OBJECT $INTERFACE $PROPERTY
			;;
		state|status)
			for query in bmcstate chassisstate hoststate
			do
				parse_arg $query
			done
			;;
		power)
			OBJECT=/org/openbmc/control/power0
			SERVICE=$(mapper get-service $OBJECT)
			INTERFACE=$SERVICE
			for property in pgood state pgood_timeout
			do
				STATE=$(get_property $SERVICE $OBJECT $INTERFACE $property | sed 's/i[ ^I]*//')
				printf "%s = %s\n" $property $STATE
			done
			;;
		*)
			echo "$USAGE"
			;;
	esac
}

parse_arg $1
