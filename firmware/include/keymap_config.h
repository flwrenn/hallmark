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
 * Dimensions
 * ---------------------------------------------------------------- */

#define KEYMAP_KEYS       36
#define KEYMAP_LAYERS     9
#define KEYMAP_COMBOS     22
#define KEYMAP_SOCD_PAIRS 2

/* -------------------------------------------------------------------
 * Layer indices
 * ---------------------------------------------------------------- */

#define L_BASE    0
#define L_SYM     1
#define L_NAV     2
#define L_NUM     3
#define L_GAME    4
#define L_GAMENUM 5
#define L_GAMEALT 6
#define L_NOMOD   7
#define L_ORIGIN  8

/* -------------------------------------------------------------------
 * Key positions
 *
 * Index into layer arrays.  Matches the physical layout in
 * docs/architecture.md.
 *
 *  L01  L02  L03  L04  L05    R01  R02  R03  R04  R05
 *  L06  L07  L08  L09  L10    R06  R07  R08  R09  R10
 *  L11  L12  L13  L14  L15    R11  R12  R13  R14  R15
 *            LT1  LT2  LT3    RT1  RT2  RT3
 * ---------------------------------------------------------------- */

#define POS_L01 0
#define POS_L02 1
#define POS_L03 2
#define POS_L04 3
#define POS_L05 4
#define POS_L06 5
#define POS_L07 6
#define POS_L08 7
#define POS_L09 8
#define POS_L10 9
#define POS_L11 10
#define POS_L12 11
#define POS_L13 12
#define POS_L14 13
#define POS_L15 14
#define POS_LT1 15
#define POS_LT2 16
#define POS_LT3 17
#define POS_R01 18
#define POS_R02 19
#define POS_R03 20
#define POS_R04 21
#define POS_R05 22
#define POS_R06 23
#define POS_R07 24
#define POS_R08 25
#define POS_R09 26
#define POS_R10 27
#define POS_R11 28
#define POS_R12 29
#define POS_R13 30
#define POS_R14 31
#define POS_R15 32
#define POS_RT1 33
#define POS_RT2 34
#define POS_RT3 35

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
 * Layer arrays
 *
 * Each layer is a flat array of KEYMAP_KEYS entries in position order:
 *
 *   L01  L02  L03  L04  L05    R01  R02  R03  R04  R05
 *   L06  L07  L08  L09  L10    R06  R07  R08  R09  R10
 *   L11  L12  L13  L14  L15    R11  R12  R13  R14  R15
 *              LT1  LT2  LT3    RT1  RT2  RT3
 * ---------------------------------------------------------------- */

