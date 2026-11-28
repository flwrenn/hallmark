/*
 * Unit tests for the keymap core.
 *
 * The tests drive a small layer table built for them, so they do not
 * break when the real layout changes.  Unlisted entries are KC_TRNS.
 * Expected output follows the HID keyboard report layout documented in
 * keymap.h.
 */

#include "keymap.h"
#include "unity.h"

/* HID modifier byte bits (HID Usage Tables, keyboard page 0x07). */
#define HID_MOD_LCTL 0x01
#define HID_MOD_LSFT 0x02
#define HID_MOD_LALT 0x04
#define HID_MOD_LGUI 0x08
#define HID_MOD_RCTL 0x10
#define HID_MOD_RSFT 0x20
#define HID_MOD_RALT 0x40
#define HID_MOD_RGUI 0x80

#define NO_KEY 0x00

/* The highest non-modifier usage, on the last bit of the last bitmap byte. */
#define LAST_KEY (KC_LCTL - 1)

/*
 *  base:  L01 a   L02 S(1)  L03 b  L04 c  L05 d
 *         L06-L09 LCtl LSft LAlt LGui   R06-R09 RCtl RSft RAlt RGui
 *         L11 e   L12 f   L13 g   L14 S(2)   L15 LAST_KEY
 *         R01-R05 the kinds later slices own
 *         LT1 TG(NAV)  LT2 MO(SYM)  LT3 MO(SYM)
 *         RT1 Ent      RT2 Ent      RT3 MO(NAV)
 *  sym:   L01 1  L04 2  L05 3  L11 a kind a later slice owns
 *  nav:   L01 Left  L05 Up  LT2 Spc
 *  num:   L01 4, named only by R01's KM_TH
 */
static const uint16_t test_layers[KEYMAP_LAYERS][KEYMAP_KEYS] = {
        [L_BASE] =
                {
                        [POS_L01] = KC_A,           [POS_L02] = KM_S(KC_1),
                        [POS_L03] = KC_B,           [POS_L04] = KC_C,
                        [POS_L05] = KC_D,           [POS_R01] = KM_TH(KC_G, L_NUM),
                        [POS_R02] = KM_OS(KC_LSFT), [POS_R03] = KM_HRM(KC_E, KC_LALT),
                        [POS_R04] = KM_MACRO(0),    [POS_R05] = KM_UC(0x20AC),
                        [POS_L06] = KC_LCTL,        [POS_L07] = KC_LSFT,
                        [POS_L08] = KC_LALT,        [POS_L09] = KC_LGUI,
                        [POS_R06] = KC_RCTL,        [POS_R07] = KC_RSFT,
                        [POS_R08] = KC_RALT,        [POS_R09] = KC_RGUI,
                        [POS_L11] = KC_E,           [POS_L12] = KC_F,
                        [POS_L13] = KC_G,           [POS_L14] = KM_S(KC_2),
                        [POS_L15] = LAST_KEY,       [POS_LT1] = KM_TG(L_NAV),
                        [POS_LT2] = KM_MO(L_SYM),   [POS_LT3] = KM_MO(L_SYM),
                        [POS_RT1] = KC_ENT,         [POS_RT2] = KC_ENT,
                        [POS_RT3] = KM_MO(L_NAV),
                },
        [L_SYM] =
                {
                        [POS_L01] = KC_1,
                        [POS_L04] = KC_2,
                        [POS_L05] = KC_3,
                        [POS_L11] = KM_OS(KC_LSFT),
                },
        [L_NAV] =
                {
                        [POS_L01] = KC_LEFT,
                        [POS_L05] = KC_UP,
                        [POS_LT2] = KC_SPC,
                },
        [L_NUM] =
                {
                        [POS_L01] = KC_4,
                },
};

static struct keymap_state km;

void setUp(void)
{
    keymap_init(&km, test_layers);
}

void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Helpers                                                               */
/* --------------------------------------------------------------------- */

/* Asserts exactly these modifier bits and these keys, NO_KEY terminated, are held. */
static void assert_held_keys(uint8_t mods, const uint8_t *keys)
{
    struct keymap_held expected = {0};
    struct keymap_held actual = keymap_get_held(&km);

    expected.mods = mods;
    for (; *keys != NO_KEY; keys++) {
        expected.keys[*keys / 8] |= (uint8_t)(1u << (*keys % 8));
    }
    TEST_ASSERT_EQUAL_HEX8(expected.mods, actual.mods);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(expected.keys, actual.keys, KEYMAP_USAGE_BYTES);
}

/* Asserts exactly these modifier bits and at most one key are held. */
static void assert_held(uint8_t mods, uint8_t key)
{
    const uint8_t keys[] = {key, NO_KEY};

    assert_held_keys(mods, keys);
}

static void assert_nothing_held(void)
{
    assert_held(0, NO_KEY);
}

static void tap(enum keymap_pos pos)
{
    keymap_press(&km, pos);
    keymap_release(&km, pos);
}

