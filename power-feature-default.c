/*
 * Copyright (C) 2015 The CyanogenMod Project
 * Copyright (C) 2016 The Paranoid Android Project
 * Copyright (C) 2018 CypherOS
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

#include <stdlib.h>

#include <hardware/power.h>
#include "power-common.h"
#include "power-feature.h"
#include "utils.h"

void set_device_specific_feature(feature_t feature, int state)
{
    switch (feature) {
#ifdef GESTURES_NODE
        case POWER_FEATURE_GESTURES:
            sysfs_write(GESTURES_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DOUBLE_TAP_TO_WAKE_NODE
        case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
            sysfs_write(DOUBLE_TAP_TO_WAKE_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_V_NODE
        case POWER_FEATURE_DRAW_V:
            sysfs_write(DRAW_V_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_INVERSE_V_NODE
        case POWER_FEATURE_DRAW_INVERSE_V:
            sysfs_write(DRAW_INVERSE_V_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_O_NODE
        case POWER_FEATURE_DRAW_O:
            sysfs_write(DRAW_O_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_M_NODE
        case POWER_FEATURE_DRAW_M:
            sysfs_write(DRAW_M_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_W_NODE
        case POWER_FEATURE_DRAW_W:
            sysfs_write(DRAW_W_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_ARROW_LEFT_NODE
        case POWER_FEATURE_DRAW_ARROW_LEFT:
            sysfs_write(DRAW_ARROW_LEFT_NODE, state ? "1" : "0");
            break;
#endif
#ifdef DRAW_ARROW_RIGHT_NODE
        case POWER_FEATURE_DRAW_ARROW_RIGHT:
            sysfs_write(DRAW_ARROW_RIGHT_NODE, state ? "1" : "0");
            break;
#endif
#ifdef ONE_FINGER_SWIPE_UP_NODE
        case POWER_FEATURE_ONE_FINGER_SWIPE_UP:
            sysfs_write(ONE_FINGER_SWIPE_UP_NODE, state ? "1" : "0");
            break;
#endif
#ifdef ONE_FINGER_SWIPE_RIGHT_NODE
        case POWER_FEATURE_ONE_FINGER_SWIPE_RIGHT:
            sysfs_write(ONE_FINGER_SWIPE_RIGHT_NODE, state ? "1" : "0");
            break;
#endif
#ifdef ONE_FINGER_SWIPE_DOWN_NODE
        case POWER_FEATURE_ONE_FINGER_SWIPE_DOWN:
            sysfs_write(ONE_FINGER_SWIPE_DOWN_NODE, state ? "1" : "0");
            break;
#endif
#ifdef ONE_FINGER_SWIPE_LEFT_NODE
        case POWER_FEATURE_ONE_FINGER_SWIPE_LEFT:
            sysfs_write(ONE_FINGER_SWIPE_LEFT_NODE, state ? "1" : "0");
            break;
#endif
#ifdef TWO_FINGER_SWIPE_NODE
        case POWER_FEATURE_TWO_FINGER_SWIPE:
            sysfs_write(TWO_FINGER_SWIPE_NODE, state ? "1" : "0");
            break;
#endif
        default:
            break;
    }
}