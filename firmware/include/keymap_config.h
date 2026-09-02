/*
 * Keymap configuration for Hallmark.
 *
 * 36-key Chocofi split keyboard. Colemak-DH base, SCAG home row mods,
 * 9 layers, 22 combos, 2 SOCD pairs.
 *
 * NOTE: Keycode values are placeholders. The Keymap firmware module
 * (not yet implemented) will define the actual encoding. Layer arrays
 * and config structs are structurally correct and ready to be wired
 * into the module.
 */

#ifndef KEYMAP_CONFIG_H
#define KEYMAP_CONFIG_H

#include <stdint.h>

/* -------------------------------------------------------------------
 * Layers
 * ---------------------------------------------------------------- */

enum keymap_layer {
    L_BASE,
    L_SYM,
    L_NAV,
    L_NUM,
    L_GAME,
    L_GAMENUM,
    L_GAMEALT,
    L_NOMOD,
    L_ORIGIN,
    KEYMAP_LAYERS,
};

/* -------------------------------------------------------------------
 * Key positions
 *
 * Index into layer arrays.  Enumerator order is the layer array
 * order: one row at a time, left half then right half, thumbs last.
 *
 *  L01  L02  L03  L04  L05    R01  R02  R03  R04  R05
 *  L06  L07  L08  L09  L10    R06  R07  R08  R09  R10
 *  L11  L12  L13  L14  L15    R11  R12  R13  R14  R15
 *            LT1  LT2  LT3    RT1  RT2  RT3
 *
 * POS_NONE terminates position lists.
 * ---------------------------------------------------------------- */

enum keymap_pos {
    POS_L01,
    POS_L02,
    POS_L03,
    POS_L04,
    POS_L05,
    POS_R01,
    POS_R02,
    POS_R03,
    POS_R04,
    POS_R05,

    POS_L06,
    POS_L07,
    POS_L08,
    POS_L09,
    POS_L10,
    POS_R06,
    POS_R07,
    POS_R08,
    POS_R09,
    POS_R10,

    POS_L11,
    POS_L12,
    POS_L13,
    POS_L14,
    POS_L15,
    POS_R11,
    POS_R12,
    POS_R13,
    POS_R14,
    POS_R15,

    POS_LT1,
    POS_LT2,
    POS_LT3,
    POS_RT1,
    POS_RT2,
    POS_RT3,

    KEYMAP_KEYS,
    POS_NONE = KEYMAP_KEYS,
};

/* -------------------------------------------------------------------
 * Keycodes  (placeholders -- Keymap module will define encoding)
 * ---------------------------------------------------------------- */

/* Special */
#define ___ 0x0000 /* Transparent: falls through to layer below */
#define XXX 0x0001 /* Blocked: no output                       */

/* Alphas (USB HID usage page 0x07) */
#define KC_A 0x04
#define KC_B 0x05
#define KC_C 0x06
#define KC_D 0x07
#define KC_E 0x08
#define KC_F 0x09
#define KC_G 0x0A
#define KC_H 0x0B
#define KC_I 0x0C
#define KC_J 0x0D
#define KC_K 0x0E
#define KC_L 0x0F
#define KC_M 0x10
#define KC_N 0x11
#define KC_O 0x12
#define KC_P 0x13
#define KC_Q 0x14
#define KC_R 0x15
#define KC_S 0x16
#define KC_T 0x17
#define KC_U 0x18
#define KC_V 0x19
#define KC_W 0x1A
#define KC_X 0x1B
#define KC_Y 0x1C
#define KC_Z 0x1D

/* Numbers */
#define KC_1 0x1E
#define KC_2 0x1F
#define KC_3 0x20
#define KC_4 0x21
#define KC_5 0x22
#define KC_6 0x23
#define KC_7 0x24
#define KC_8 0x25
#define KC_9 0x26
#define KC_0 0x27

