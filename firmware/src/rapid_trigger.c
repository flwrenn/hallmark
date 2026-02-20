/*
 * Rapid Trigger FSM implementation.
 *
 * Higher ADC values mean a deeper key press.  The FSM tracks peak
 * (deepest) and trough (shallowest) positions and fires events when
 * the direction reverses by more than the configured sensitivity.
 */

#include "rapid_trigger.h"

/* --------------------------------------------------------------------- */

void rt_init(struct rt_key_state *key, const struct rt_config *cfg)
{
    key->state = RT_IDLE;
    key->peak = cfg->rest_adc;
    key->trough = cfg->rest_adc;
    key->cfg = *cfg;
}

/* --------------------------------------------------------------------- */

/*
 * Transition helpers -- keep the main update function readable.
 */

static enum rt_event handle_idle(struct rt_key_state *key, uint16_t adc)
{
    if (adc >= key->cfg.actuation_adc) {
        key->state = RT_ACTIVE;
        key->peak = adc;
        key->trough = adc;
        return RT_EVENT_PRESS;
    }
    return RT_EVENT_NONE;
}

static enum rt_event handle_active(struct rt_key_state *key, uint16_t adc)
{
    /* Track the deepest point reached. */
    if (adc >= key->peak) {
        key->peak = adc;
        return RT_EVENT_NONE;
    }

    /* Key is moving upward -- check if it returned fully to rest. */
    if (adc <= key->cfg.release_adc) {
        key->state = RT_IDLE;
        key->peak = key->cfg.rest_adc;
        key->trough = key->cfg.rest_adc;
        return RT_EVENT_RELEASE;
    }

    /* Upward reversal large enough? */
    if (key->peak - adc >= key->cfg.rt_sensitivity) {
        key->state = RT_RELEASING;
        key->trough = adc;
        return RT_EVENT_RELEASE;
    }

    return RT_EVENT_NONE;
}

static enum rt_event handle_releasing(struct rt_key_state *key, uint16_t adc)
{
    /* Track the shallowest point reached. */
    if (adc <= key->trough) {
        key->trough = adc;

        /* Returned fully to rest? */
        if (adc <= key->cfg.release_adc) {
            key->state = RT_IDLE;
            key->peak = key->cfg.rest_adc;
            key->trough = key->cfg.rest_adc;
            return RT_EVENT_NONE;
        }
        return RT_EVENT_NONE;
    }

    /* Key is moving downward -- re-actuation? */
    if (adc - key->trough >= key->cfg.rt_sensitivity) {
        key->state = RT_ACTIVE;
        key->peak = adc;
        return RT_EVENT_PRESS;
    }

    return RT_EVENT_NONE;
}

/* --------------------------------------------------------------------- */

enum rt_event rt_update(struct rt_key_state *key, uint16_t adc)
{
    switch (key->state) {
    case RT_IDLE:
        return handle_idle(key, adc);
    case RT_ACTIVE:
        return handle_active(key, adc);
    case RT_RELEASING:
        return handle_releasing(key, adc);
    }

    /* Unreachable -- but keep the compiler happy. */
    return RT_EVENT_NONE;
}