/* Presses pos alone, checks what it holds, and checks its release. */
static void assert_press_holds(enum keymap_pos pos, uint8_t mods, uint8_t key)
{
    keymap_press(&km, pos);
    assert_held(mods, key);
    keymap_release(&km, pos);
    assert_nothing_held();
}

/* --------------------------------------------------------------------- */
/* Tests: keycodes                                                       */
/* --------------------------------------------------------------------- */

void test_basic_key_held_only_while_pressed(void)
{
    assert_nothing_held();
    keymap_press(&km, POS_L01);
    assert_held(0, KC_A);
    keymap_release(&km, POS_L01);
    assert_nothing_held();
}

void test_each_modifier_key_sets_its_report_bit(void)
{
    static const struct {
        enum keymap_pos pos;
        uint8_t mod;
    } cases[] = {
            {POS_L06, HID_MOD_LCTL}, {POS_L07, HID_MOD_LSFT}, {POS_L08, HID_MOD_LALT},
            {POS_L09, HID_MOD_LGUI}, {POS_R06, HID_MOD_RCTL}, {POS_R07, HID_MOD_RSFT},
            {POS_R08, HID_MOD_RALT}, {POS_R09, HID_MOD_RGUI},
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        assert_press_holds(cases[i].pos, cases[i].mod, NO_KEY);
    }
}

void test_shifted_key_holds_shift_with_its_key(void)
{
    assert_press_holds(POS_L02, HID_MOD_LSFT, KC_1);
}

void test_usage_stays_held_until_its_last_holder_releases(void)
{
    keymap_press(&km, POS_RT1);
    keymap_press(&km, POS_RT2);
    assert_held(0, KC_ENT);
    keymap_release(&km, POS_RT1);
    assert_held(0, KC_ENT);
    keymap_release(&km, POS_RT2);
    assert_nothing_held();

    keymap_press(&km, POS_L07);
    keymap_press(&km, POS_L02);
    assert_held(HID_MOD_LSFT, KC_1);
    keymap_release(&km, POS_L02);
    assert_held(HID_MOD_LSFT, NO_KEY);
}

void test_shift_stays_held_until_its_last_holder_releases(void)
{
    static const uint8_t both[] = {KC_1, KC_2, NO_KEY};

    keymap_press(&km, POS_L02);
    keymap_press(&km, POS_L14);
    assert_held_keys(HID_MOD_LSFT, both);
    keymap_release(&km, POS_L02);
    assert_held(HID_MOD_LSFT, KC_2);

    keymap_press(&km, POS_L07);
    keymap_release(&km, POS_L14);
    assert_held(HID_MOD_LSFT, NO_KEY);
    keymap_release(&km, POS_L07);
    assert_nothing_held();
}

void test_last_usage_sets_the_last_bitmap_bit(void)
{
    struct keymap_held held;

    TEST_ASSERT_EQUAL_UINT(28, sizeof(held.keys));
    keymap_press(&km, POS_L15);
    held = keymap_get_held(&km);
    TEST_ASSERT_EQUAL_HEX8(0x80, held.keys[27]);
    TEST_ASSERT_EQUAL_HEX8(0, held.mods);
    keymap_release(&km, POS_L15);
    assert_nothing_held();
}

void test_seven_keys_are_held_together(void)
{
    static const enum keymap_pos chord[] = {POS_L01, POS_L03, POS_L04, POS_L05,
                                            POS_L11, POS_L12, POS_L13};
    static const uint8_t all[] = {KC_A, KC_B, KC_C, KC_D, KC_E, KC_F, KC_G, NO_KEY};
    static const uint8_t without_c[] = {KC_A, KC_B, KC_D, KC_E, KC_F, KC_G, NO_KEY};

    for (size_t i = 0; i < sizeof(chord) / sizeof(chord[0]); i++) {
        keymap_press(&km, chord[i]);
    }
    assert_held_keys(0, all);
    keymap_release(&km, POS_L04);
    assert_held_keys(0, without_c);
}

void test_kinds_owned_by_later_slices_hold_nothing(void)
{
    static const enum keymap_pos later[] = {POS_R01, POS_R02, POS_R03, POS_R04, POS_R05};

    for (size_t i = 0; i < sizeof(later) / sizeof(later[0]); i++) {
        keymap_press(&km, later[i]);
        assert_nothing_held();
        assert_press_holds(POS_L01, 0, KC_A);
    }
}

/* SYM's L11 is a later slice's kind.  It hides base's e instead of falling through. */
void test_kind_owned_by_a_later_slice_is_not_transparent(void)
{
    keymap_press(&km, POS_LT2);
    assert_press_holds(POS_L11, 0, NO_KEY);
    keymap_release(&km, POS_LT2);
    assert_press_holds(POS_L11, 0, KC_E);
}

