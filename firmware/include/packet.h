/*
 * Inter-half packet -- frames one half's key positions for the wire.
 *
 * Pure C module, no RTOS dependencies.  The transport moves the bytes;
 * this module only packs, unpacks, and finds packets in a byte stream.
 *
 * Layout (PKT_SIZE bytes, positions little-endian):
 *
 *   [0]      PKT_SYNC
 *   [1]      half id (enum pkt_half)
 *   [2..13]  PKT_KEY_COUNT x uint16_t positions
 *   [14]     CRC-8 of bytes [0..13]
 *
 * CRC is CRC-8/SMBUS: poly 0x07, init 0x00, no reflection, no final
 * XOR, check value 0xF4 over "123456789".  Over a codeword of up to 120
 * bits it detects every error of 1, 2, or 3 flipped bits; a 15-byte
 * packet is exactly 120 bits, so a longer packet needs a wider CRC.
 */

#ifndef PACKET_H
#define PACKET_H

#include <stddef.h>
#include <stdint.h>

/* ---------------------------------------------------------------------
 * Constants
 * ------------------------------------------------------------------ */

#define PKT_SYNC      0xA5
#define PKT_KEY_COUNT 6
#define PKT_SIZE      15

_Static_assert(PKT_SIZE == 1 + 1 + 2 * PKT_KEY_COUNT + 1,
               "PKT_SIZE must equal sync + half id + positions + CRC");
_Static_assert(PKT_SIZE * 8 <= 120,
               "CRC-8/SMBUS guarantees detection of all 1-3 bit errors only up to 15 bytes");

/* ---------------------------------------------------------------------
 * Types
 * ------------------------------------------------------------------ */

enum pkt_half {
    PKT_HALF_LEFT = 0,
    PKT_HALF_RIGHT = 1,
};

/**
 * Decoded packet contents.
 *
 * @half  Which half sent the positions.
 * @pos   Raw key positions in ADC counts, in that half's scan order.
 */
struct pkt_frame {
    enum pkt_half half;
    uint16_t pos[PKT_KEY_COUNT];
};

enum pkt_status {
    PKT_OK,
    PKT_ERR_SYNC,
    PKT_ERR_HALF,
    PKT_ERR_CRC,
};

enum pkt_parse_result {
    PKT_PARSE_PENDING,
    PKT_PARSE_FRAME,
    PKT_PARSE_REJECTED,
};

/**
 * Byte-stream parser state.  Initialise with pkt_parser_init().
 *
 * @buf  Bytes of the candidate packet, starting at a sync byte.
 * @len  Number of bytes held in buf.
 */
struct pkt_parser {
    uint8_t buf[PKT_SIZE];
    uint8_t len;
};

/* ---------------------------------------------------------------------
 * API
 * ------------------------------------------------------------------ */

/**
 * CRC-8/SMBUS of len bytes.
 */
uint8_t pkt_crc8(const uint8_t *data, size_t len);

/**
 * Write frame into out as a complete packet, CRC included.
 */
void pkt_pack(const struct pkt_frame *frame, uint8_t out[PKT_SIZE]);

/**
 * Validate one packet and decode it into *frame.
 *
 * *frame is written only when the result is PKT_OK.
 */
enum pkt_status pkt_unpack(const uint8_t in[PKT_SIZE], struct pkt_frame *frame);

/**
 * Reset the parser to wait for a sync byte.
 */
void pkt_parser_init(struct pkt_parser *parser);

/**
 * Feed one received byte.
 *
 * Returns PKT_PARSE_FRAME and writes *frame when the byte completes a
 * valid packet.  Returns PKT_PARSE_REJECTED when it completes a candidate
 * that fails validation; the parser then resumes from the next sync byte
 * inside the rejected candidate. A false candidate that happens to pass
 * validation (about 1 in 256 for random bytes) is reported as a frame and
 * can consume the real packet that follows it.
 * Bytes before a sync byte are skipped and return PKT_PARSE_PENDING.
 */
enum pkt_parse_result pkt_parser_feed(struct pkt_parser *parser, uint8_t byte,
                                      struct pkt_frame *frame);

#endif /* PACKET_H */
