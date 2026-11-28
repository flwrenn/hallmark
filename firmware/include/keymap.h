/*
 * Keymap -- turns key positions going down and up into the HID
 * keyboard usages and modifiers held now.
 *
 * Pure C module, no RTOS dependencies.  Runs on the central device
 * after rapid trigger, one call per press or release event.
 *
 * Each press resolves its position through the active layers once and
 * records what that action holds.  The release clears the record, so a
 * key releases what it pressed even if the layers changed meanwhile.
 * Momentary layers and HID output are the union of those records, so
 * a layer or usage stays held until its last holder lets go.
 *
 * Handled: basic and modifier keycodes, KM_S, KM_MO, KM_TG, and
 * KC_TRNS.  Every other action kind holds nothing yet.
 */

#ifndef KEYMAP_H
#define KEYMAP_H

#include "keymap_config.h"

#include <stdint.h>

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

/* Bytes in the bitmap of non-modifier keyboard usages, 0x00 to 0xDF. */
#define KEYMAP_USAGE_BYTES (KC_LCTL / 8)

/**
 * Keyboard usages (HID usage page 0x07) held now.  Shaped so the HID
 * module can build either a 6KRO boot report or an NKRO bitmap.
 *
 * @mods  Bit n set: modifier usage 0xE0 + n is held.  Same layout as
 *        the modifier byte of a HID keyboard report.
 * @keys  Bit (u % 8) of keys[u / 8] set: usage u is held.  Usage 0
 *        (no event) is never set.
 */
struct keymap_held {
    uint8_t mods;
    uint8_t keys[KEYMAP_USAGE_BYTES];
};

/**
 * What one position holds, decided when it went down.  All zero while
 * the position is up.
 *
 * @usage   Non-modifier keyboard usage, 0 for none.
 * @mods    Modifier bits, laid out as keymap_held.mods.
 * @layers  Bit per layer this position keeps active.
 */
struct keymap_hold {
    uint8_t usage;
    uint8_t mods;
    uint16_t layers;
};

/**
 * Keymap runtime state.  Initialise with keymap_init() before first
 * use.  Fields are private to keymap.c.
 *
 * @layers   Layer arrays, indexed [layer][position].
 * @holds    What each position holds.
 * @toggled  Bit per layer, flipped by each KM_TG press.
 */
struct keymap_state {
    const uint16_t (*layers)[KEYMAP_KEYS];
    struct keymap_hold holds[KEYMAP_KEYS];
    uint16_t toggled;
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * Initialise (or reset) the keymap: every position up, no layer
 * toggled, so only L_BASE is active.
 *
 * @layers  KEYMAP_LAYERS layer arrays of KEYMAP_KEYS actions each.
 *          The firmware passes keymap_layers.  Must outlive the state.
 */
void keymap_init(struct keymap_state *km, const uint16_t (*layers)[KEYMAP_KEYS]);

/**
 * A position went down.  Resolves it through the active layers: the
 * highest active layer whose entry is not KC_TRNS wins, and L_BASE is
 * always active.  A KM_TG action flips its layer here.  POS_NONE is
 * ignored.
 *
 * Release a position before pressing it again.  A second press with no
 * release resolves the position again, and a KM_TG flips twice.  Rapid
 * trigger alternates the two.
 */
void keymap_press(struct keymap_state *km, enum keymap_pos pos);

/**
 * A position went up.  Releases whatever its press held.  POS_NONE is
 * ignored.
 */
void keymap_release(struct keymap_state *km, enum keymap_pos pos);

/**
 * Keyboard usages and modifiers held now.  Read it after each press
 * or release to see every change.
 */
struct keymap_held keymap_get_held(const struct keymap_state *km);

#endif /* KEYMAP_H */
