/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 * Copyright (C) 2018 The LineageOS Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_NIDEBUG 0

#include <errno.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "QCOM PowerHAL"
#include <log/log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>

#include "utils.h"
#include "metadata-defs.h"
#include "hint-data.h"
#include "performance.h"
#include "power-common.h"

static int first_display_off_hint;

/**
 * Returns true if the target is MSM8974AB or MSM8974AC.
 */
static bool is_target_8974pro(void)
{
    static int is_8974pro = -1;
    int soc_id;

    if (is_8974pro >= 0)
        return is_8974pro;

    soc_id = get_soc_id();
    is_8974pro = soc_id == 194 || (soc_id >= 208 && soc_id <= 218);

    return is_8974pro;
}

static int resources_interaction_fling_boost[] = {
    CPUS_ONLINE_MIN_3,
    0x20F,
    0x30F,
    0x40F,
    0x50F
};

static int resources_interaction_boost[] = {
    CPUS_ONLINE_MIN_2,
    0x20F,
    0x30F,
    0x40F,
    0x50F
};

static int resources_launch[] = {
    CPUS_ONLINE_MIN_3,
    CPU0_MIN_FREQ_TURBO_MAX,
    CPU1_MIN_FREQ_TURBO_MAX,
    CPU2_MIN_FREQ_TURBO_MAX,
    CPU3_MIN_FREQ_TURBO_MAX
};

static int process_activity_launch_hint(void *data)
{
    static int launch_handle = -1;
    static int launch_mode = 0;

    // release lock early if launch has finished
    if (!data) {
        if (CHECK_HANDLE(launch_handle)) {
            release_request(launch_handle);
            launch_handle = -1;
        }
        launch_mode = 0;
        return HINT_HANDLED;
    }

    if (!launch_mode) {
        launch_handle = interaction_with_handle(launch_handle, 5000,
                ARRAY_SIZE(resources_launch), resources_launch);
        if (!CHECK_HANDLE(launch_handle)) {
            ALOGE("Failed to perform launch boost");
            return HINT_NONE;
        }
        launch_mode = 1;
    }
    return HINT_HANDLED;
}

int power_hint_override(power_hint_t hint, void *data)
{
    static struct timespec s_previous_boost_timespec;
    struct timespec cur_boost_timespec;
    long long elapsed_time;
    static int s_previous_duration = 0;
    int duration;

    switch (hint) {
        case POWER_HINT_INTERACTION:
            duration = 500; // 500ms by default
            if (data) {
                int input_duration = *((int*)data);
                if (input_duration > duration) {
                    duration = (input_duration > 5000) ? 5000 : input_duration;
                }
            }

            clock_gettime(CLOCK_MONOTONIC, &cur_boost_timespec);

            elapsed_time = calc_timespan_us(s_previous_boost_timespec, cur_boost_timespec);
            // don't hint if previous hint's duration covers this hint's duration
            if ((s_previous_duration * 1000) > (elapsed_time + duration * 1000)) {
                return HINT_HANDLED;
            }
            s_previous_boost_timespec = cur_boost_timespec;
            s_previous_duration = duration;

            if (duration >= 1500) {
                interaction(duration, ARRAY_SIZE(resources_interaction_fling_boost),
                        resources_interaction_fling_boost);
            } else {
                interaction(duration, ARRAY_SIZE(resources_interaction_boost),
                        resources_interaction_boost);
            }
            return HINT_HANDLED;
        case POWER_HINT_LAUNCH:
            return process_activity_launch_hint(data);
        default:
            break;
    }
    return HINT_NONE;
}

int set_interactive_override(int on)
{
    char governor[80];

    if (get_scaling_governor(governor, sizeof(governor)) == -1) {
        ALOGE("Can't obtain scaling governor.");
        return HINT_NONE;
    }

    if (!on) {
        /* Display off. */
        /*
         * We need to be able to identify the first display off hint
         * and release the current lock holder
         */
        if (is_target_8974pro()) {
            if (!first_display_off_hint) {
                undo_initial_hint_action();
                first_display_off_hint = 1;
            }
            /* used for all subsequent toggles to the display */
            undo_hint_action(DISPLAY_STATE_HINT_ID_2);
        }

        if (is_ondemand_governor(governor)) {
            int resource_values[] = {
                MS_500, SYNC_FREQ_600, OPTIMAL_FREQ_600, THREAD_MIGRATION_SYNC_OFF
            };
            perform_hint_action(DISPLAY_STATE_HINT_ID,
                    resource_values, ARRAY_SIZE(resource_values));
        }
    } else {
        /* Display on */
        if (is_target_8974pro()) {
            int resource_values2[] = {
                CPUS_ONLINE_MIN_2
            };
            perform_hint_action(DISPLAY_STATE_HINT_ID_2,
                    resource_values2, ARRAY_SIZE(resource_values2));
        }

        if (is_ondemand_governor(governor)) {
            undo_hint_action(DISPLAY_STATE_HINT_ID);
        }
    }
    return HINT_HANDLED;
}