static const uint16_t keymap[KEYMAP_LAYERS][KEYMAP_KEYS] = {

        /* Layer 0: Base (Colemak-DH)
         *
         *   q     w     f     p     b          j     l     u     y     '
         *   a/S   r/C   s/A   t/G   g          m     n/G   e/A   i/C   o/S
         *   z     x     c     d     v          k     h     ,     .     ;
         *               Esc   NAV   Spc        Ent   SYM   Bks
         */
        [L_BASE] = {KC_Q,    KC_W,   KC_F,    KC_P,   KC_B,   KC_J,   KC_L,   KC_U,   KC_Y,
                    KC_QUOT, HM_A,   HM_R,    HM_S,   HM_T,   KC_G,   KC_M,   HM_N,   HM_E,
                    HM_I,    HM_O,   KC_Z,    KC_X,   KC_C,   KC_D,   KC_V,   KC_K,   KC_H,
                    KC_COMM, KC_DOT, KC_SCLN, KC_ESC, MO_NAV, KC_SPC, KC_ENT, MO_SYM, KC_BSPC},

        /* Layer 1: Symbols
         *
         *   Esc   [     {     (     ~          ^     )     }     ]     `
         *   -     *     =     _     $          EUR   Gos   Aos   Cos   Sos
         *   +     |     @     /     %          #     \     &     ?     !
         *               NUM   ___   ___        ___   ___   ___
         */
        [L_SYM] = {KC_ESC,  KC_LBRC, KC_LCBR, KC_LPRN, KC_TILD, KC_CIRC, KC_RPRN, KC_RCBR, KC_RBRC,
                   KC_GRV,  KC_MINS, KC_ASTR, KC_EQL,  KC_UNDS, KC_DLR,  KC_EUR,  OS_RGUI, OS_RALT,
                   OS_RCTL, OS_RSFT, KC_PLUS, KC_PIPE, KC_AT,   KC_SLSH, KC_PERC, KC_HASH, KC_BSLS,
                   KC_AMPR, KC_QUES, KC_EXLM, MO_NUM,  ___,     ___,     ___,     ___,     ___},

        /* Layer 2: Navigation
         *
         *   Tab   PTb   NTb   Vo-   Vo+        Bck   Fwd   Hom   End   Del
         *   Sos   Cos   Aos   Gos   Mut        <     v     ^     >     Bks
         *   Br-   Br+   Prv   Nxt   Ply        Cap   PgD   PgU   Ins   Ent
         *               ___   ___   ___        NUM   ___   ___
         */
        [L_NAV] = {KC_TAB,  MC_PTAB, MC_NTAB, MC_VOLD, MC_VOLU, MC_BACK, MC_FWD,  KC_HOME, KC_END,
                   KC_DEL,  OS_LSFT, OS_LCTL, OS_LALT, OS_LGUI, MC_MUTE, KC_LEFT, KC_DOWN, KC_UP,
                   KC_RGHT, KC_BSPC, MC_BRDN, MC_BRUP, MC_PREV, MC_NEXT, MC_PLAY, KC_CAPS, KC_PGDN,
                   KC_PGUP, KC_INS,  KC_ENT,  ___,     ___,     ___,     MO_NUM,  ___,     ___},

        /* Layer 3: Numeric
         *
         *   F10   F11   ___   ___   ___        ___   ___   ___   ___   ___
         *   7/S   5/C   3/A   1/G   9          8     0/G   2/A   4/C   6/S
         *   F7    F5    F3    F1    F9         F8    F12   F2    F4    F6
         *               ___   ___   ___        ___   ___   ___
         */
        [L_NUM] = {KC_F10, KC_F11, ___,   ___,   ___,   ___,   ___,   ___,   ___,
                   ___,    HM_7,   HM_5,  HM_3,  HM_1,  KC_9,  KC_8,  HM_0,  HM_2,
                   HM_4,   HM_6,   KC_F7, KC_F5, KC_F3, KC_F1, KC_F9, KC_F8, KC_F12,
                   KC_F2,  KC_F4,  KC_F6, ___,   ___,   ___,   ___,   ___,   ___},

        /* Layer 4: Gaming
         *
         *   Tab   Q     W     E     R          ___   ___   ___   ___   ___
         *   Esc   A     S     D     F          ___   ___   ___   ___   ___
         *   LSf   Z     X     C     V          ___   ___   ___   ___   ___
         *               LCt   G/NUM Spc        ___   ___   ___
         */
        [L_GAME] = {KC_TAB, KC_Q,   KC_W,    KC_E,    KC_R,    ___,    ___,  ___, ___,
                    ___,    KC_ESC, KC_A,    KC_S,    KC_D,    KC_F,   ___,  ___, ___,
                    ___,    ___,    KC_LSFT, KC_Z,    KC_X,    KC_C,   KC_V, ___, ___,
                    ___,    ___,    ___,     KC_LCTL, MO_GNUM, KC_SPC, ___,  ___, ___},

        /* Layer 5: Gaming Numbers  (hold LT2 from Gaming layer)
         *
         *   1     2     3     4     5          ___   ___   ___   ___   ___
         *   6     7     8     9     0          ___   ___   ___   ___   ___
         *   F1    F2    F3    F4    F5         ___   ___   ___   ___   ___
         *               ___   ___   ___        ___   ___   ___
         */
        [L_GAMENUM] = {KC_1,  KC_2, KC_3, KC_4, KC_5, ___, ___, ___, ___,   ___,   KC_6,  KC_7,
                       KC_8,  KC_9, KC_0, ___,  ___,  ___, ___, ___, KC_F1, KC_F2, KC_F3, KC_F4,
                       KC_F5, ___,  ___,  ___,  ___,  ___, ___, ___, ___,   ___,   ___,   ___},

        /* Layer 6: Gaming Alt
         *
         *   1     2     W     3     4          6     ___   ___   ___   ___
         *   Q     A     S     D     E          T     B     V     Y     Esc
         *   Tab   5     ___   F     G          P     ___   ___   ___   ___
         *               Ctl   Sft   Spc        ___   ___   ___
         */
        [L_GAMEALT] = {KC_1, KC_2,   KC_W,   KC_3,    KC_4,    KC_6,   ___,  ___,  ___,
                       ___,  KC_Q,   KC_A,   KC_S,    KC_D,    KC_E,   KC_T, KC_B, KC_V,
                       KC_Y, KC_ESC, KC_TAB, KC_5,    ___,     KC_F,   KC_G, KC_P, ___,
                       ___,  ___,    ___,    KC_LCTL, KC_LSFT, KC_SPC, ___,  ___,  ___},

        /* Layer 7: Nomods
         *
         *   q     w     f     p     b          j     l     u     y     '
         *   a     r     s     t     g          m     n     e     i     o
         *   z     x     c     d     v          k     h     ,     .     ;
         *               Esc   NAV   Spc        Ent   SYM   Bks
         */
        [L_NOMOD] = {KC_Q,    KC_W,   KC_F,    KC_P,   KC_B,   KC_J,   KC_L,   KC_U,   KC_Y,
                     KC_QUOT, KC_A,   KC_R,    KC_S,   KC_T,   KC_G,   KC_M,   KC_N,   KC_E,
                     KC_I,    KC_O,   KC_Z,    KC_X,   KC_C,   KC_D,   KC_V,   KC_K,   KC_H,
                     KC_COMM, KC_DOT, KC_SCLN, KC_ESC, MO_NAV, KC_SPC, KC_ENT, MO_SYM, KC_BSPC},

        /* Layer 8: Origin (QWERTY)
         *
         *   q     w     e     r     t          y     u     i     o     p
         *   a     s     d     f     g          h     j     k     l     ;
         *   z     x     c     v     b          n     m     ,     .     /
         *               Esc   NAV   Spc        Ent   SYM   Bks
         */
        [L_ORIGIN] = {KC_Q,    KC_W,    KC_E,    KC_R,   KC_T,   KC_Y,   KC_U,   KC_I,   KC_O,
                      KC_P,    KC_A,    KC_S,    KC_D,   KC_F,   KC_G,   KC_H,   KC_J,   KC_K,
                      KC_L,    KC_SCLN, KC_Z,    KC_X,   KC_C,   KC_V,   KC_B,   KC_N,   KC_M,
                      KC_COMM, KC_DOT,  KC_SLSH, KC_ESC, MO_NAV, KC_SPC, KC_ENT, MO_SYM, KC_BSPC},
};

