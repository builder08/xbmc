/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2003 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
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
 * [xad] FLASH player, by Riven the Mage <riven@ok.ru>
 */

/*
    - discovery -

  file(s) : LA-INTRO.EXE
     type : Lunatic Asylum BBStro
     tune : by Rogue [Logic Design]
   player : by Flash [Logic Design]
*/

#include "flash.h"
#include "debug.h"

const unsigned char CxadflashPlayer::flash_adlib_registers[99] =
{
  0x23, 0x20, 0x43, 0x40, 0x63, 0x60, 0x83, 0x80, 0xC0, 0xE3, 0xE0,
  0x24, 0x21, 0x44, 0x41, 0x64, 0x61, 0x84, 0x81, 0xC1, 0xE4, 0xE1,
  0x25, 0x22, 0x45, 0x42, 0x65, 0x62, 0x85, 0x82, 0xC2, 0xE5, 0xE2,
  0x2B, 0x28, 0x4B, 0x48, 0x6B, 0x68, 0x8B, 0x88, 0xC3, 0xEB, 0xE8,
  0x2C, 0x29, 0x4C, 0x49, 0x6C, 0x69, 0x8C, 0x89, 0xC4, 0xEC, 0xE9,
  0x2D, 0x2A, 0x4D, 0x4A, 0x6D, 0x6A, 0x8D, 0x8A, 0xC5, 0xED, 0xEA,
  0x33, 0x30, 0x53, 0x50, 0x73, 0x70, 0x93, 0x90, 0xC6, 0xF3, 0xF0,
  0x34, 0x31, 0x54, 0x51, 0x74, 0x71, 0x94, 0x91, 0xC7, 0xF4, 0xF1,
  0x35, 0x32, 0x55, 0x52, 0x75, 0x72, 0x95, 0x92, 0xC8, 0xF5, 0xF2
};

const unsigned short CxadflashPlayer::flash_notes_encoded[268] =
{
  0x000,
  0x100, 0x200, 0x300, 0x400, 0x500, 0x600, 0x700, 0x800, 0x900, 0xA00, 0xB00, 0xC00,
  0x101, 0x201, 0x301, 0x401, 0x501, 0x601, 0x701, 0x801, 0x901, 0xA01, 0xB01, 0xC01,
  0x102, 0x202, 0x302, 0x402, 0x502, 0x602, 0x702, 0x802, 0x902, 0xA02, 0xB02, 0xC02,
  0x103, 0x203, 0x303, 0x403, 0x503, 0x603, 0x703, 0x803, 0x903, 0xA03, 0xB03, 0xC03,
  0x104, 0x204, 0x304, 0x404, 0x504, 0x604, 0x704, 0x804, 0x904, 0xA04, 0xB04, 0xC04,
  0x105, 0x205, 0x305, 0x405, 0x505, 0x605, 0x705, 0x805, 0x905, 0xA05, 0xB05, 0xC05,
  0x106, 0x206, 0x306, 0x406, 0x506, 0x606, 0x706, 0x806, 0x906, 0xA06, 0xB06, 0xC06,
  0x107, 0x207, 0x307, 0x407, 0x507, 0x607, 0x707, 0x807, 0x907, 0xA07, 0xB07, 0xC07,
  0x108, 0x208, 0x308, 0x408, 0x508, 0x608, 0x708, 0x808, 0x908, 0xA08, 0xB08, 0xC08,
  0x109, 0x209, 0x309, 0x409, 0x509, 0x609, 0x709, 0x809, 0x909, 0xA09, 0xB09, 0xC09,
  0x10A, 0x20A, 0x30A, 0x40A, 0x50A, 0x60A, 0x70A, 0x80A, 0x90A, 0xA0A, 0xB0A, 0xC0A,
  0x10B, 0x20B, 0x30B, 0x40B, 0x50B, 0x60B, 0x70B, 0x80B, 0x90B, 0xA0B, 0xB0B, 0xC0B,
  0x10C, 0x20C, 0x30C, 0x40C, 0x50C, 0x60C, 0x70C, 0x80C, 0x90C, 0xA0C, 0xB0C, 0xC0C,
  0x10D, 0x20D, 0x30D, 0x40D, 0x50D, 0x60D, 0x70D, 0x80D, 0x90D, 0xA0D, 0xB0D, 0xC0D,
  0x10E, 0x20E, 0x30E, 0x40E, 0x50E, 0x60E, 0x70E, 0x80E, 0x90E, 0xA0E, 0xB0E, 0xC0E,
  0x10F, 0x20F, 0x30F, 0x40F, 0x50F, 0x60F, 0x70F, 0x80F, 0x90F, 0xA0F, 0xB0F, 0xC0F,
  0x110, 0x210, 0x310, 0x410, 0x510, 0x610, 0x710, 0x810, 0x910, 0xA10, 0xB10, 0xC10,
  0x111, 0x211, 0x311, 0x411, 0x511, 0x611, 0x711, 0x811, 0x911, 0xA11, 0xB11, 0xC11,
  0x112, 0x212, 0x312, 0x412, 0x512, 0x612, 0x712, 0x812, 0x912, 0xA12, 0xB12, 0xC12,
  0x113, 0x213, 0x313, 0x413, 0x513, 0x613, 0x713, 0x813, 0x913, 0xA13, 0xB13, 0xC13,
  0x114, 0x214, 0x314, 0x414, 0x514, 0x614, 0x714, 0x814, 0x914, 0xA14, 0xB14, 0xC14,
  0x115, 0x215, 0x315
};