/* NAV is toggled on first, so a POS_NONE edge that disturbed the layers would show. */
void test_pos_none_is_ignored(void)
{
    tap(POS_LT1);
    keymap_press(&km, POS_NONE);
    assert_nothing_held();
    keymap_press(&km, POS_L01);
    keymap_release(&km, POS_NONE);
    assert_held(0, KC_LEFT);
    keymap_release(&km, POS_L01);
    assert_press_holds(POS_L01, 0, KC_LEFT);
}

/* --------------------------------------------------------------------- */
/* Tests: layers                                                         */
/* --------------------------------------------------------------------- */

void test_momentary_layer_active_only_while_held(void)
{
    keymap_press(&km, POS_LT2);
    assert_nothing_held();
    assert_press_holds(POS_L01, 0, KC_1);
    keymap_release(&km, POS_LT2);
    assert_press_holds(POS_L01, 0, KC_A);
}

void test_toggle_flips_layer_on_each_press(void)
{
    keymap_press(&km, POS_LT1);
    assert_nothing_held();
    assert_press_holds(POS_L01, 0, KC_LEFT);
    keymap_release(&km, POS_LT1);
    assert_press_holds(POS_L01, 0, KC_LEFT);

    keymap_press(&km, POS_LT1);
    assert_press_holds(POS_L01, 0, KC_A);
    keymap_release(&km, POS_LT1);
    assert_press_holds(POS_L01, 0, KC_A);
}

void test_transparent_falls_through_active_layers_to_base(void)
{
    keymap_press(&km, POS_LT2);
    tap(POS_LT1);

    assert_press_holds(POS_L05, 0, KC_UP);
    assert_press_holds(POS_L04, 0, KC_2);
    assert_press_holds(POS_L03, 0, KC_B);
    assert_press_holds(POS_L10, 0, NO_KEY);

    keymap_release(&km, POS_LT2);
    assert_press_holds(POS_L04, 0, KC_C);
}

void test_layer_stays_active_while_any_source_holds_it(void)
{
    keymap_press(&km, POS_LT2);
    keymap_press(&km, POS_LT3);
    assert_press_holds(POS_L01, 0, KC_1);
    keymap_release(&km, POS_LT2);
    assert_press_holds(POS_L01, 0, KC_1);
    keymap_release(&km, POS_LT3);
    assert_press_holds(POS_L01, 0, KC_A);

    tap(POS_LT1);
    keymap_press(&km, POS_RT3);
    assert_press_holds(POS_L01, 0, KC_LEFT);
    keymap_release(&km, POS_RT3);
    assert_press_holds(POS_L01, 0, KC_LEFT);
}

/* --------------------------------------------------------------------- */
/* Tests: release after a layer change                                   */
/* --------------------------------------------------------------------- */

void test_key_keeps_its_layer_output_after_layer_released(void)
{
    keymap_press(&km, POS_LT2);
    keymap_press(&km, POS_L01);
    keymap_release(&km, POS_LT2);
    assert_held(0, KC_1);
    keymap_release(&km, POS_L01);
    assert_nothing_held();
}

void test_key_pressed_before_layer_releases_its_base_output(void)
{
    keymap_press(&km, POS_L01);
    keymap_press(&km, POS_LT2);
    assert_held(0, KC_A);
    keymap_release(&km, POS_L01);
    assert_nothing_held();
}

/* On NAV, the MO(SYM) position is Spc.  Its release still ends SYM. */
void test_layer_key_releases_its_layer_after_layers_change(void)
{
    keymap_press(&km, POS_LT2);
    tap(POS_LT1);
    keymap_release(&km, POS_LT2);
    assert_nothing_held();
    assert_press_holds(POS_L04, 0, KC_C);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_basic_key_held_only_while_pressed);
    RUN_TEST(test_each_modifier_key_sets_its_report_bit);
    RUN_TEST(test_shifted_key_holds_shift_with_its_key);
    RUN_TEST(test_usage_stays_held_until_its_last_holder_releases);
    RUN_TEST(test_shift_stays_held_until_its_last_holder_releases);
    RUN_TEST(test_last_usage_sets_the_last_bitmap_bit);
    RUN_TEST(test_seven_keys_are_held_together);
    RUN_TEST(test_kinds_owned_by_later_slices_hold_nothing);
    RUN_TEST(test_kind_owned_by_a_later_slice_is_not_transparent);
    RUN_TEST(test_pos_none_is_ignored);

    RUN_TEST(test_momentary_layer_active_only_while_held);
    RUN_TEST(test_toggle_flips_layer_on_each_press);
    RUN_TEST(test_transparent_falls_through_active_layers_to_base);
    RUN_TEST(test_layer_stays_active_while_any_source_holds_it);

    RUN_TEST(test_key_keeps_its_layer_output_after_layer_released);
    RUN_TEST(test_key_pressed_before_layer_releases_its_base_output);
    RUN_TEST(test_layer_key_releases_its_layer_after_layers_change);

    return UNITY_END();
}
