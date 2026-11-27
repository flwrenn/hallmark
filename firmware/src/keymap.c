/*
 * Keymap implementation -- layer resolution and held usages.
 */

#include "keymap.h"

#define MOD_BIT(usage) ((uint8_t)(1u << ((usage) - KC_LCTL)))

_Static_assert(KEYMAP_LAYERS <= 16, "layer masks are 16 bits wide");

/* --------------------------------------------------------------------- */

/* The layer a KM_MO or KM_TG action names, as a bit.  0 past the last layer. */
static uint16_t layer_bit(uint16_t action)
{
    unsigned int layer = (unsigned int)(action & ~KM_KIND(action));

    return layer < KEYMAP_LAYERS ? (uint16_t)(1u << layer) : 0;
}

/* L_BASE, the toggled layers, and every layer a held position keeps active. */
static uint16_t active_layers(const struct keymap_state *km)
{
    uint16_t active = (uint16_t)(km->toggled | (1u << L_BASE));

    for (int pos = 0; pos < KEYMAP_KEYS; pos++) {
        active |= km->holds[pos].layers;
    }
    return active;
}

/* The highest active layer's entry that is not KC_TRNS, else L_BASE's. */
static uint16_t resolve(const struct keymap_state *km, enum keymap_pos pos)
{
    uint16_t active = active_layers(km);

    for (int layer = KEYMAP_LAYERS - 1; layer > L_BASE; layer--) {
        uint16_t action = km->layers[layer][pos];

        if ((active & (1u << layer)) && action != KC_TRNS) {
            return action;
        }
    }
    return km->layers[L_BASE][pos];
}

/*
 * One keyboard usage held together with mods.  Modifier usages 0xE0 to
 * 0xE7 become their modifier bit.  Usage 0 means no event and usages
 * above KC_RGUI are reserved, so neither holds a key.
 */
static struct keymap_hold usage_hold(uint8_t usage, uint8_t mods)
{
    struct keymap_hold hold = {.mods = mods};

    if (usage >= KC_LCTL && usage <= KC_RGUI) {
        hold.mods |= MOD_BIT(usage);
    } else if (usage < KC_LCTL) {
        hold.usage = usage;
    }
    return hold;
}

/*
 * What an action holds from the moment its position goes down.  The
 * one place action kinds are told apart.  KM_TG holds nothing: it
 * flips its layer here instead.  The kinds later slices own (KM_TH,
 * KM_OS, KM_HRM, KM_MACRO, KM_UC) hold nothing until those slices
 * handle them.
 */
static struct keymap_hold press_action(struct keymap_state *km, uint16_t action)
{
    struct keymap_hold hold = {0};

    switch (KM_KIND(action)) {
    case KM_KIND_BASIC:
        return usage_hold((uint8_t)action, 0);
    case KM_KIND_S:
        return usage_hold((uint8_t)action, MOD_BIT(KC_LSFT));
    case KM_KIND_MO:
        hold.layers = layer_bit(action);
        break;
    case KM_KIND_TG:
        km->toggled ^= layer_bit(action);
        break;
    default:
        break;
    }
    return hold;
}

/* --------------------------------------------------------------------- */

void keymap_init(struct keymap_state *km, const uint16_t (*layers)[KEYMAP_KEYS])
{
    *km = (struct keymap_state){.layers = layers};
}

void keymap_press(struct keymap_state *km, enum keymap_pos pos)
{
    if ((unsigned int)pos < KEYMAP_KEYS) {
        km->holds[pos] = press_action(km, resolve(km, pos));
    }
}

void keymap_release(struct keymap_state *km, enum keymap_pos pos)
{
    if ((unsigned int)pos < KEYMAP_KEYS) {
        km->holds[pos] = (struct keymap_hold){0};
    }
}

struct keymap_held keymap_get_held(const struct keymap_state *km)
{
    struct keymap_held held = {0};

    for (int pos = 0; pos < KEYMAP_KEYS; pos++) {
        struct keymap_hold hold = km->holds[pos];

        held.mods |= hold.mods;
        if (hold.usage != 0) {
            held.keys[hold.usage / 8] |= (uint8_t)(1u << (hold.usage % 8));
        }
    }
    return held;
}
