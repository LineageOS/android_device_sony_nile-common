/*
 * Copyright (C) 2018 The LineageOS Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "LightService"

#include "Lights.h"

#include <android-base/logging.h>

namespace {
static constexpr int RAMP_SIZE = 8;
static constexpr int RAMP_STEP_DURATION = 50;

static constexpr int BRIGHTNESS_RAMP[RAMP_SIZE] = {0, 12, 25, 37, 50, 72, 85, 100};
static constexpr int DEFAULT_MAX_BRIGHTNESS = 255;

static uint32_t rgbToBrightness(const HwLightState& state) {
    uint32_t color = state.color & 0x00ffffff;
    return ((77 * ((color >> 16) & 0xff)) + (150 * ((color >> 8) & 0xff)) +
            (29 * (color & 0xff))) >> 8;
}

static bool isLit(const HwLightState& state) {
    return (state.color & 0x00ffffff);
}

static std::string getScaledDutyPcts(int brightness) {
    std::string buf, pad;

    for (auto i : BRIGHTNESS_RAMP) {
        buf += pad;
        buf += std::to_string(i * brightness / 255);
        pad = ",";
    }

    return buf;
}
}  // anonymous namespace

namespace aidl {
namespace android {
namespace hardware {
namespace light {

Lights::Lights(std::pair<std::ofstream, uint32_t>&& lcd_backlight,
               std::ofstream&& red_led, std::ofstream&& green_led, std::ofstream&& blue_led,
               std::ofstream&& red_duty_pcts, std::ofstream&& green_duty_pcts, std::ofstream&& blue_duty_pcts,
               std::ofstream&& red_start_idx, std::ofstream&& green_start_idx, std::ofstream&& blue_start_idx,
               std::ofstream&& red_pause_lo, std::ofstream&& green_pause_lo, std::ofstream&& blue_pause_lo,
               std::ofstream&& red_pause_hi, std::ofstream&& green_pause_hi, std::ofstream&& blue_pause_hi,
               std::ofstream&& red_ramp_step_ms, std::ofstream&& green_ramp_step_ms, std::ofstream&& blue_ramp_step_ms,
               std::ofstream&& red_blink, std::ofstream&& green_blink, std::ofstream&& blue_blink,
               std::ofstream&& rgb_blink)
    : mLcdBacklight(std::move(lcd_backlight)),
      mRedLed(std::move(red_led)),
      mGreenLed(std::move(green_led)),
      mBlueLed(std::move(blue_led)),
      mRedDutyPcts(std::move(red_duty_pcts)),
      mGreenDutyPcts(std::move(green_duty_pcts)),
      mBlueDutyPcts(std::move(blue_duty_pcts)),
      mRedStartIdx(std::move(red_start_idx)),
      mGreenStartIdx(std::move(green_start_idx)),
      mBlueStartIdx(std::move(blue_start_idx)),
      mRedPauseLo(std::move(red_pause_lo)),
      mGreenPauseLo(std::move(green_pause_lo)),
      mBluePauseLo(std::move(blue_pause_lo)),
      mRedPauseHi(std::move(red_pause_hi)),
      mGreenPauseHi(std::move(green_pause_hi)),
      mBluePauseHi(std::move(blue_pause_hi)),
      mRedRampStepMs(std::move(red_ramp_step_ms)),
      mGreenRampStepMs(std::move(green_ramp_step_ms)),
      mBlueRampStepMs(std::move(blue_ramp_step_ms)),
      mRedBlink(std::move(red_blink)),
      mGreenBlink(std::move(green_blink)),
      mBlueBlink(std::move(blue_blink)),
      mRgbBlink(std::move(rgb_blink)) {
    auto attnFn(std::bind(&Lights::setAttentionLight, this, std::placeholders::_1));
    auto backlightFn(std::bind(&Lights::setLcdBacklight, this, std::placeholders::_1));
    auto batteryFn(std::bind(&Lights::setBatteryLight, this, std::placeholders::_1));
    auto notifFn(std::bind(&Lights::setNotificationLight, this, std::placeholders::_1));
    mLights.emplace(std::make_pair(LightType::ATTENTION, attnFn));
    mLights.emplace(std::make_pair(LightType::BACKLIGHT, backlightFn));
    mLights.emplace(std::make_pair(LightType::BATTERY, batteryFn));
    mLights.emplace(std::make_pair(LightType::NOTIFICATIONS, notifFn));
}

// Methods from ::aidl::android::hardware::light::BnLights follow.
ndk::ScopedAStatus Lights::setLightState(int32_t id, const HwLightState& state) {
    auto it = mLights.find(static_cast<LightType>(id));

    if (it == mLights.end()) {
        return ndk::ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }

    it->second(state);

    return ndk::ScopedAStatus::ok();
}

#define AutoHwLight(light) {.id = (int32_t)light, .type = light, .ordinal = 0}

ndk::ScopedAStatus Lights::getLights(std::vector<HwLight> *_aidl_return) {
    for (auto const& light : mLights) {
        _aidl_return->push_back(AutoHwLight(light.first));
    }

    return ndk::ScopedAStatus::ok();
}

void Lights::setAttentionLight(const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mAttentionState = state;
    setSpeakerBatteryLightLocked();
}

void Lights::setLcdBacklight(const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);

    uint32_t brightness = rgbToBrightness(state);

    // If max panel brightness is not the default (255),
    // apply linear scaling across the accepted range.
    if (mLcdBacklight.second != DEFAULT_MAX_BRIGHTNESS) {
        int old_brightness = brightness;
        brightness = brightness * mLcdBacklight.second / DEFAULT_MAX_BRIGHTNESS;
        LOG(VERBOSE) << "scaling brightness " << old_brightness << " => " << brightness;
    }

    mLcdBacklight.first << brightness << std::endl;
}

void Lights::setBatteryLight(const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mBatteryState = state;
    setSpeakerBatteryLightLocked();
}

void Lights::setNotificationLight(const HwLightState& state) {
    std::lock_guard<std::mutex> lock(mLock);
    mNotificationState = state;
    setSpeakerBatteryLightLocked();
}

void Lights::setSpeakerBatteryLightLocked() {
    if (isLit(mNotificationState)) {
        setSpeakerLightLocked(mNotificationState);
    } else if (isLit(mAttentionState)) {
        setSpeakerLightLocked(mAttentionState);
    } else if (isLit(mBatteryState)) {
        setSpeakerLightLocked(mBatteryState);
    } else {
        // Lights off
        mRedLed << 0 << std::endl;
        mGreenLed << 0 << std::endl;
        mBlueLed << 0 << std::endl;
        mRedBlink << 0 << std::endl;
        mGreenBlink << 0 << std::endl;
        mBlueBlink << 0 << std::endl;
    }
}

void Lights::setSpeakerLightLocked(const HwLightState& state) {
    int red, green, blue, blink;
    int onMs, offMs, stepDuration, pauseHi;
    uint32_t alpha;

    // Extract brightness from AARRGGBB
    alpha = (state.color >> 24) & 0xff;

    // Retrieve each of the RGB colors
    red = (state.color >> 16) & 0xff;
    green = (state.color >> 8) & 0xff;
    blue = state.color & 0xff;

    // Scale RGB colors if a brightness has been applied by the user
    if (alpha > 0 && alpha < 255) {
        red = (red * alpha) / 0xff;
        green = (green * alpha) / 0xff;
        blue = (blue * alpha) / 0xff;
    }

    switch (state.flashMode) {
        case FlashMode::TIMED:
            onMs = state.flashOnMs;
            offMs = state.flashOffMs;
            break;
        case FlashMode::NONE:
        default:
            onMs = 0;
            offMs = 0;
            break;
    }
    blink = onMs > 0 && offMs > 0;

    // Disable all blinking to start
    mRgbBlink << 0 << std::endl;

    if (blink) {
        stepDuration = RAMP_STEP_DURATION;
        pauseHi = onMs - (stepDuration * RAMP_SIZE * 2);

        if (stepDuration * RAMP_SIZE * 2 > onMs) {
            stepDuration = onMs / (RAMP_SIZE * 2);
            pauseHi = 0;
        }

        // Red
        mRedStartIdx << 0 << std::endl;
        mRedDutyPcts << getScaledDutyPcts(red) << std::endl;
        mRedPauseLo << offMs << std::endl;
        mRedPauseHi << pauseHi << std::endl;
        mRedRampStepMs << stepDuration << std::endl;

        // Green
        mGreenStartIdx << RAMP_SIZE << std::endl;
        mGreenDutyPcts << getScaledDutyPcts(green) << std::endl;
        mGreenPauseLo << offMs << std::endl;
        mGreenPauseHi << pauseHi << std::endl;
        mGreenRampStepMs << stepDuration << std::endl;

        // Blue
        mBlueStartIdx << RAMP_SIZE * 2 << std::endl;
        mBlueDutyPcts << getScaledDutyPcts(blue) << std::endl;
        mBluePauseLo << offMs << std::endl;
        mBluePauseHi << pauseHi << std::endl;
        mBlueRampStepMs << stepDuration << std::endl;

        // Start the party
        mRgbBlink << 1 << std::endl;
    } else {
        if (red == 0 && green == 0 && blue == 0) {
            mRedBlink << 0 << std::endl;
            mGreenBlink << 0 << std::endl;
            mBlueBlink << 0 << std::endl;
        }
        mRedLed << red << std::endl;
        mGreenLed << green << std::endl;
        mBlueLed << blue << std::endl;
    }
}

}  // namespace light
}  // namespace hardware
}  // namespace android
}  // namespace aidl
