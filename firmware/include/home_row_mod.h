/*
 * Depth-based Home Row Mods -- tap/hold decision using key travel
 * depth combined with a time fallback.
 *
 * Pure C module, no RTOS dependencies.
 *
 * Resolution paths:
 *   1. Released before depth AND time thresholds  ->  TAP  (letter)
 *   2. Crosses depth threshold at any time        ->  HOLD (modifier, immediate)
 *   3. Held past time threshold without depth     ->  HOLD (fallback)
 */

#ifndef HOME_ROW_MOD_H
#define HOME_ROW_MOD_H

#include <stdbool.h>
#include <stdint.h>

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

enum hrm_action {
    HRM_PENDING, /* decision not yet made */
    HRM_TAP,     /* resolved as tap (letter) */
    HRM_HOLD,    /* resolved as hold (modifier) */
};

/**
 * Per-key configuration.  Supports per-finger tuning -- pinkies may
 * need a deeper depth threshold or longer time window than index fingers.
 *
 * @depth_threshold_adc  ADC value that triggers an immediate HOLD.
 *                       Relative to rest (rest_adc + depth_threshold_adc
 *                       gives the absolute trigger point).
 * @time_threshold_ms    Milliseconds after which a sustained press
 *                       without depth resolves as HOLD (fallback).
 * @rest_adc             ADC reading at rest (key fully released).
 */
struct hrm_config {
    uint16_t depth_threshold_adc;
    uint32_t time_threshold_ms;
    uint16_t rest_adc;
};

/**
 * Per-key runtime state.  Call hrm_init() once at startup to
 * zero-initialise; hrm_press() resets the decision state on each
 * key-down.
 */
struct hrm_key_state {
    enum hrm_action action;
    uint32_t press_start_ms;
    uint16_t max_depth_adc; /* peak depth relative to rest */
    bool pressed;           /* whether the key is currently down */
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * Initialise a key state.  Call once at startup.
 */
void hrm_init(struct hrm_key_state *key);

/**
 * Signal that the key has been pressed.  Resets the HRM decision state.
 *
 * @timestamp_ms  Current time in milliseconds.
 */
void hrm_press(struct hrm_key_state *key, uint32_t timestamp_ms);

/**
 * Feed a new ADC sample while the key is held.
 *
 * @adc           Current ADC reading for this key.
 * @timestamp_ms  Current time in milliseconds.
 * @cfg           Per-key HRM configuration.
 *
 * Returns the current action (PENDING, TAP, or HOLD).  Once resolved
 * to TAP or HOLD the result is latched until the next hrm_press().
 */
enum hrm_action hrm_update(struct hrm_key_state *key, uint16_t adc, uint32_t timestamp_ms,
                           const struct hrm_config *cfg);

/**
 * Signal that the key has been released.
 *
 * If the action is still PENDING at release, it resolves as TAP.
 *
 * Returns the final resolved action.
 */
enum hrm_action hrm_release(struct hrm_key_state *key);

#endif /* HOME_ROW_MOD_H */
