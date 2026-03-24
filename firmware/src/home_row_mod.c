/*
 * Depth-based Home Row Mod implementation.
 */

#include "home_row_mod.h"

/* --------------------------------------------------------------------- */

void hrm_init(struct hrm_key_state *key)
{
    key->action = HRM_PENDING;
    key->press_start_ms = 0;
    key->max_depth_adc = 0;
    key->pressed = false;
}

/* --------------------------------------------------------------------- */

void hrm_press(struct hrm_key_state *key, uint32_t timestamp_ms)
{
    key->action = HRM_PENDING;
    key->press_start_ms = timestamp_ms;
    key->max_depth_adc = 0;
    key->pressed = true;
}

/* --------------------------------------------------------------------- */

enum hrm_action hrm_update(struct hrm_key_state *key, uint16_t adc, uint32_t timestamp_ms,
                           const struct hrm_config *cfg)
{
    /* Already resolved -- return latched result. */
    if (key->action != HRM_PENDING) {
        return key->action;
    }

    /* Not pressed -- nothing to do. */
    if (!key->pressed) {
        return HRM_PENDING;
    }

    /* Track depth relative to rest. */
    uint16_t depth = (adc > cfg->rest_adc) ? (adc - cfg->rest_adc) : 0;
    if (depth > key->max_depth_adc) {
        key->max_depth_adc = depth;
    }

    /* Path 2: depth threshold crossed -> immediate HOLD. */
    if (key->max_depth_adc >= cfg->depth_threshold_adc) {
        key->action = HRM_HOLD;
        return HRM_HOLD;
    }

    /* Path 3: time threshold exceeded -> fallback HOLD. */
    uint32_t elapsed = timestamp_ms - key->press_start_ms;
    if (elapsed >= cfg->time_threshold_ms) {
        key->action = HRM_HOLD;
        return HRM_HOLD;
    }

    /* Still undecided. */
    return HRM_PENDING;
}

/* --------------------------------------------------------------------- */

enum hrm_action hrm_release(struct hrm_key_state *key)
{
    key->pressed = false;

    /* Path 1: still pending at release -> TAP. */
    if (key->action == HRM_PENDING) {
        key->action = HRM_TAP;
    }

    return key->action;
}