/* -------------------------------------------------------------------
 * Home row mod configuration
 *
 * Per-finger depth and time thresholds.  ADC estimates assume
 * ~147 counts/mm (500-count range over 3.4mm Owlab Ti HE travel).
 * Actual values calibrated during Stage 1, tuned during Stage 4.
 *
 * Uses struct hrm_config from firmware/include/home_row_mod.h:
 *   { .depth_threshold_adc, .time_threshold_ms, .rest_adc }
 *
 * rest_adc is set to 0 here; actual values come from calibration.
 * ---------------------------------------------------------------- */

struct hrm_assignment {
    uint8_t position;
    uint16_t depth_threshold_adc;
    uint32_t time_threshold_ms;
};

static const struct hrm_assignment hrm_config_table[] = {
        /* Left hand: SCAG (Shift-Ctrl-Alt-GUI, pinky to index) */
        {POS_L06, 265, 250}, /* a -> LShift  (pinky, 1.8mm) */
        {POS_L07, 235, 220}, /* r -> LCtrl   (ring,  1.6mm) */
        {POS_L08, 206, 200}, /* s -> LAlt    (mid,   1.4mm) */
        {POS_L09, 176, 180}, /* t -> LGUI    (index, 1.2mm) */

        /* Right hand: mirrors left */
        {POS_R07, 176, 180}, /* n -> RGUI    (index, 1.2mm) */
        {POS_R08, 206, 200}, /* e -> RAlt    (mid,   1.4mm) */
        {POS_R09, 235, 220}, /* i -> RCtrl   (ring,  1.6mm) */
        {POS_R10, 265, 250}, /* o -> RShift  (pinky, 1.8mm) */
};

#define HRM_CONFIG_COUNT (sizeof(hrm_config_table) / sizeof(hrm_config_table[0]))

/* -------------------------------------------------------------------
 * Bilateral key sets
 *
 * A modifier only fires when the opposite hand presses a key.
 * ---------------------------------------------------------------- */

static const uint8_t left_hand_positions[] = {
        POS_L01, POS_L02, POS_L03, POS_L04, POS_L05, POS_L06, POS_L07, POS_L08, POS_L09,
        POS_L10, POS_L11, POS_L12, POS_L13, POS_L14, POS_L15, POS_LT1, POS_LT2, POS_LT3,
};

