/*
 * RTP demuxer definitions
 * Copyright (c) 2002 Fabrice Bellard
 * Copyright (c) 2006 Ryan Martell <rdm4@martellventures.com>
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
#ifndef AVFORMAT_RTPDEC_H
#define AVFORMAT_RTPDEC_H

#include "libavcodec/avcodec.h"
#include "avformat.h"
#include "rtp.h"
#include "url.h"

typedef struct PayloadContext PayloadContext;
typedef struct RTPDynamicProtocolHandler_s RTPDynamicProtocolHandler;

#define RTP_MIN_PACKET_LENGTH 12
#define RTP_MAX_PACKET_LENGTH 1500 /* XXX: suppress this define */

#define RTP_REORDER_QUEUE_DEFAULT_SIZE 10

#define RTP_NOTS_VALUE ((uint32_t)-1)

typedef struct RTPDemuxContext RTPDemuxContext;
RTPDemuxContext *ff_rtp_parse_open(AVFormatContext *s1, AVStream *st, URLContext *rtpc, int payload_type, int queue_size);
void ff_rtp_parse_set_dynamic_protocol(RTPDemuxContext *s, PayloadContext *ctx,
                                       RTPDynamicProtocolHandler *handler);
int ff_rtp_parse_packet(RTPDemuxContext *s, AVPacket *pkt,
                        uint8_t **buf, int len);
void ff_rtp_parse_close(RTPDemuxContext *s);
int64_t ff_rtp_queued_packet_time(RTPDemuxContext *s);
void ff_rtp_reset_packet_queue(RTPDemuxContext *s);
int ff_rtp_get_local_rtp_port(URLContext *h);
int ff_rtp_get_local_rtcp_port(URLContext *h);

int ff_rtp_set_remote_url(URLContext *h, const char *uri);

/**
 * Send a dummy packet on both port pairs to set up the connection
 * state in potential NAT routers, so that we're able to receive
 * packets.
 *
 * Note, this only works if the NAT router doesn't remap ports. This
 * isn't a standardized procedure, but it works in many cases in practice.
 *
 * The same routine is used with RDT too, even if RDT doesn't use normal
 * RTP packets otherwise.
 */
void ff_rtp_send_punch_packets(URLContext* rtp_handle);

/**
 * some rtp servers assume client is dead if they don't hear from them...
 * so we send a Receiver Report to the provided ByteIO context
 * (we don't have access to the rtcp handle from here)
 */
int ff_rtp_check_and_send_back_rr(RTPDemuxContext *s, int count);

/**
 * Get the file handle for the RTCP socket.
 */
int ff_rtp_get_rtcp_file_handle(URLContext *h);

// these statistics are used for rtcp receiver reports...
typedef struct {
    uint16_t max_seq;           ///< highest sequence number seen
    uint32_t cycles;            ///< shifted count of sequence number cycles
    uint32_t base_seq;          ///< base sequence number
    uint32_t bad_seq;           ///< last bad sequence number + 1
    int probation;              ///< sequence packets till source is valid
    int received;               ///< packets received
    int expected_prior;         ///< packets expected in last interval
    int received_prior;         ///< packets received in last interval
    uint32_t transit;           ///< relative transit time for previous packet
    uint32_t jitter;            ///< estimated jitter.
} RTPStatistics;

#define RTP_FLAG_KEY    0x1 ///< RTP packet contains a keyframe
#define RTP_FLAG_MARKER 0x2 ///< RTP marker bit was set for this packet
/**
 * Packet parsing for "private" payloads in the RTP specs.
 *
 * @param ctx RTSP demuxer context
 * @param s stream context
 * @param st stream that this packet belongs to
 * @param pkt packet in which to write the parsed data
 * @param timestamp pointer in which to write the timestamp of this RTP packet
 * @param buf pointer to raw RTP packet data
 * @param len length of buf
 * @param flags flags from the RTP packet header (RTP_FLAG_*)
 */
typedef int (*DynamicPayloadPacketHandlerProc) (AVFormatContext *ctx,
                                                PayloadContext *s,
                                                AVStream *st,
                                                AVPacket * pkt,
                                                uint32_t *timestamp,
                                                const uint8_t * buf,
                                                int len, int flags);

