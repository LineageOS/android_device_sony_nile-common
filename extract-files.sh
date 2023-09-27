#!/bin/bash
#
# Copyright (C) 2016 The CyanogenMod Project
# Copyright (C) 2017-2020 The LineageOS Project
#
# SPDX-License-Identifier: Apache-2.0
#

set -e

# Load extract_utils and do some sanity checks
MY_DIR="${BASH_SOURCE%/*}"
if [[ ! -d "${MY_DIR}" ]]; then MY_DIR="${PWD}"; fi

ANDROID_ROOT="${MY_DIR}/../../.."

HELPER="${ANDROID_ROOT}/tools/extract-utils/extract_utils.sh"
if [ ! -f "${HELPER}" ]; then
    echo "Unable to find helper script at ${HELPER}"
    exit 1
fi
source "${HELPER}"

# Default to sanitizing the vendor folder before extraction
CLEAN_VENDOR=true

ONLY_COMMON=
ONLY_TARGET=
KANG=
SECTION=

while [ "${#}" -gt 0 ]; do
    case "${1}" in
        --only-common )
                ONLY_COMMON=true
                ;;
        --only-target )
                ONLY_TARGET=true
                ;;
        -n | --no-cleanup )
                CLEAN_VENDOR=false
                ;;
        -k | --kang )
                KANG="--kang"
                ;;
        -s | --section )
                SECTION="${2}"; shift
                CLEAN_VENDOR=false
                ;;
        * )
                SRC="${1}"
                ;;
    esac
    shift
done

if [ -z "${SRC}" ]; then
    SRC="adb"
fi

function blob_fixup() {
    case "${1}" in
        product/lib64/libdpmframework.so)
            sed -i "s/libhidltransport.so/libcutils-v29.so\x00\x00\x00/" "${2}"
            ;;
        vendor/bin/qns|vendor/lib/libSonyIMX300PdafLibrary.so)
            "${PATCHELF}" --replace-needed "libstdc++.so" "libstdc++_vendor.so" "${2}"
            ;;
        vendor/bin/sony-modem-switcher)
            sed -i "s/\/oem\/modem-config\/%s\/modem.conf/\/vendor\/modemconf\/%s\/modem.conf/" "${2}"
            sed -i "s/\/oem\/modem-config\/modem.conf/\/vendor\/modemconf\/modem.conf/" "${2}"
            sed -i "s/persist.radio.multisim.config/vendor.radio.multisim.config\x00/" "${2}"
            ;;
        vendor/etc/init/init.sony.idd.rc)
            sed -i "s/restorecon_recursive --force/restorecon_recursive/g" "${2}"
            ;;
        vendor/etc/init/init.sony-modem-switcher.rc)
            sed -i "s/\/system\/bin\/sony-modem-switcher/\/vendor\/bin\/sony-modem-switcher/" "${2}"
            ;;
        vendor/lib/libbtnv.so)
            sed -i "s/.bt_nv.bin/.bt_nv.noo/" "${2}"
            ;;
        vendor/lib64/libbtnv.so)
            sed -i "s/.bt_nv.bin/.bt_nv.noo/" "${2}"
            ;;
        vendor/lib*/vendor.somc.hardware.security.secd@1.0.so)
            "${PATCHELF}" --replace-needed "libhidlbase.so" "libhidlbase-v29.so" "${2}"
            ;;
    esac
}

if [ -z "${ONLY_TARGET}" ]; then
    # Initialize the helper for common device
    setup_vendor "${DEVICE_COMMON}" "${VENDOR}" "${ANDROID_ROOT}" true "${CLEAN_VENDOR}"

    extract "${MY_DIR}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

if [ -z "${ONLY_COMMON}" ] && [ -s "${MY_DIR}/../${DEVICE}/proprietary-files.txt" ]; then
    # Reinitialize the helper for device
    source "${MY_DIR}/../${DEVICE}/extract-files.sh"
    setup_vendor "${DEVICE}" "${VENDOR}" "${ANDROID_ROOT}" false "${CLEAN_VENDOR}"

    extract "${MY_DIR}/../${DEVICE}/proprietary-files.txt" "${SRC}" "${KANG}" --section "${SECTION}"
fi

"${MY_DIR}/setup-makefiles.sh"