/* Punctuation */
#define KC_QUOT 0x34 /* '  */
#define KC_GRV  0x35 /* `  */
#define KC_COMM 0x36 /* ,  */
#define KC_DOT  0x37 /* .  */
#define KC_SCLN 0x33 /* ;  */
#define KC_SLSH 0x38 /* /  */
#define KC_BSLS 0x31 /* \  */
#define KC_MINS 0x2D /* -  */
#define KC_EQL  0x2E /* =  */
#define KC_LBRC 0x2F /* [  */
#define KC_RBRC 0x30 /* ]  */

/* Editing */
#define KC_ESC  0x29
#define KC_TAB  0x2B
#define KC_SPC  0x2C
#define KC_ENT  0x28
#define KC_BSPC 0x2A
#define KC_DEL  0x4C
#define KC_INS  0x49
#define KC_CAPS 0x39

/* Navigation */
#define KC_LEFT 0x50
#define KC_DOWN 0x51
#define KC_UP   0x52
#define KC_RGHT 0x4F
#define KC_HOME 0x4A
#define KC_END  0x4D
#define KC_PGUP 0x4B
#define KC_PGDN 0x4E

/* Function keys */
#define KC_F1  0x3A
#define KC_F2  0x3B
#define KC_F3  0x3C
#define KC_F4  0x3D
#define KC_F5  0x3E
#define KC_F6  0x3F
#define KC_F7  0x40
#define KC_F8  0x41
#define KC_F9  0x42
#define KC_F10 0x43
#define KC_F11 0x44
#define KC_F12 0x45

/* Modifiers */
#define KC_LSFT 0xE1
#define KC_LCTL 0xE0
#define KC_LALT 0xE2
#define KC_LGUI 0xE3
#define KC_RSFT 0xE5
#define KC_RCTL 0xE4
#define KC_RALT 0xE6
#define KC_RGUI 0xE7

/*
 * Actions  (encoding TBD by Keymap module)
 *
 * MO(layer)        Momentary layer hold
 * TG(layer)        Toggle layer on/off
 * TH(tap, layer)   Tap for key, hold for momentary layer
 * OS(mod)          One-shot modifier
 * HRM(tap, mod)    Home row mod: tap for key, hold for modifier
 * S(kc)            Shifted keycode
 * UC(cp)           Unicode codepoint
 * MACRO(id)        Predefined macro
 */
#define MO(l)     (0x5000 | (l))
#define TG(l)     (0x5100 | (l))
#define TH(t, l)  (0x5200 | ((l) << 8) | (t))
#define OS(m)     (0x5300 | (m))
#define HRM(t, m) (0x5400 | ((m) << 8) | (t))
#define S(kc)     (0x0100 | (kc))
#define UC(cp)    (0x6000 | (cp))
#define MACRO(id) (0x7000 | (id))

/* Shifted key aliases */
#define KC_EXLM S(KC_1)    /* ! */
#define KC_AT   S(KC_2)    /* @ */
#define KC_HASH S(KC_3)    /* # */
#define KC_DLR  S(KC_4)    /* $ */
#define KC_PERC S(KC_5)    /* % */
#define KC_CIRC S(KC_6)    /* ^ */
#define KC_AMPR S(KC_7)    /* & */
#define KC_ASTR S(KC_8)    /* * */
#define KC_LPRN S(KC_9)    /* ( */
#define KC_RPRN S(KC_0)    /* ) */
#define KC_UNDS S(KC_MINS) /* _ */
#define KC_PLUS S(KC_EQL)  /* + */
#define KC_LCBR S(KC_LBRC) /* { */
#define KC_RCBR S(KC_RBRC) /* } */
#define KC_PIPE S(KC_BSLS) /* | */
#define KC_TILD S(KC_GRV)  /* ~ */
#define KC_QUES S(KC_SLSH) /* ? */

/* Unicode aliases */
#define KC_EUR UC(0x20AC) /* Euro sign */

