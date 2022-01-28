#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

blob_fixups: blob_fixups_user_type = {
    'system_ext/etc/init/dpmd.rc': blob_fixup()
        .regex_replace('/system/product/bin/', '/system/system_ext/bin/'),
    ('system_ext/etc/permissions/com.qti.dpmframework.xml', 'system_ext/etc/permissions/dpmapi.xml', 'system_ext/etc/permissions/qcrilhook.xml', 'system_ext/etc/permissions/telephonyservice.xml'): blob_fixup()
        .regex_replace('/product/framework/', '/system_ext/framework/'),
    'system_ext/etc/permissions/qti_libpermissions.xml': blob_fixup()
        .regex_replace('name="android.hidl.manager-V1.0-java', 'name="android.hidl.manager@1.0-java'),
    'system_ext/lib64/lib-imscamera.so': blob_fixup()
        .add_needed('libgui_shim.so'),
    'system_ext/lib64/lib-imsvideocodec.so': blob_fixup()
        .add_needed('libgui_shim.so'),
    'system_ext/lib64/libdpmframework.so': blob_fixup()
        .replace_needed('libhidltransport.so', 'libcutils_shim.so'),
    'vendor/bin/pm-service': blob_fixup()
        .add_needed('libutils-v33.so'),
    'vendor/bin/sony-modem-switcher': blob_fixup()
        .binary_regex_replace(b'/oem/modem-config/%s/modem.conf', b'/vendor/modemconf/%s/modem.conf')
        .binary_regex_replace(b'/oem/modem-config/modem.conf', b'/vendor/modemconf/modem.conf')
        .binary_regex_replace(b'persist.radio.multisim.config', b'vendor.radio.multisim.config\x00'),
    'vendor/etc/init/init.sony.idd.rc': blob_fixup()
        .regex_replace('restorecon_recursive --force', 'restorecon_recursive'),
    'vendor/etc/init/init.sony-modem-switcher.rc': blob_fixup()
        .regex_replace('/system/bin/sony-modem-switcher', '/vendor/bin/sony-modem-switcher'),
    ('vendor/lib/libbtnv.so', 'vendor/lib64/libbtnv.so'): blob_fixup()
        .binary_regex_replace(b'.bt_nv.bin', b'.bt_nv.noo'),
    ('vendor/lib/libwvhidl.so', 'vendor/lib64/libwvhidl.so'): blob_fixup()
        .add_needed('libcrypto_shim.so'),
    'vendor/lib64/com.fingerprints.extension@1.0.so': blob_fixup()
        .add_needed('libhidlbase_shim.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'nile-common',
    'sony',
    blob_fixups=blob_fixups,
    check_elf=False,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
