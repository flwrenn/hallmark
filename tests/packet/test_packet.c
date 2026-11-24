/*
 * Unit tests for the inter-half packet: pack, unpack, CRC, and the
 * byte-stream parser.
 */

#include "packet.h"
#include "unity.h"

#include <string.h>

#define MAX_FRAMES 4

/* --------------------------------------------------------------------- */
/* Test helpers                                                          */
/* --------------------------------------------------------------------- */

static const struct pkt_frame frame_a = {
        .half = PKT_HALF_LEFT,
        .pos = {200, 900, 0x0FFF, 0, 0xFFFF, 0x1234},
};

/* Positions whose bytes contain PKT_SYNC, so a lost sync byte leaves
 * false sync bytes in the stream. */
static const struct pkt_frame frame_b = {
        .half = PKT_HALF_RIGHT,
        .pos = {0xA5A5, 0x00A5, 0xA500, 303, 220, 0x0A5A},
};

/*
 * Results of feeding a byte stream through a fresh parser.
 */
struct feed_result {
    int frames;
    int rejected;
    struct pkt_frame frame[MAX_FRAMES];
};

static struct feed_result feed_all(const uint8_t *bytes, size_t len)
{
    struct pkt_parser parser;
    struct feed_result res = {0};
    struct pkt_frame frame;

    pkt_parser_init(&parser);
    for (size_t i = 0; i < len; i++) {
        switch (pkt_parser_feed(&parser, bytes[i], &frame)) {
        case PKT_PARSE_FRAME:
            TEST_ASSERT_LESS_THAN_INT(MAX_FRAMES, res.frames);
            res.frame[res.frames++] = frame;
            break;
        case PKT_PARSE_REJECTED:
            res.rejected++;
            break;
        case PKT_PARSE_PENDING:
            break;
        }
    }
    return res;
}

static void assert_frame_equal(const struct pkt_frame *expected, const struct pkt_frame *actual)
{
    TEST_ASSERT_EQUAL_INT(expected->half, actual->half);
    TEST_ASSERT_EQUAL_UINT16_ARRAY(expected->pos, actual->pos, PKT_KEY_COUNT);
}

void setUp(void)
{
}
void tearDown(void)
{
}

/* --------------------------------------------------------------------- */
/* Tests: CRC                                                            */
/* --------------------------------------------------------------------- */

/* Check value for CRC-8/SMBUS from the reveng CRC catalogue. */
void test_crc8_known_answer(void)
{
    const char *check = "123456789";

    TEST_ASSERT_EQUAL_HEX8(0xF4, pkt_crc8((const uint8_t *)check, strlen(check)));
}

void test_crc8_of_nothing_is_zero(void)
{
    TEST_ASSERT_EQUAL_HEX8(0x00, pkt_crc8(NULL, 0));
}

/* --------------------------------------------------------------------- */
/* Tests: pack and unpack                                                */
/* --------------------------------------------------------------------- */

void test_pack_layout(void)
{
    uint8_t pkt[PKT_SIZE];

    pkt_pack(&frame_a, pkt);

    TEST_ASSERT_EQUAL_HEX8(PKT_SYNC, pkt[0]);
    TEST_ASSERT_EQUAL_HEX8(PKT_HALF_LEFT, pkt[1]);
    TEST_ASSERT_EQUAL_HEX8(0xC8, pkt[2]);
    TEST_ASSERT_EQUAL_HEX8(0x00, pkt[3]);
    TEST_ASSERT_EQUAL_HEX8(0x84, pkt[4]);
    TEST_ASSERT_EQUAL_HEX8(0x03, pkt[5]);
    TEST_ASSERT_EQUAL_HEX8(0x34, pkt[12]);
    TEST_ASSERT_EQUAL_HEX8(0x12, pkt[13]);
    TEST_ASSERT_EQUAL_HEX8(pkt_crc8(pkt, PKT_SIZE - 1), pkt[PKT_SIZE - 1]);
}

void test_round_trip_left(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_a, pkt);

    TEST_ASSERT_EQUAL_INT(PKT_OK, pkt_unpack(pkt, &out));
    assert_frame_equal(&frame_a, &out);
}

void test_round_trip_right(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_b, pkt);

    TEST_ASSERT_EQUAL_INT(PKT_OK, pkt_unpack(pkt, &out));
    assert_frame_equal(&frame_b, &out);
}

void test_every_single_bit_flip_detected(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_a, pkt);
    for (int bit = 0; bit < PKT_SIZE * 8; bit++) {
        pkt[bit / 8] ^= (uint8_t)(1u << (bit % 8));
        TEST_ASSERT_NOT_EQUAL_INT(PKT_OK, pkt_unpack(pkt, &out));
        pkt[bit / 8] ^= (uint8_t)(1u << (bit % 8));
    }
}

void test_every_double_bit_flip_detected(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_b, pkt);
    for (int a = 0; a < PKT_SIZE * 8; a++) {
        for (int b = a + 1; b < PKT_SIZE * 8; b++) {
            pkt[a / 8] ^= (uint8_t)(1u << (a % 8));
            pkt[b / 8] ^= (uint8_t)(1u << (b % 8));
            TEST_ASSERT_NOT_EQUAL_INT(PKT_OK, pkt_unpack(pkt, &out));
            pkt[a / 8] ^= (uint8_t)(1u << (a % 8));
            pkt[b / 8] ^= (uint8_t)(1u << (b % 8));
        }
    }
}

