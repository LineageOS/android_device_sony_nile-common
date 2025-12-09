#!/usr/bin/env -S PYTHONPATH=../../../tools/extract-utils python3
#
# SPDX-FileCopyrightText: 2024 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0
#

from extract_utils.fixups_blob import (
    blob_fixup,
    blob_fixups_user_type,
)
from extract_utils.fixups_lib import (
    lib_fixups,
    lib_fixups_user_type,
)
from extract_utils.main import (
    ExtractUtils,
    ExtractUtilsModule,
)

namespace_imports = [
    'device/sony/nile-common',
    'hardware/qcom-caf/sdm660',
    'hardware/qcom-caf/wlan',
    'vendor/qcom/opensource/commonsys/display',
    'vendor/qcom/opensource/dataservices',
    'vendor/qcom/opensource/display',
]


def lib_fixup_vendor_suffix(lib: str, partition: str, *args, **kwargs):
    return f'{lib}_{partition}' if partition == 'vendor' else None


lib_fixups: lib_fixups_user_type = {
    **lib_fixups,
    (
        'com.qualcomm.qti.dpm.api@1.0',
        'com.qualcomm.qti.imscmservice@2.0',
        'com.qualcomm.qti.imscmservice@2.1',
        'com.qualcomm.qti.imscmservice@2.2',
        'vendor.qti.hardware.fm@1.0',
        'vendor.qti.imsrtpservice@2.0',
        'vendor.qti.imsrtpservice@2.1',
    ): lib_fixup_vendor_suffix,
}

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
    'vendor/bin/hw/android.hardware.drm@1.1-service.widevine': blob_fixup()
        .replace_needed('libhidltransport.so', 'libhidlbase.so')
        .remove_needed('libhwbinder.so'),
    'vendor/bin/sony-modem-switcher': blob_fixup()
        .binary_regex_replace(b'/oem/modem-config/%s/modem.conf', b'/vendor/modemconf/%s/modem.conf')
        .binary_regex_replace(b'/oem/modem-config/modem.conf', b'/vendor/modemconf/modem.conf')
        .binary_regex_replace(b'persist.radio.multisim.config', b'vendor.radio.multisim.config\x00'),
    'vendor/etc/init/android.hardware.gnss@2.1-service-qti.rc': blob_fixup()
        .regex_replace('    disabled', '    #disabled'),
    'vendor/etc/init/init.sony.idd.rc': blob_fixup()
        .regex_replace('restorecon_recursive --force', 'restorecon_recursive'),
    'vendor/etc/init/init.sony-modem-switcher.rc': blob_fixup()
        .regex_replace('/system/bin/sony-modem-switcher', '/vendor/bin/sony-modem-switcher'),
    ('vendor/etc/msm_irqbalance.conf', 'vendor/etc/msm_irqbalance_sdm630.conf'): blob_fixup()
        .regex_replace('IGNORED_IRQ=19,22,39,200,203$', 'IGNORED_IRQ=19,22,39,115,200,203,332')
        .regex_replace('BLACKLIST_IRQ=446,455,456$', 'BLACKLIST_IRQ=445,446,447,448,449,450,452,453,454,455,456,457'),
    ('vendor/lib/libbtnv.so', 'vendor/lib64/libbtnv.so'): blob_fixup()
        .binary_regex_replace(b'.bt_nv.bin', b'.bt_nv.noo'),
    ('vendor/lib/libwvhidl.so', 'vendor/lib64/libwvhidl.so'): blob_fixup()
        .add_needed('libcrypto_shim.so'),
    'vendor/lib/libznr.so': blob_fixup()
        .add_needed('liblog.so')
        .clear_symbol_version('__aeabi_memcpy')
        .clear_symbol_version('__aeabi_memset')
        .clear_symbol_version('__gnu_Unwind_Find_exidx'),
    'vendor/lib64/com.fingerprints.extension@1.0.so': blob_fixup()
        .add_needed('libhidlbase_shim.so'),
    'vendor/lib64/fpc_tac.so': blob_fixup()
        .replace_needed('libprotobuf-c.so', 'libprotobuf-c-idd.so'),
}  # fmt: skip

module = ExtractUtilsModule(
    'nile-common',
    'sony',
    blob_fixups=blob_fixups,
    lib_fixups=lib_fixups,
    namespace_imports=namespace_imports,
)

if __name__ == '__main__':
    utils = ExtractUtils.device(module)
    utils.run()