/* Macro aliases */
#define MC_PTAB MACRO(0)  /* Ctrl+Shift+Tab (previous tab) */
#define MC_NTAB MACRO(1)  /* Ctrl+Tab (next tab)           */
#define MC_BACK MACRO(2)  /* Browser back                  */
#define MC_FWD  MACRO(3)  /* Browser forward               */
#define MC_PREV MACRO(4)  /* Media previous track          */
#define MC_NEXT MACRO(5)  /* Media next track              */
#define MC_PLAY MACRO(6)  /* Media play/pause              */
#define MC_BRUP MACRO(7)  /* Brightness up                 */
#define MC_BRDN MACRO(8)  /* Brightness down               */
#define MC_VOLU MACRO(9)  /* Volume up                     */
#define MC_VOLD MACRO(10) /* Volume down                   */
#define MC_MUTE MACRO(11) /* Mute                          */

/* One-shot modifier aliases */
#define OS_LSFT OS(KC_LSFT)
#define OS_LCTL OS(KC_LCTL)
#define OS_LALT OS(KC_LALT)
#define OS_LGUI OS(KC_LGUI)
#define OS_RSFT OS(KC_RSFT)
#define OS_RCTL OS(KC_RCTL)
#define OS_RALT OS(KC_RALT)
#define OS_RGUI OS(KC_RGUI)

/* One-shot hyper (Shift+Ctrl+Alt+GUI) */
#define OS_HYPR MACRO(12)

/* Caps Word toggle (auto-deactivates after non-alpha; firmware TBD) */
#define CW_TOGG MACRO(13)

/* Home row mod aliases */
#define HM_A HRM(KC_A, KC_LSFT)
#define HM_R HRM(KC_R, KC_LCTL)
#define HM_S HRM(KC_S, KC_LALT)
#define HM_T HRM(KC_T, KC_LGUI)
#define HM_N HRM(KC_N, KC_RGUI)
#define HM_E HRM(KC_E, KC_RALT)
#define HM_I HRM(KC_I, KC_RCTL)
#define HM_O HRM(KC_O, KC_RSFT)

/* Numeric layer HRM aliases */
#define HM_7 HRM(KC_7, KC_LSFT)
#define HM_5 HRM(KC_5, KC_LCTL)
#define HM_3 HRM(KC_3, KC_LALT)
#define HM_1 HRM(KC_1, KC_LGUI)
#define HM_0 HRM(KC_0, KC_RGUI)
#define HM_2 HRM(KC_2, KC_RALT)
#define HM_4 HRM(KC_4, KC_RCTL)
#define HM_6 HRM(KC_6, KC_RSFT)

/* Layer action aliases */
#define MO_NAV  MO(L_NAV)
#define MO_SYM  MO(L_SYM)
#define MO_NUM  MO(L_NUM)
#define MO_GNUM TH(KC_G, L_GAMENUM)

/* -------------------------------------------------------------------
 * Configuration tables  (defined in firmware/src/keymap_config.c)
 * ---------------------------------------------------------------- */

#define KEYMAP_HRM_COUNT      8
#define KEYMAP_HAND_KEYS      18
#define KEYMAP_COMBOS         22
#define KEYMAP_COMBO_MAX_KEYS 4
#define KEYMAP_SOCD_PAIRS     2

/*
 * Home row mod thresholds for one position.  Field names mirror
 * struct hrm_config (home_row_mod.h); the Keymap module copies them
 * into one at runtime and fills rest_adc from calibration.
 */
struct keymap_hrm {
    enum keymap_pos position;
    uint16_t depth_threshold_adc;
    uint32_t time_threshold_ms;
};

/*
 * Chord of up to KEYMAP_COMBO_MAX_KEYS positions, POS_NONE terminated.
 */
struct keymap_combo {
    enum keymap_pos positions[KEYMAP_COMBO_MAX_KEYS + 1];
    uint16_t output;
};

/*
 * Opposing key pair fed to one struct socd_pair (socd.h).
 */
struct keymap_socd_pair {
    enum keymap_pos pos_a;
    enum keymap_pos pos_b;
};

extern const uint16_t keymap_layers[KEYMAP_LAYERS][KEYMAP_KEYS];
extern const struct keymap_hrm keymap_hrm_table[];
extern const enum keymap_pos keymap_left_hand[];
extern const enum keymap_pos keymap_right_hand[];
extern const struct keymap_combo keymap_combos[];
extern const struct keymap_socd_pair keymap_socd_pairs[];

#endif /* KEYMAP_CONFIG_H */
