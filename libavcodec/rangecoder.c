/*
 * Range coder
 * Copyright (c) 2004 Michael Niedermayer <michaelni@gmx.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
/**
 * @file rangecoder.c
 * Range coder.
 * based upon
 *    "Range encoding: an algorithm for removing redundancy from a digitised
 *                     message.
 *     G. N. N. Martin                  Presented in March 1979 to the Video &
 *                                      Data Recording Conference,
 *     IBM UK Scientific Center         held in Southampton July 24-27 1979."
 *
 */

#include <string.h>

#include "avcodec.h"
#include "common.h"
#include "rangecoder.h"


void ff_init_range_encoder(RangeCoder *c, uint8_t *buf, int buf_size){
    c->bytestream_start= 
    c->bytestream= buf;
    c->bytestream_end= buf + buf_size;

    c->low= 0;
    c->range= 0xFF00;
    c->outstanding_count= 0;
    c->outstanding_byte= -1;
}

void ff_init_range_decoder(RangeCoder *c, const uint8_t *buf, int buf_size){
    ff_init_range_encoder(c, buf, buf_size);

    c->low =(*c->bytestream++)<<8;
    c->low+= *c->bytestream++;
}

void ff_build_rac_states(RangeCoder *c, int factor, int max_p){
    const int64_t one= 1LL<<32;
    int64_t p;
    int last_p8, p8, i;

    memset(c->zero_state, 0, sizeof(c->zero_state));
    memset(c-> one_state, 0, sizeof(c-> one_state));

#if 0
    for(i=1; i<256; i++){
        if(c->one_state[i]) 
            continue;
        
        p= (i*one + 128) >> 8;
        last_p8= i;
        for(;;){
            p+= ((one-p)*factor + one/2) >> 32;
            p8= (256*p + one/2) >> 32; //FIXME try without the one
            if(p8 <= last_p8) p8= last_p8+1;
            if(p8 > max_p) p8= max_p;
            if(p8 < last_p8)
                break;
            c->one_state[last_p8]=     p8;
            if(p8 == last_p8)
                break;
            last_p8= p8;
        }
    }
#endif
#if 1
    last_p8= 0;
    p= one/2;
    for(i=0; i<128; i++){
        p8= (256*p + one/2) >> 32; //FIXME try without the one
        if(p8 <= last_p8) p8= last_p8+1;
        if(last_p8 && last_p8<256 && p8<=max_p)
            c->one_state[last_p8]= p8;
        
        p+= ((one-p)*factor + one/2) >> 32;
        last_p8= p8;
    }
#endif
    for(i=256-max_p; i<=max_p; i++){
        if(c->one_state[i]) 
            continue;

        p= (i*one + 128) >> 8;
        p+= ((one-p)*factor + one/2) >> 32;
        p8= (256*p + one/2) >> 32; //FIXME try without the one
        if(p8 <= i) p8= i+1;
        if(p8 > max_p) p8= max_p;
        c->one_state[    i]=     p8;
    }
    
    for(i=0; i<256; i++)
        c->zero_state[i]= 256-c->one_state[256-i];
#if 0
    for(i=0; i<256; i++)
        av_log(NULL, AV_LOG_DEBUG, "%3d %3d\n", i, c->one_state[i]);
#endif
}

/**
 *
 * @return the number of bytes written
 */
int ff_rac_terminate(RangeCoder *c){
    c->range=0xFF;
    c->low +=0xFF;
    renorm_encoder(c);
    c->range=0xFF;
    renorm_encoder(c);

    assert(c->low   == 0);
    assert(c->range >= 0x100);

    return c->bytestream - c->bytestream_start;
}

#if 0 //selftest
#define SIZE 10240
int main(){
    RangeCoder c;
    uint8_t b[9*SIZE];
    uint8_t r[9*SIZE];
    int i;
    uint8_t state[10]= {0};
    
    ff_init_range_encoder(&c, b, SIZE);
    ff_build_rac_states(&c, 0.05*(1LL<<32), 128+64+32+16);
    
    memset(state, 128, sizeof(state));

    for(i=0; i<SIZE; i++){
        r[i]= random()%7;
    }
    
  
    for(i=0; i<SIZE; i++){
START_TIMER
        put_rac(&c, state, r[i]&1);
STOP_TIMER("put_rac")
    }

    ff_put_rac_terminate(&c);
    
    ff_init_range_decoder(&c, b, SIZE);
    
    memset(state, 128, sizeof(state));
    
    for(i=0; i<SIZE; i++){
START_TIMER
        if( (r[i]&1) != get_rac(&c, state) )
            av_log(NULL, AV_LOG_DEBUG, "rac failure at %d\n", i);
STOP_TIMER("get_rac")
    }
    
    return 0;
}

#endif
