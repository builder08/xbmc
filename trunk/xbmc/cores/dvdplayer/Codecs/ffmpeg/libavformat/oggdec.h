/**
    Copyright (C) 2005  Michael Ahlberg, Måns Rullgård

    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
**/

#ifndef AVFORMAT_OGGDEC_H
#define AVFORMAT_OGGDEC_H

#include "avformat.h"
#include "metadata.h"

struct ogg_codec {
    const int8_t *magic;
    uint8_t magicsize;
    const int8_t *name;
    /**
     * Attempt to process a packet as a header
     * @return 1 if the packet was a valid header,
     *         0 if the packet was not a header (was a data packet)
     *         -1 if an error occurred or for unsupported stream
     */
    int (*header)(AVFormatContext *, int);
    int (*packet)(AVFormatContext *, int);
    uint64_t (*gptopts)(AVFormatContext *, int, uint64_t);
    /**
     * 1 if granule is the start time of the associated packet.
     * 0 if granule is the end time of the associated packet.
     */
    int granule_is_start;
};

struct ogg_stream {
    uint8_t *buf;
    unsigned int bufsize;
    unsigned int bufpos;
    unsigned int pstart;
    unsigned int psize;
    unsigned int pflags;
    unsigned int pduration;
    uint32_t serial;
    uint32_t seq;
    uint64_t granule;
    int64_t lastpts;
    int flags;
    const struct ogg_codec *codec;
    int header;
    int nsegs, segp;
    uint8_t segments[255];
    int page_end;   ///< current packet is the last one completed in the page
    void *private;
};

struct ogg_state {
    uint64_t pos;
    int curidx;
    struct ogg_state *next;
    int nstreams;
    struct ogg_stream streams[1];
};

struct ogg {
    struct ogg_stream *streams;
    int nstreams;
    int headers;
    int curidx;
    uint64_t size;
    struct ogg_state *state;
};

#define OGG_FLAG_CONT 1
#define OGG_FLAG_BOS  2
#define OGG_FLAG_EOS  4

extern const struct ogg_codec ff_flac_codec;
extern const struct ogg_codec ff_ogm_audio_codec;
extern const struct ogg_codec ff_ogm_old_codec;
extern const struct ogg_codec ff_ogm_text_codec;
extern const struct ogg_codec ff_ogm_video_codec;
extern const struct ogg_codec ff_old_flac_codec;
extern const struct ogg_codec ff_speex_codec;
extern const struct ogg_codec ff_theora_codec;
extern const struct ogg_codec ff_vorbis_codec;

extern const AVMetadataConv ff_vorbiscomment_metadata_conv[];

int vorbis_comment(AVFormatContext *ms, uint8_t *buf, int size);

#endif /* AVFORMAT_OGGDEC_H */
