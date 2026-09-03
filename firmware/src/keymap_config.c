/*
 * Keymap configuration tables for Hallmark.
 *
 * Layer arrays, home row mod thresholds, hand sets, combos, and SOCD
 * pairs.  Types, keycodes, and dimensions live in keymap_config.h.
 */

#include "keymap_config.h"

#define KEYMAP_LEN(a) (sizeof(a) / sizeof((a)[0]))

/* -------------------------------------------------------------------
 * Layer arrays
 *
 * Each layer is a flat array of KEYMAP_KEYS entries in position order:
 *
 *   L01  L02  L03  L04  L05    R01  R02  R03  R04  R05
 *   L06  L07  L08  L09  L10    R06  R07  R08  R09  R10
 *   L11  L12  L13  L14  L15    R11  R12  R13  R14  R15
 *              LT1  LT2  LT3    RT1  RT2  RT3
 *
 * ___ in the per-layer diagrams is KC_TRNS.
 * ---------------------------------------------------------------- */

const uint16_t keymap_layers[KEYMAP_LAYERS][KEYMAP_KEYS] = {

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
                   KC_AMPR, KC_QUES, KC_EXLM, MO_NUM,  KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},

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
                   KC_PGUP, KC_INS,  KC_ENT,  KC_TRNS, KC_TRNS, KC_TRNS, MO_NUM,  KC_TRNS, KC_TRNS},

        /* Layer 3: Numeric
         *
         *   F10   F11   ___   ___   ___        ___   ___   ___   ___   ___
         *   7/S   5/C   3/A   1/G   9          8     0/G   2/A   4/C   6/S
         *   F7    F5    F3    F1    F9         F8    F12   F2    F4    F6
         *               ___   ___   ___        ___   ___   ___
         */
        [L_NUM] = {KC_F10,  KC_F11, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                   KC_TRNS, HM_7,   HM_5,    HM_3,    HM_1,    KC_9,    KC_8,    HM_0,    HM_2,
                   HM_4,    HM_6,   KC_F7,   KC_F5,   KC_F3,   KC_F1,   KC_F9,   KC_F8,   KC_F12,
                   KC_F2,   KC_F4,  KC_F6,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},

        /* Layer 4: Gaming
         *
         *   Tab   Q     W     E     R          ___   ___   ___   ___   ___
         *   Esc   A     S     D     F          ___   ___   ___   ___   ___
         *   LSf   Z     X     C     V          ___   ___   ___   ___   ___
         *               LCt   G/NUM Spc        ___   ___   ___
         */
        [L_GAME] = {KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_TRNS, KC_TRNS, KC_TRNS,
                    KC_TRNS, KC_TRNS, KC_ESC,  KC_A,    KC_S,    KC_D,    KC_F,    KC_TRNS,
                    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LSFT, KC_Z,    KC_X,    KC_C,
                    KC_V,    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LCTL, MO_GNUM,
                    KC_SPC,  KC_TRNS, KC_TRNS, KC_TRNS},

        /* Layer 5: Gaming Numbers  (hold LT2 from Gaming layer)
         *
         *   1     2     3     4     5          ___   ___   ___   ___   ___
         *   6     7     8     9     0          ___   ___   ___   ___   ___
         *   F1    F2    F3    F4    F5         ___   ___   ___   ___   ___
         *               ___   ___   ___        ___   ___   ___
         */
        [L_GAMENUM] = {KC_1,    KC_2,    KC_3,    KC_4,    KC_5,    KC_TRNS, KC_TRNS, KC_TRNS,
                       KC_TRNS, KC_TRNS, KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_TRNS,
                       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_F1,   KC_F2,   KC_F3,   KC_F4,
                       KC_F5,   KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS,
                       KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS},

        /* Layer 6: Gaming Alt
         *
         *   1     2     W     3     4          6     ___   ___   ___   ___
         *   Q     A     S     D     E          T     B     V     Y     Esc
         *   Tab   5     ___   F     G          P     ___   ___   ___   ___
         *               Ctl   Sft   Spc        ___   ___   ___
         */
        [L_GAMEALT] = {KC_1,    KC_2,    KC_W,    KC_3,    KC_4,    KC_6,    KC_TRNS, KC_TRNS,
                       KC_TRNS, KC_TRNS, KC_Q,    KC_A,    KC_S,    KC_D,    KC_E,    KC_T,
                       KC_B,    KC_V,    KC_Y,    KC_ESC,  KC_TAB,  KC_5,    KC_TRNS, KC_F,
                       KC_G,    KC_P,    KC_TRNS, KC_TRNS, KC_TRNS, KC_TRNS, KC_LCTL, KC_LSFT,
                       KC_SPC,  KC_TRNS, KC_TRNS, KC_TRNS},

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
 * ---------------------------------------------------------------- */