static const uint8_t right_hand_positions[] = {
        POS_R01, POS_R02, POS_R03, POS_R04, POS_R05, POS_R06, POS_R07, POS_R08, POS_R09,
        POS_R10, POS_R11, POS_R12, POS_R13, POS_R14, POS_R15, POS_RT1, POS_RT2, POS_RT3,
};

#define LEFT_HAND_COUNT  (sizeof(left_hand_positions) / sizeof(left_hand_positions[0]))
#define RIGHT_HAND_COUNT (sizeof(right_hand_positions) / sizeof(right_hand_positions[0]))

/* -------------------------------------------------------------------
 * Combos
 *
 * Chord timeout: 50ms (adjustable).
 *
 * positions[]  Physical key positions (0xFF terminated).
 * output       Keycode produced when all keys are pressed.
 * ---------------------------------------------------------------- */

#define COMBO_END 0xFF

struct combo_config {
    uint8_t positions[5];
    uint16_t output;
};

static const struct combo_config combo_table[KEYMAP_COMBOS] = {

        /* -- Editing (vertical: top + home) -- */
        {{POS_L02, POS_L07, COMBO_END}, KC_ESC},  /* w+r  */
        {{POS_L03, POS_L08, COMBO_END}, KC_TAB},  /* f+s  */
        {{POS_R03, POS_R08, COMBO_END}, KC_DEL},  /* u+e  */
        {{POS_R04, POS_R09, COMBO_END}, CW_TOGG}, /* y+i  -> Caps Word */

        /* -- Brackets (vertical: home + bottom) -- */
        {{POS_L07, POS_L12, COMBO_END}, KC_LBRC}, /* r+x  -> [  */
        {{POS_L08, POS_L13, COMBO_END}, KC_LPRN}, /* s+c  -> (  */
        {{POS_L09, POS_L14, COMBO_END}, KC_LCBR}, /* t+d  -> {  */
        {{POS_R07, POS_R12, COMBO_END}, KC_RCBR}, /* n+h  -> }  */
        {{POS_R08, POS_R13, COMBO_END}, KC_RPRN}, /* e+,  -> )  */
        {{POS_R09, POS_R14, COMBO_END}, KC_RBRC}, /* i+.  -> ]  */

        /* -- Displaced keys (vertical) -- */
        {{POS_R02, POS_R07, COMBO_END}, KC_BSLS}, /* l+n  -> \  */
        {{POS_L01, POS_L06, COMBO_END}, KC_GRV},  /* q+a  -> `  */
        {{POS_R05, POS_R10, COMBO_END}, KC_TILD}, /* '+o  -> ~  */
        {{POS_L04, POS_L09, COMBO_END}, OS_HYPR}, /* p+t  -> Hyper */

        /* -- Programming shortcuts (horizontal) -- */
        {{POS_R13, POS_R14, COMBO_END}, KC_MINS}, /* ,+.  -> -  */
        {{POS_R14, POS_R15, COMBO_END}, KC_EQL},  /* .+;  -> =  */
        {{POS_R12, POS_R13, COMBO_END}, KC_UNDS}, /* h+,  -> _  */
        {{POS_L12, POS_L13, COMBO_END}, KC_CAPS}, /* x+c  -> Caps Lock */

        /* -- Layer toggles -- */
        {{POS_LT1, POS_LT2, POS_LT3, COMBO_END}, TG(L_GAME)},    /* all L thumbs -> Gaming  */
        {{POS_RT1, POS_RT2, POS_RT3, COMBO_END}, TG(L_GAMEALT)}, /* all R thumbs -> Game Alt */
        {{POS_LT1, POS_RT3, COMBO_END}, TG(L_NOMOD)},            /* outer thumbs -> Nomods  */
        {{POS_LT1, POS_LT2, POS_RT2, POS_RT3, COMBO_END}, TG(L_ORIGIN)}, /* 4 thumbs -> Origin */
};

/* -------------------------------------------------------------------
 * SOCD pairs  (gaming layer only)
 *
 * Resolution: last-input-wins.
 * Uses struct socd_pair from firmware/include/socd.h.
 * ---------------------------------------------------------------- */

struct socd_assignment {
    uint8_t pos_a;
    uint8_t pos_b;
};

static const struct socd_assignment socd_table[KEYMAP_SOCD_PAIRS] = {
        {POS_L03, POS_L08}, /* W / S  (vertical)   */
        {POS_L07, POS_L09}, /* A / D  (horizontal) */
};

#endif /* KEYMAP_CONFIG_H */