void test_wrong_sync_rejected(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_a, pkt);
    pkt[0] = 0x5A;

    TEST_ASSERT_EQUAL_INT(PKT_ERR_SYNC, pkt_unpack(pkt, &out));
}

void test_unknown_half_rejected_even_with_valid_crc(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out;

    pkt_pack(&frame_a, pkt);
    pkt[1] = 2;
    pkt[PKT_SIZE - 1] = pkt_crc8(pkt, PKT_SIZE - 1);

    TEST_ASSERT_EQUAL_INT(PKT_ERR_HALF, pkt_unpack(pkt, &out));
}

void test_unpack_leaves_frame_untouched_on_error(void)
{
    uint8_t pkt[PKT_SIZE];
    struct pkt_frame out = frame_b;

    pkt_pack(&frame_a, pkt);
    pkt[PKT_SIZE - 1] ^= 0x01;

    TEST_ASSERT_EQUAL_INT(PKT_ERR_CRC, pkt_unpack(pkt, &out));
    assert_frame_equal(&frame_b, &out);
}

/* --------------------------------------------------------------------- */
/* Tests: byte-stream parser                                             */
/* --------------------------------------------------------------------- */

void test_parser_single_packet(void)
{
    uint8_t stream[PKT_SIZE];

    pkt_pack(&frame_a, stream);
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(1, res.frames);
    TEST_ASSERT_EQUAL_INT(0, res.rejected);
    assert_frame_equal(&frame_a, &res.frame[0]);
}

void test_parser_back_to_back_packets(void)
{
    uint8_t stream[2 * PKT_SIZE];

    pkt_pack(&frame_a, stream);
    pkt_pack(&frame_b, stream + PKT_SIZE);
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(2, res.frames);
    TEST_ASSERT_EQUAL_INT(0, res.rejected);
    assert_frame_equal(&frame_a, &res.frame[0]);
    assert_frame_equal(&frame_b, &res.frame[1]);
}

void test_parser_truncated_packet_yields_nothing(void)
{
    uint8_t stream[PKT_SIZE];

    pkt_pack(&frame_a, stream);
    struct feed_result res = feed_all(stream, PKT_SIZE - 1);

    TEST_ASSERT_EQUAL_INT(0, res.frames);
    TEST_ASSERT_EQUAL_INT(0, res.rejected);
}

void test_parser_recovers_after_truncated_packet(void)
{
    uint8_t stream[2 * PKT_SIZE - 1];

    pkt_pack(&frame_b, stream);
    pkt_pack(&frame_a, stream + PKT_SIZE - 1);
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(1, res.frames);
    TEST_ASSERT_GREATER_THAN_INT(0, res.rejected);
    assert_frame_equal(&frame_a, &res.frame[0]);
}

void test_parser_skips_garbage_before_packet(void)
{
    uint8_t stream[6 + PKT_SIZE] = {0x00, 0xA5, 0x13, 0xA5, 0xA5, 0xFF};

    pkt_pack(&frame_a, stream + 6);
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(1, res.frames);
    assert_frame_equal(&frame_a, &res.frame[0]);
}

void test_parser_recovers_after_lost_sync_byte(void)
{
    uint8_t stream[2 * PKT_SIZE - 1];

    pkt_pack(&frame_b, stream);
    memmove(stream, stream + 1, PKT_SIZE - 1);
    pkt_pack(&frame_a, stream + PKT_SIZE - 1);
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(1, res.frames);
    TEST_ASSERT_GREATER_THAN_INT(0, res.rejected);
    assert_frame_equal(&frame_a, &res.frame[0]);
}

void test_parser_rejects_corrupt_packet_and_keeps_next(void)
{
    uint8_t stream[2 * PKT_SIZE];

    pkt_pack(&frame_a, stream);
    pkt_pack(&frame_b, stream + PKT_SIZE);
    stream[5] ^= 0x10;
    struct feed_result res = feed_all(stream, sizeof(stream));

    TEST_ASSERT_EQUAL_INT(1, res.frames);
    TEST_ASSERT_GREATER_THAN_INT(0, res.rejected);
    assert_frame_equal(&frame_b, &res.frame[0]);
}

/* --------------------------------------------------------------------- */
/* Runner                                                                */
/* --------------------------------------------------------------------- */

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_crc8_known_answer);
    RUN_TEST(test_crc8_of_nothing_is_zero);

    RUN_TEST(test_pack_layout);
    RUN_TEST(test_round_trip_left);
    RUN_TEST(test_round_trip_right);
    RUN_TEST(test_every_single_bit_flip_detected);
    RUN_TEST(test_every_double_bit_flip_detected);
    RUN_TEST(test_wrong_sync_rejected);
    RUN_TEST(test_unknown_half_rejected_even_with_valid_crc);
    RUN_TEST(test_unpack_leaves_frame_untouched_on_error);

    RUN_TEST(test_parser_single_packet);
    RUN_TEST(test_parser_back_to_back_packets);
    RUN_TEST(test_parser_truncated_packet_yields_nothing);
    RUN_TEST(test_parser_recovers_after_truncated_packet);
    RUN_TEST(test_parser_skips_garbage_before_packet);
    RUN_TEST(test_parser_recovers_after_lost_sync_byte);
    RUN_TEST(test_parser_rejects_corrupt_packet_and_keeps_next);

    return UNITY_END();
}