const struct keymap_hrm keymap_hrm_table[] = {
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

_Static_assert(KEYMAP_LEN(keymap_hrm_table) == KEYMAP_HRM_COUNT, "KEYMAP_HRM_COUNT");

/* -------------------------------------------------------------------
 * Bilateral key sets
 *
 * A modifier only fires when the opposite hand presses a key.
 * ---------------------------------------------------------------- */

const enum keymap_pos keymap_left_hand[] = {
        POS_L01, POS_L02, POS_L03, POS_L04, POS_L05, POS_L06, POS_L07, POS_L08, POS_L09,
        POS_L10, POS_L11, POS_L12, POS_L13, POS_L14, POS_L15, POS_LT1, POS_LT2, POS_LT3,
};

const enum keymap_pos keymap_right_hand[] = {
        POS_R01, POS_R02, POS_R03, POS_R04, POS_R05, POS_R06, POS_R07, POS_R08, POS_R09,
        POS_R10, POS_R11, POS_R12, POS_R13, POS_R14, POS_R15, POS_RT1, POS_RT2, POS_RT3,
};

_Static_assert(KEYMAP_LEN(keymap_left_hand) == KEYMAP_HAND_KEYS, "KEYMAP_HAND_KEYS");
_Static_assert(KEYMAP_LEN(keymap_right_hand) == KEYMAP_HAND_KEYS, "KEYMAP_HAND_KEYS");

/* -------------------------------------------------------------------
 * Combos
 *
 * Chord timeout: 50ms (adjustable).
 * ---------------------------------------------------------------- */

const struct keymap_combo keymap_combos[] = {

        /* -- Editing (vertical: top + home) -- */
        {{POS_L02, POS_L07, POS_NONE}, KC_ESC},  /* w+r  */
        {{POS_L03, POS_L08, POS_NONE}, KC_TAB},  /* f+s  */
        {{POS_R03, POS_R08, POS_NONE}, KC_DEL},  /* u+e  */
        {{POS_R04, POS_R09, POS_NONE}, CW_TOGG}, /* y+i  -> Caps Word */

        /* -- Brackets (vertical: home + bottom) -- */
        {{POS_L07, POS_L12, POS_NONE}, KC_LBRC}, /* r+x  -> [  */
        {{POS_L08, POS_L13, POS_NONE}, KC_LPRN}, /* s+c  -> (  */
        {{POS_L09, POS_L14, POS_NONE}, KC_LCBR}, /* t+d  -> {  */
        {{POS_R07, POS_R12, POS_NONE}, KC_RCBR}, /* n+h  -> }  */
        {{POS_R08, POS_R13, POS_NONE}, KC_RPRN}, /* e+,  -> )  */
        {{POS_R09, POS_R14, POS_NONE}, KC_RBRC}, /* i+.  -> ]  */

        /* -- Displaced keys (vertical) -- */
        {{POS_R02, POS_R07, POS_NONE}, KC_BSLS}, /* l+n  -> \  */
        {{POS_L01, POS_L06, POS_NONE}, KC_GRV},  /* q+a  -> `  */
        {{POS_R05, POS_R10, POS_NONE}, KC_TILD}, /* '+o  -> ~  */
        {{POS_L04, POS_L09, POS_NONE}, OS_HYPR}, /* p+t  -> Hyper */

        /* -- Programming shortcuts (horizontal) -- */
        {{POS_R13, POS_R14, POS_NONE}, KC_MINS}, /* ,+.  -> -  */
        {{POS_R14, POS_R15, POS_NONE}, KC_EQL},  /* .+;  -> =  */
        {{POS_R12, POS_R13, POS_NONE}, KC_UNDS}, /* h+,  -> _  */
        {{POS_L12, POS_L13, POS_NONE}, KC_CAPS}, /* x+c  -> Caps Lock */

        /* -- Layer toggles -- */
        {{POS_LT1, POS_LT2, POS_LT3, POS_NONE}, KM_TG(L_GAME)},    /* all L thumbs -> Gaming  */
        {{POS_RT1, POS_RT2, POS_RT3, POS_NONE}, KM_TG(L_GAMEALT)}, /* all R thumbs -> Game Alt */
        {{POS_LT1, POS_RT3, POS_NONE}, KM_TG(L_NOMOD)},            /* outer thumbs -> Nomods  */
        {{POS_LT1, POS_LT2, POS_RT2, POS_RT3, POS_NONE}, KM_TG(L_ORIGIN)}, /* 4 thumbs -> Origin */
};

_Static_assert(KEYMAP_LEN(keymap_combos) == KEYMAP_COMBOS, "KEYMAP_COMBOS");

/* -------------------------------------------------------------------
 * SOCD pairs  (gaming layer only)
 *
 * Resolution: last-input-wins.
 * ---------------------------------------------------------------- */

const struct keymap_socd_pair keymap_socd_pairs[] = {
        {POS_L03, POS_L08}, /* W / S  (vertical)   */
        {POS_L07, POS_L09}, /* A / D  (horizontal) */
};

_Static_assert(KEYMAP_LEN(keymap_socd_pairs) == KEYMAP_SOCD_PAIRS, "KEYMAP_SOCD_PAIRS");