const unsigned short CxadflashPlayer::flash_notes[12] =
{
  0x157, 0x16B, 0x181, 0x198, 0x1B0, 0x1CA, 0x1E5, 0x202, 0x220, 0x241, 0x263, 0x287
};

const unsigned char CxadflashPlayer::flash_default_instrument[8] =
{
  0x00, 0x00, 0x3F, 0x3F, 0xFF, 0xFF, 0xFF, 0xFF
};

CPlayer *CxadflashPlayer::factory(Copl *newopl)
{
  return new CxadflashPlayer(newopl);
}

void CxadflashPlayer::xadplayer_rewind(int subsong)
{
  int i;

  plr.speed = xad.speed;

  flash.order_pos = 0;
  flash.pattern_pos = 0;

  opl_write(0x08, 0x00);
  opl_write(0xBD, 0x00);

  // assign default instrument
  for(i=0; i<9; i++)
  {
    opl_write(0xA0+i, 0x00);
    opl_write(0xB0+i, 0x00);
  }

  // assign instruments
  for(i=0; i<9; i++)
    for(int j=0; j<11; j++)
      opl_write(flash_adlib_registers[i*11+j], tune[i*12+j]);
}

void CxadflashPlayer::xadplayer_update()
{
  unsigned short event_pos = (tune[0x600+flash.order_pos]*1152) + \
                             (flash.pattern_pos*18) + \
                              0x633;

  for (int i=0; i<9; i++)
  {
    unsigned short flash_channel_freq = (adlib[0xB0+i] << 8) + adlib[0xA0+i];

    unsigned char event_b0 = tune[event_pos++];
    unsigned char event_b1 = tune[event_pos++];
#ifdef DEBUG
  AdPlug_LogWrite("channel %02X, event %02X %02X:\n",i+1,event_b0,event_b1);
#endif

    if (event_b0 == 0x80)               // 0.0x80: Set Instrument
    {
      for(int j=0; j<11; j++)
        opl_write(flash_adlib_registers[i*11+j], tune[event_b1*12+j]);
    }
    else
    {
      if (event_b1 == 0x01)
        flash.pattern_pos = 0x3F;       // 1.0x01: Pattern Break

      unsigned char fx = (event_b1 >> 4);
      unsigned char fx_p = (event_b1 & 0x0F);

      switch(fx)
      {
        case 0x0A:                      // 1.0xAy: Set Carrier volume
          opl_write(flash_adlib_registers[11*i+2], fx_p << 2);
          break;
        case 0x0B:                      // 1.0xBy: Set Modulator volume
          opl_write(flash_adlib_registers[11*i+3], fx_p << 2);
          break;
        case 0x0C:                      // 1.0xCy: Set both operators volume
          opl_write(flash_adlib_registers[11*i+2], fx_p << 2);
          opl_write(flash_adlib_registers[11*i+3], fx_p << 2);
          break;
//      case 0x0E:                      // 1.0xEy: ? (increase some value)
        case 0x0F:                      // 1.0xFy: Set Speed
          plr.speed = (fx_p + 1);
          break;
      }

      if (event_b0)
      {
        // mute channel
        opl_write(0xA0+i, adlib[0xA0+i]);
        opl_write(0xB0+i, adlib[0xB0+i] & 0xDF);

        // is note ?
        if (event_b0 != 0x7F)
        {
          unsigned short note_encoded = flash_notes_encoded[event_b0];
          unsigned short freq = flash_notes[(note_encoded >> 8) - 1];

          flash_channel_freq = freq | ((note_encoded & 0xFF) << 10) | 0x2000;

          opl_write(0xA0+i, flash_channel_freq & 0xFF);
          opl_write(0xB0+i, flash_channel_freq >> 8);
        }
      }

      if (fx == 0x01)                   // 1.0x1y: Fine Frequency Slide Up
      {
        flash_channel_freq += (fx_p << 1);

        opl_write(0xA0+i, flash_channel_freq & 0xFF);
        opl_write(0xB0+i, flash_channel_freq >> 8);
      }
      else if (fx == 0x02)              // 1.0x2y: Fine Frequency Slide Down
      {
        flash_channel_freq -= (fx_p << 1);

        opl_write(0xA0+i, flash_channel_freq & 0xFF);
        opl_write(0xB0+i, flash_channel_freq >> 8);
      }
    }
  }

  // next row
  flash.pattern_pos++;

  // end of pattern ?
  if (flash.pattern_pos >= 0x40)
  {
    flash.pattern_pos = 0;

    flash.order_pos++;

    // end of module ?
    if (tune[0x600+flash.order_pos] == 0xFF)
    {
      flash.order_pos = 0;

      plr.looping = 1;
    }
  }
}

float CxadflashPlayer::xadplayer_getrefresh()
{
  return 17.5f;
}

std::string CxadflashPlayer::xadplayer_gettype()
{
  return std::string("xad: flash player");
}

unsigned int CxadflashPlayer::xadplayer_getinstruments()
{
  return 32;
}
