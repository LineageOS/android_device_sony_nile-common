[AID_QCOM_DIAG]
value: 2950

[AID_RFS]
value: 2951

[AID_RFS_SHARED]
value: 2952

[AID_QNS]
value: 2985

[AID_IDD]
value: 2987

[AID_UPDATEMISCTA]
value: 2991

[AID_TRIMAREA]
value: 2993

[AID_PERSIST_RW]
value: 2995

[AID_CREDMGR_CLIENT]
value: 2996

[AID_TAD]
value: 2997

[AID_TA_QMI]
value: 2998

[firmware/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[bt_firmware/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[dsp/]
mode: 0771
user: AID_MEDIA
group: AID_MEDIA
caps: 0

[persist/]
mode: 0771
user: AID_SYSTEM
group: AID_SYSTEM
caps: 0

[vendor/bin/cnd]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE BLOCK_SUSPEND NET_ADMIN

[vendor/bin/cnss-daemon]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE NET_ADMIN

[vendor/bin/imsdatadaemon]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE

[vendor/bin/ims_rtp_daemon]
mode: 0755
user: AID_SYSTEM
group: AID_RADIO
caps: NET_BIND_SERVICE

[vendor/bin/imsrcsd]
mode: 0755
user: AID_SYSTEM
group: AID_RADIO
caps: WAKE_ALARM

[vendor/bin/pm-service]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE

[vendor/bin/pm-proxy]
mode: 0755
user: AID_SYSTEM
group: AID_SYSTEM
caps: NET_BIND_SERVICE

[vendor/bin/slim_daemon]
mode: 0755
user:  AID_GPS
group: AID_GPS
caps: NET_BIND_SERVICE

[vendor/bin/wcnss_filter]
mode: 0755
user: AID_BLUETOOTH
group: AID_BLUETOOTH
caps: BLOCK_SUSPEND SYS_NICE

[vendor/bin/xtwifi-client]
mode: 0755
user:  AID_GPS
group: AID_GPS
caps: NET_BIND_SERVICE BLOCK_SUSPEND

[vendor/bin/hw/android.hardware.wifi@1.0-service]
mode: 0755
user: AID_WIFI
group: AID_WIFI
caps: NET_ADMIN NET_RAW SYS_MODULE