struct RTPDynamicProtocolHandler_s {
    // fields from AVRtpDynamicPayloadType_s
    const char enc_name[50];    /* XXX: still why 50 ? ;-) */
    enum AVMediaType codec_type;
    enum CodecID codec_id;
    int static_payload_id; /* 0 means no payload id is set. 0 is a valid
                            * payload ID (PCMU), too, but that format doesn't
                            * require any custom depacketization code. */

    // may be null
    int (*init)(AVFormatContext *s, int st_index, PayloadContext *priv_data); ///< Initialize dynamic protocol handler, called after the full rtpmap line is parsed
    int (*parse_sdp_a_line) (AVFormatContext *s,
                             int st_index,
                             PayloadContext *priv_data,
                             const char *line); ///< Parse the a= line from the sdp field
    PayloadContext *(*alloc) (void); ///< allocate any data needed by the rtp parsing for this dynamic data.
    void (*free)(PayloadContext *protocol_data); ///< free any data needed by the rtp parsing for this dynamic data.
    DynamicPayloadPacketHandlerProc parse_packet; ///< parse handler for this dynamic packet.

    struct RTPDynamicProtocolHandler_s *next;
};

typedef struct RTPPacket {
    uint16_t seq;
    uint8_t *buf;
    int len;
    int64_t recvtime;
    struct RTPPacket *next;
} RTPPacket;

// moved out of rtp.c, because the h264 decoder needs to know about this structure..
struct RTPDemuxContext {
    AVFormatContext *ic;
    AVStream *st;
    int payload_type;
    uint32_t ssrc;
    uint16_t seq;
    uint32_t timestamp;
    uint32_t base_timestamp;
    uint32_t cur_timestamp;
    int64_t  unwrapped_timestamp;
    int64_t  range_start_offset;
    int max_payload_size;
    struct MpegTSContext *ts;   /* only used for MP2T payloads */
    int read_buf_index;
    int read_buf_size;
    /* used to send back RTCP RR */
    URLContext *rtp_ctx;
    char hostname[256];

    RTPStatistics statistics; ///< Statistics for this stream (used by RTCP receiver reports)

    /** Fields for packet reordering @{ */
    int prev_ret;     ///< The return value of the actual parsing of the previous packet
    RTPPacket* queue; ///< A sorted queue of buffered packets not yet returned
    int queue_len;    ///< The number of packets in queue
    int queue_size;   ///< The size of queue, or 0 if reordering is disabled
    /*@}*/

    /* rtcp sender statistics receive */
    int64_t last_rtcp_ntp_time;    // TODO: move into statistics
    int64_t first_rtcp_ntp_time;   // TODO: move into statistics
    uint32_t last_rtcp_timestamp;  // TODO: move into statistics
    int64_t rtcp_ts_offset;

    /* rtcp sender statistics */
    unsigned int packet_count;     // TODO: move into statistics (outgoing)
    unsigned int octet_count;      // TODO: move into statistics (outgoing)
    unsigned int last_octet_count; // TODO: move into statistics (outgoing)
    int first_packet;
    /* buffer for output */
    uint8_t buf[RTP_MAX_PACKET_LENGTH];
    uint8_t *buf_ptr;

    /* dynamic payload stuff */
    DynamicPayloadPacketHandlerProc parse_packet;     ///< This is also copied from the dynamic protocol handler structure
    PayloadContext *dynamic_protocol_context;        ///< This is a copy from the values setup from the sdp parsing, in rtsp.c don't free me.
    int max_frames_per_packet;
};

void ff_register_dynamic_payload_handler(RTPDynamicProtocolHandler *handler);
RTPDynamicProtocolHandler *ff_rtp_handler_find_by_name(const char *name,
                                                  enum AVMediaType codec_type);
RTPDynamicProtocolHandler *ff_rtp_handler_find_by_id(int id,
                                                enum AVMediaType codec_type);

int ff_rtsp_next_attr_and_value(const char **p, char *attr, int attr_size, char *value, int value_size); ///< from rtsp.c, but used by rtp dynamic protocol handlers.

int ff_parse_fmtp(AVStream *stream, PayloadContext *data, const char *p,
                  int (*parse_fmtp)(AVStream *stream,
                                    PayloadContext *data,
                                    char *attr, char *value));

void av_register_rtp_dynamic_payload_handlers(void);

#endif /* AVFORMAT_RTPDEC_H */
