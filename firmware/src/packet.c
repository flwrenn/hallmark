/*
 * Inter-half packet -- implementation.
 */

#include "packet.h"

#include <string.h>

#define PKT_CRC8_POLY  0x07
#define PKT_HALF_INDEX 1
#define PKT_POS_INDEX  2
#define PKT_CRC_INDEX  (PKT_SIZE - 1)

uint8_t pkt_crc8(const uint8_t *data, size_t len)
{
    uint8_t crc = 0;

    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? (uint8_t)((crc << 1) ^ PKT_CRC8_POLY) : (uint8_t)(crc << 1);
        }
    }
    return crc;
}

void pkt_pack(const struct pkt_frame *frame, uint8_t out[PKT_SIZE])
{
    out[0] = PKT_SYNC;
    out[PKT_HALF_INDEX] = (uint8_t)frame->half;
    for (int k = 0; k < PKT_KEY_COUNT; k++) {
        out[PKT_POS_INDEX + 2 * k] = (uint8_t)(frame->pos[k] & 0xFF);
        out[PKT_POS_INDEX + 2 * k + 1] = (uint8_t)(frame->pos[k] >> 8);
    }
    out[PKT_CRC_INDEX] = pkt_crc8(out, PKT_CRC_INDEX);
}

enum pkt_status pkt_unpack(const uint8_t in[PKT_SIZE], struct pkt_frame *frame)
{
    if (in[0] != PKT_SYNC) {
        return PKT_ERR_SYNC;
    }
    if (pkt_crc8(in, PKT_CRC_INDEX) != in[PKT_CRC_INDEX]) {
        return PKT_ERR_CRC;
    }
    if (in[PKT_HALF_INDEX] != PKT_HALF_LEFT && in[PKT_HALF_INDEX] != PKT_HALF_RIGHT) {
        return PKT_ERR_HALF;
    }

    frame->half = (enum pkt_half)in[PKT_HALF_INDEX];
    for (int k = 0; k < PKT_KEY_COUNT; k++) {
        frame->pos[k] =
                (uint16_t)(in[PKT_POS_INDEX + 2 * k] | (in[PKT_POS_INDEX + 2 * k + 1] << 8));
    }
    return PKT_OK;
}

void pkt_parser_init(struct pkt_parser *parser)
{
    parser->len = 0;
}

/*
 * Drop the rejected candidate's leading sync byte and keep everything
 * from the next sync byte on, which may start a real packet.
 */
static void pkt_parser_resync(struct pkt_parser *parser)
{
    const uint8_t *next = memchr(&parser->buf[1], PKT_SYNC, parser->len - 1u);

    if (next == NULL) {
        parser->len = 0;
        return;
    }

    uint8_t skip = (uint8_t)(next - parser->buf);

    memmove(parser->buf, next, parser->len - skip);
    parser->len -= skip;
}

enum pkt_parse_result pkt_parser_feed(struct pkt_parser *parser, uint8_t byte,
                                      struct pkt_frame *frame)
{
    if (parser->len == 0 && byte != PKT_SYNC) {
        return PKT_PARSE_PENDING;
    }

    parser->buf[parser->len++] = byte;
    if (parser->len < PKT_SIZE) {
        return PKT_PARSE_PENDING;
    }

    if (pkt_unpack(parser->buf, frame) == PKT_OK) {
        parser->len = 0;
        return PKT_PARSE_FRAME;
    }

    pkt_parser_resync(parser);
    return PKT_PARSE_REJECTED;
}
