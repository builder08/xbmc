/*
 * RTMP packet utilities
 * Copyright (c) 2009 Kostya Shishkov
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVFORMAT_RTMPPKT_H
#define AVFORMAT_RTMPPKT_H

#include "avformat.h"

/** maximum possible number of different RTMP channels */
#define RTMP_CHANNELS 65599

/**
 * channels used to for RTMP packets with different purposes (i.e. data, network
 * control, remote procedure calls, etc.)
 */
enum RTMPChannel {
    RTMP_NETWORK_CHANNEL = 2,   ///< channel for network-related messages (bandwidth report, ping, etc)
    RTMP_SYSTEM_CHANNEL,        ///< channel for sending server control messages
    RTMP_SOURCE_CHANNEL,        ///< channel for sending a/v to server
    RTMP_VIDEO_CHANNEL = 8,     ///< channel for video data
    RTMP_AUDIO_CHANNEL,         ///< channel for audio data
};

/**
 * known RTMP packet types
 */
typedef enum RTMPPacketType {
    RTMP_PT_CHUNK_SIZE   =  1,  ///< chunk size change
    RTMP_PT_BYTES_READ   =  3,  ///< number of bytes read
    RTMP_PT_PING,               ///< ping
    RTMP_PT_SERVER_BW,          ///< server bandwidth
    RTMP_PT_CLIENT_BW,          ///< client bandwidth
    RTMP_PT_AUDIO        =  8,  ///< audio packet
    RTMP_PT_VIDEO,              ///< video packet
    RTMP_PT_FLEX_STREAM  = 15,  ///< Flex shared stream
    RTMP_PT_FLEX_OBJECT,        ///< Flex shared object
    RTMP_PT_FLEX_MESSAGE,       ///< Flex shared message
    RTMP_PT_NOTIFY,             ///< some notification
    RTMP_PT_SHARED_OBJ,         ///< shared object
    RTMP_PT_INVOKE,             ///< invoke some stream action
    RTMP_PT_METADATA     = 22,  ///< FLV metadata
} RTMPPacketType;

/**
 * possible RTMP packet header sizes
 */
enum RTMPPacketSize {
    RTMP_PS_TWELVEBYTES = 0, ///< packet has 12-byte header
    RTMP_PS_EIGHTBYTES,      ///< packet has 8-byte header
    RTMP_PS_FOURBYTES,       ///< packet has 4-byte header
    RTMP_PS_ONEBYTE          ///< packet is really a next chunk of a packet
};

/**
 * structure for holding RTMP packets
 */
typedef struct RTMPPacket {
    int            channel_id; ///< RTMP channel ID (nothing to do with audio/video channels though)
    RTMPPacketType type;       ///< packet payload type
    uint32_t       timestamp;  ///< packet full timestamp
    uint32_t       ts_delta;   ///< timestamp increment to the previous one in milliseconds (latter only for media packets)
    uint32_t       extra;      ///< probably an additional channel ID used during streaming data
    uint8_t        *data;      ///< packet payload
    int            data_size;  ///< packet payload size
} RTMPPacket;

/**
 * Creates new RTMP packet with given attributes.
 *
 * @param pkt        packet
 * @param channel_id packet channel ID
 * @param type       packet type
 * @param timestamp  packet timestamp
 * @param size       packet size
 * @return zero on success, negative value otherwise
 */
int ff_rtmp_packet_create(RTMPPacket *pkt, int channel_id, RTMPPacketType type,
                          int timestamp, int size);

/**
 * Frees RTMP packet.
 *
 * @param pkt packet
 */
void ff_rtmp_packet_destroy(RTMPPacket *pkt);

/**
 * Reads RTMP packet sent by the server.
 *
 * @param h          reader context
 * @param p          packet
 * @param chunk_size current chunk size
 * @param prev_pkt   previously read packet headers for all channels
 *                   (may be needed for restoring incomplete packet header)
 * @return zero on success, negative value otherwise
 */
int ff_rtmp_packet_read(URLContext *h, RTMPPacket *p,
                        int chunk_size, RTMPPacket *prev_pkt);

/**
 * Sends RTMP packet to the server.
 *
 * @param h          reader context
 * @param p          packet to send
 * @param chunk_size current chunk size
 * @param prev_pkt   previously sent packet headers for all channels
 *                   (may be used for packet header compressing)
 * @return zero on success, negative value otherwise
 */
int ff_rtmp_packet_write(URLContext *h, RTMPPacket *p,
                         int chunk_size, RTMPPacket *prev_pkt);

/**
 * Prints information and contents of RTMP packet.
 *
 * @param h          output context
 * @param p          packet to dump
 */
void ff_rtmp_packet_dump(void *ctx, RTMPPacket *p);

/**
 * @defgroup amffuncs functions used to work with AMF format (which is also used in .flv)
 * @see amf_* funcs in libavformat/flvdec.c
 * @{
 */

/**
 * Calculates number of bytes taken by first AMF entry in data.
 *
 * @param data input data
 * @param data_end input buffer end
 * @return number of bytes used by first AMF entry
 */
int ff_amf_tag_size(const uint8_t *data, const uint8_t *data_end);

/**
 * Retrieves value of given AMF object field in string form.
 *
 * @param data     AMF object data
 * @param data_end input buffer end
 * @param name     name of field to retrieve
 * @param dst      buffer for storing result
 * @param dst_size output buffer size
 * @return 0 if search and retrieval succeeded, negative value otherwise
 */
int ff_amf_get_field_value(const uint8_t *data, const uint8_t *data_end,
                           const uint8_t *name, uint8_t *dst, int dst_size);

/**
 * Writes boolean value in AMF format to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 * @param val value to write
 */
void ff_amf_write_bool(uint8_t **dst, int val);

/**
 * Writes number in AMF format to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 * @param num value to write
 */
void ff_amf_write_number(uint8_t **dst, double num);

/**
 * Writes string in AMF format to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 * @param str string to write
 */
void ff_amf_write_string(uint8_t **dst, const char *str);

/**
 * Writes AMF NULL value to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 */
void ff_amf_write_null(uint8_t **dst);

/**
 * Writes marker for AMF object to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 */
void ff_amf_write_object_start(uint8_t **dst);

/**
 * Writes string used as field name in AMF object to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 * @param str string to write
 */
void ff_amf_write_field_name(uint8_t **dst, const char *str);

/**
 * Writes marker for end of AMF object to buffer.
 *
 * @param dst pointer to the input buffer (will be modified)
 */
void ff_amf_write_object_end(uint8_t **dst);

/** @} */ // AMF funcs

#endif /* AVFORMAT_RTMPPKT_H */
