#!/vendor/bin/sh

MSIM_DEVICES=(
    H4113 H4133 # XA2
    H4213 H4233 # XA2 Ultra
)
MSIM_DEVICE=0

for device in "${MSIM_DEVICES[@]}"; do
    if grep -q "Model: ${device}" /dev/block/bootdevice/by-name/LTALabel; then
        MSIM_DEVICE=1
        break
    fi
done

if [[ "${MSIM_DEVICE}" -eq 1 ]]; then
    setprop ro.vendor.radio.multisim.config dsds
else
    setprop ro.vendor.radio.multisim.config ss
fi
