/*
   Copyright (c) 2013, The Linux Foundation. All rights reserved.
   Copyright (C) 2018 The LineageOS Project.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials provided
      with the distribution.
    * Neither the name of The Linux Foundation nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
   WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
   ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
   BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
   IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <fstream>
#include <map>
#include <string>
#include <unistd.h>
#include <vector>

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#include "vendor_init.h"

constexpr auto LTALABEL_PATH = "/dev/block/bootdevice/by-name/LTALabel";

// copied from build/tools/releasetools/ota_from_target_files.py
// but with "." at the end and empty entry
std::vector<std::string> ro_product_props_default_source_order = {
    "",
    "product.",
    "product_services.",
    "odm.",
    "vendor.",
    "system.",
};

void property_override(char const prop[], char const value[], bool add = true)
{
    auto pi = (prop_info *) __system_property_find(prop);

    if (pi != nullptr) {
        __system_property_update(pi, value, strlen(value));
    } else if (add) {
        __system_property_add(prop, strlen(prop), value, strlen(value));
    }
}

void vendor_load_properties()
{
    if (access("/system/bin/recovery", F_OK) == 0) {
        return;
    }

    if (std::ifstream file = std::ifstream(LTALABEL_PATH, std::ios::binary)) {
        std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        size_t offset = str.find("Model: ");

        if (offset != std::string::npos) {
            std::string model = str.substr(offset + strlen("Model: "), 5);

            const auto set_ro_product_prop = [](const std::string &source,
                    const std::string &prop, const std::string &value) {
                auto prop_name = "ro.product." + source + prop;
                property_override(prop_name.c_str(), value.c_str(), false);
            };

            for (const auto &source : ro_product_props_default_source_order) {
                set_ro_product_prop(source, "device", model);
                set_ro_product_prop(source, "model", model);
                set_ro_product_prop(source, "name", model);
            }
        }
    }
}
