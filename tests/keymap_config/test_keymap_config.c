/*
 * Structural checks for the keymap configuration tables.
 *
 * The tables are data, so the tests pin the invariants a consumer
 * relies on: position indices address the layer arrays, every
 * position-typed entry points at the key its comment names, and
 * the hand sets partition the board.
 */

#include "keymap_config.h"
#include "unity.h"

void setUp(void)
{
}

void tearDown(void)
{
}

static uint16_t base_key(int pos)
{
    return keymap_layers[L_BASE][pos];
}

void test_positions_index_layers_row_major(void)
{
    TEST_ASSERT_EQUAL_HEX16(KC_Q, base_key(POS_L01));
    TEST_ASSERT_EQUAL_HEX16(KC_B, base_key(POS_L05));
    TEST_ASSERT_EQUAL_HEX16(KC_J, base_key(POS_R01));
    TEST_ASSERT_EQUAL_HEX16(KC_QUOT, base_key(POS_R05));
    TEST_ASSERT_EQUAL_HEX16(HM_A, base_key(POS_L06));
    TEST_ASSERT_EQUAL_HEX16(KC_G, base_key(POS_L10));
    TEST_ASSERT_EQUAL_HEX16(KC_M, base_key(POS_R06));
    TEST_ASSERT_EQUAL_HEX16(HM_O, base_key(POS_R10));
    TEST_ASSERT_EQUAL_HEX16(KC_Z, base_key(POS_L11));
    TEST_ASSERT_EQUAL_HEX16(KC_SCLN, base_key(POS_R15));
    TEST_ASSERT_EQUAL_HEX16(KC_ESC, base_key(POS_LT1));
    TEST_ASSERT_EQUAL_HEX16(MO_NAV, base_key(POS_LT2));
    TEST_ASSERT_EQUAL_HEX16(MO_SYM, base_key(POS_RT2));
    TEST_ASSERT_EQUAL_HEX16(KC_BSPC, base_key(POS_RT3));
}

void test_hrm_table_targets_home_row_keys(void)
{
    static const uint16_t expected[] = {HM_A, HM_R, HM_S, HM_T, HM_N, HM_E, HM_I, HM_O};

    TEST_ASSERT_EQUAL_size_t(8, KEYMAP_HRM_COUNT);
    for (size_t i = 0; i < KEYMAP_HRM_COUNT; i++) {
        TEST_ASSERT_EQUAL_HEX16(expected[i], base_key(keymap_hrm_table[i].position));
    }
}

void test_hrm_table_splits_evenly_across_hands(void)
{
    size_t left = 0;

    for (size_t i = 0; i < KEYMAP_HRM_COUNT; i++) {
        for (size_t j = 0; j < KEYMAP_HAND_KEYS; j++) {
            if (keymap_left_hand[j] == keymap_hrm_table[i].position) {
                left++;
            }
        }
    }
    TEST_ASSERT_EQUAL_size_t(KEYMAP_HRM_COUNT / 2, left);
}

void test_hand_sets_partition_all_keys(void)
{
    uint8_t seen[KEYMAP_KEYS] = {0};

    for (size_t i = 0; i < KEYMAP_HAND_KEYS; i++) {
        seen[keymap_left_hand[i]]++;
    }
    for (size_t i = 0; i < KEYMAP_HAND_KEYS; i++) {
        seen[keymap_right_hand[i]]++;
    }
    for (size_t p = 0; p < KEYMAP_KEYS; p++) {
        TEST_ASSERT_EQUAL_UINT8(1, seen[p]);
    }
}

void test_combos_are_well_formed(void)
{
    for (size_t i = 0; i < KEYMAP_COMBOS; i++) {
        const struct keymap_combo *c = &keymap_combos[i];
        size_t n = 0;

        while (n <= KEYMAP_COMBO_MAX_KEYS && c->positions[n] != POS_NONE) {
            TEST_ASSERT_LESS_THAN_INT(KEYMAP_KEYS, c->positions[n]);
            n++;
        }
        TEST_ASSERT_GREATER_OR_EQUAL_size_t(2, n);
        TEST_ASSERT_LESS_OR_EQUAL_size_t(KEYMAP_COMBO_MAX_KEYS, n);
    }
}

