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
    return keymap[L_BASE][pos];
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

    TEST_ASSERT_EQUAL_size_t(8, HRM_CONFIG_COUNT);
    for (size_t i = 0; i < HRM_CONFIG_COUNT; i++) {
        TEST_ASSERT_EQUAL_HEX16(expected[i], base_key(hrm_config_table[i].position));
    }
}

void test_hrm_table_splits_evenly_across_hands(void)
{
    size_t left = 0;

    for (size_t i = 0; i < HRM_CONFIG_COUNT; i++) {
        for (size_t j = 0; j < LEFT_HAND_COUNT; j++) {
            if (left_hand_positions[j] == hrm_config_table[i].position) {
                left++;
            }
        }
    }
    TEST_ASSERT_EQUAL_size_t(HRM_CONFIG_COUNT / 2, left);
}

void test_hand_sets_partition_all_keys(void)
{
    uint8_t seen[KEYMAP_KEYS] = {0};

    TEST_ASSERT_EQUAL_size_t(LEFT_HAND_COUNT, RIGHT_HAND_COUNT);
    for (size_t i = 0; i < LEFT_HAND_COUNT; i++) {
        seen[left_hand_positions[i]]++;
    }
    for (size_t i = 0; i < RIGHT_HAND_COUNT; i++) {
        seen[right_hand_positions[i]]++;
    }
    for (size_t p = 0; p < KEYMAP_KEYS; p++) {
        TEST_ASSERT_EQUAL_UINT8(1, seen[p]);
    }
}

void test_combos_are_well_formed(void)
{
    for (size_t i = 0; i < KEYMAP_COMBOS; i++) {
        const struct combo_config *c = &combo_table[i];
        size_t n = 0;

        while (n < 5 && c->positions[n] != POS_NONE) {
            TEST_ASSERT_LESS_THAN_INT(KEYMAP_KEYS, c->positions[n]);
            n++;
        }
        TEST_ASSERT_GREATER_OR_EQUAL_size_t(2, n);
        TEST_ASSERT_LESS_THAN_size_t(5, n);
    }
}

void test_socd_pairs_are_opposing_gaming_keys(void)
{
    TEST_ASSERT_EQUAL_HEX16(KC_W, keymap[L_GAME][socd_table[0].pos_a]);
    TEST_ASSERT_EQUAL_HEX16(KC_S, keymap[L_GAME][socd_table[0].pos_b]);
    TEST_ASSERT_EQUAL_HEX16(KC_A, keymap[L_GAME][socd_table[1].pos_a]);
    TEST_ASSERT_EQUAL_HEX16(KC_D, keymap[L_GAME][socd_table[1].pos_b]);
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

    return UNITY_END();
}
