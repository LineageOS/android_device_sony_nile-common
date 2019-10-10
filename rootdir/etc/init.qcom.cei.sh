#!/vendor/bin/sh

setprop persist.radio.multisim.config "$(cat /proc/cei_simslot_id | xargs)"
setprop ro.vendor.cei_mainboard_id "$(cat /proc/cei_mainboard_id | xargs)"
setprop ro.vendor.cei_phase_id "$(cat /proc/cei_phase_id | xargs)"
setprop ro.vendor.cei_project_id "$(cat /proc/cei_project_id | xargs)"
setprop ro.vendor.cei_sku_id "$(cat /proc/cei_sku_id | xargs)"
setprop vendor.hw.fingerprint "$(cat /proc/cei_fp_id | xargs)"