void test_socd_pairs_are_opposing_gaming_keys(void)
{
    TEST_ASSERT_EQUAL_HEX16(KC_W, keymap_layers[L_GAME][keymap_socd_pairs[0].pos_a]);
    TEST_ASSERT_EQUAL_HEX16(KC_S, keymap_layers[L_GAME][keymap_socd_pairs[0].pos_b]);
    TEST_ASSERT_EQUAL_HEX16(KC_A, keymap_layers[L_GAME][keymap_socd_pairs[1].pos_a]);
    TEST_ASSERT_EQUAL_HEX16(KC_D, keymap_layers[L_GAME][keymap_socd_pairs[1].pos_b]);
}

void test_action_aliases_are_distinct(void)
{
    static const uint16_t aliases[] = {
            KC_TRNS,
            KC_A,
            KC_1,
            KC_QUOT,
            KC_ESC,
            KC_LEFT,
            KC_F1,
            KC_LSFT,
            KC_RGUI,
            KC_EXLM,
            KC_AT,
            KC_HASH,
            KC_DLR,
            KC_PERC,
            KC_CIRC,
            KC_AMPR,
            KC_ASTR,
            KC_LPRN,
            KC_RPRN,
            KC_UNDS,
            KC_PLUS,
            KC_LCBR,
            KC_RCBR,
            KC_PIPE,
            KC_TILD,
            KC_QUES,
            KC_EUR,
            MC_PTAB,
            MC_NTAB,
            MC_BACK,
            MC_FWD,
            MC_PREV,
            MC_NEXT,
            MC_PLAY,
            MC_BRUP,
            MC_BRDN,
            MC_VOLU,
            MC_VOLD,
            MC_MUTE,
            OS_LSFT,
            OS_LCTL,
            OS_LALT,
            OS_LGUI,
            OS_RSFT,
            OS_RCTL,
            OS_RALT,
            OS_RGUI,
            OS_HYPR,
            CW_TOGG,
            HM_A,
            HM_R,
            HM_S,
            HM_T,
            HM_N,
            HM_E,
            HM_I,
            HM_O,
            HM_7,
            HM_5,
            HM_3,
            HM_1,
            HM_0,
            HM_2,
            HM_4,
            HM_6,
            MO_NAV,
            MO_SYM,
            MO_NUM,
            MO_GNUM,
            KM_TG(L_GAME),
            KM_TG(L_GAMEALT),
            KM_TG(L_NOMOD),
            KM_TG(L_ORIGIN),
    };
    const size_t n = sizeof(aliases) / sizeof(aliases[0]);

    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            TEST_ASSERT_NOT_EQUAL_HEX16(aliases[i], aliases[j]);
        }
    }
}

void test_action_kind_survives_encoding(void)
{
    TEST_ASSERT_EQUAL_HEX16(KM_KIND_HRM, KM_KIND(HM_A));
    TEST_ASSERT_EQUAL_HEX16(KM_KIND_HRM, KM_KIND(HM_O));
    TEST_ASSERT_EQUAL_HEX16(KM_KIND_TH, KM_KIND(MO_GNUM));
    TEST_ASSERT_EQUAL_HEX16(KM_KIND_UC, KM_KIND(KC_EUR));
    TEST_ASSERT_NOT_EQUAL_HEX16(KM_UC(0x00AC), KM_UC(0x20AC));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_positions_index_layers_row_major);
    RUN_TEST(test_hrm_table_targets_home_row_keys);
    RUN_TEST(test_hrm_table_splits_evenly_across_hands);
    RUN_TEST(test_hand_sets_partition_all_keys);
    RUN_TEST(test_combos_are_well_formed);
    RUN_TEST(test_socd_pairs_are_opposing_gaming_keys);
    RUN_TEST(test_action_aliases_are_distinct);
    RUN_TEST(test_action_kind_survives_encoding);

    return UNITY_END();
}
