#! /vendor/bin/sh

# For Power Off Charging mode only
cei_mainboard_id=`cat /proc/cei_mainboard_id`
setprop ro.cei_mainboard_id $cei_mainboard_id


