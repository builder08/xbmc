/*
** FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
** Copyright (C) 2003 M. Bakker, Ahead Software AG, http://www.nero.com
**  
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software 
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
**
** Any non-GPL usage of this software or parts of this software is strictly
** forbidden.
**
** Commercial non-GPL licensing of this software is possible.
** For more info contact Ahead Software through Mpeg4AAClicense@nero.com.
**
** $Id: hcb_5.h,v 1.3 2003/09/09 18:12:01 menno Exp $
**/

/* Binary search huffman table HCB_5 */


static hcb_bin_pair hcb5[] = {
    { /*  0 */ 0, {  1,  2 } },
    { /*  1 */ 1, {  0,  0 } }, /* 0 */
    { /*  2 */ 0, {  1,  2 } },
    { /*  3 */ 0, {  2,  3 } },
    { /*  4 */ 0, {  3,  4 } },
    { /*  5 */ 0, {  4,  5 } },
    { /*  6 */ 0, {  5,  6 } },
    { /*  7 */ 0, {  6,  7 } },
    { /*  8 */ 0, {  7,  8 } },
    { /*  9 */ 1, { -1,  0 } }, /* 1000 */
    { /* 10 */ 1, {  1,  0 } }, /* 1001 */
    { /* 11 */ 1, {  0,  1 } }, /* 1010 */
    { /* 12 */ 1, {  0, -1 } }, /* 1011 */
    { /* 13 */ 0, {  4,  5 } },
    { /* 14 */ 0, {  5,  6 } },
    { /* 15 */ 0, {  6,  7 } },
    { /* 16 */ 0, {  7,  8 } },
    { /* 17 */ 1, {  1, -1 } },
    { /* 18 */ 1, { -1,  1 } },
    { /* 19 */ 1, { -1, -1 } },
    { /* 20 */ 1, {  1,  1 } },
    { /* 21 */ 0, {  4,  5 } },
    { /* 22 */ 0, {  5,  6 } },
    { /* 23 */ 0, {  6,  7 } },
    { /* 24 */ 0, {  7,  8 } },
    { /* 25 */ 0, {  8,  9 } },
    { /* 26 */ 0, {  9, 10 } },
    { /* 27 */ 0, { 10, 11 } },
    { /* 28 */ 0, { 11, 12 } },
    { /* 29 */ 0, { 12, 13 } },
    { /* 30 */ 0, { 13, 14 } },
    { /* 31 */ 0, { 14, 15 } },
    { /* 32 */ 0, { 15, 16 } },
    { /* 33 */ 1, { -2,  0 } },
    { /* 34 */ 1, {  0,  2 } },
    { /* 35 */ 1, {  2,  0 } },
    { /* 36 */ 1, {  0, -2 } },
    { /* 37 */ 0, { 12, 13 } },
    { /* 38 */ 0, { 13, 14 } },
    { /* 39 */ 0, { 14, 15 } },
    { /* 40 */ 0, { 15, 16 } },
    { /* 41 */ 0, { 16, 17 } },
    { /* 42 */ 0, { 17, 18 } },
    { /* 43 */ 0, { 18, 19 } },
    { /* 44 */ 0, { 19, 20 } },
    { /* 45 */ 0, { 20, 21 } },
    { /* 46 */ 0, { 21, 22 } },
    { /* 47 */ 0, { 22, 23 } },
    { /* 48 */ 0, { 23, 24 } },
    { /* 49 */ 1, { -2, -1 } },
    { /* 50 */ 1, {  2,  1 } },
    { /* 51 */ 1, { -1, -2 } },
    { /* 52 */ 1, {  1,  2 } },
    { /* 53 */ 1, { -2,  1 } },
    { /* 54 */ 1, {  2, -1 } },
    { /* 55 */ 1, { -1,  2 } },
    { /* 56 */ 1, {  1, -2 } },
    { /* 57 */ 1, { -3,  0 } },
    { /* 58 */ 1, {  3,  0 } },
    { /* 59 */ 1, {  0, -3 } },
    { /* 60 */ 1, {  0,  3 } },
    { /* 61 */ 0, { 12, 13 } },
    { /* 62 */ 0, { 13, 14 } },
    { /* 63 */ 0, { 14, 15 } },
    { /* 64 */ 0, { 15, 16 } },
    { /* 65 */ 0, { 16, 17 } },
    { /* 66 */ 0, { 17, 18 } },
    { /* 67 */ 0, { 18, 19 } },
    { /* 68 */ 0, { 19, 20 } },
    { /* 69 */ 0, { 20, 21 } },
    { /* 70 */ 0, { 21, 22 } },
    { /* 71 */ 0, { 22, 23 } },
    { /* 72 */ 0, { 23, 24 } },
    { /* 73 */ 1, { -3, -1 } },
    { /* 74 */ 1, {  1,  3 } },
    { /* 75 */ 1, {  3,  1 } },
    { /* 76 */ 1, { -1, -3 } },
    { /* 77 */ 1, { -3,  1 } },
    { /* 78 */ 1, {  3, -1 } },
    { /* 79 */ 1, {  1, -3 } },
    { /* 80 */ 1, { -1,  3 } },
    { /* 81 */ 1, { -2,  2 } },
    { /* 82 */ 1, {  2,  2 } },
    { /* 83 */ 1, { -2, -2 } },
    { /* 84 */ 1, {  2, -2 } },
    { /* 85 */ 0, { 12, 13 } },
    { /* 86 */ 0, { 13, 14 } },
    { /* 87 */ 0, { 14, 15 } },
    { /* 88 */ 0, { 15, 16 } },
    { /* 89 */ 0, { 16, 17 } },
    { /* 90 */ 0, { 17, 18 } },
    { /* 91 */ 0, { 18, 19 } },
    { /* 92 */ 0, { 19, 20 } },
    { /* 93 */ 0, { 20, 21 } },
    { /* 94 */ 0, { 21, 22 } },
    { /* 95 */ 0, { 22, 23 } },
    { /* 96 */ 0, { 23, 24 } },
    { /* 97 */ 1, { -3, -2 } },
    { /* 98 */ 1, {  3, -2 } },
    { /* 99 */ 1, { -2,  3 } },
    { /* 00 */ 1, {  2, -3 } },
    { /* 01 */ 1, {  3,  2 } },
    { /* 02 */ 1, {  2,  3 } },
    { /* 03 */ 1, { -3,  2 } },
    { /* 04 */ 1, { -2, -3 } },
    { /* 05 */ 1, {  0, -4 } },
    { /* 06 */ 1, { -4,  0 } },
    { /* 07 */ 1, {  4,  1 } },
    { /* 08 */ 1, {  4,  0 } },
    { /* 09 */ 0, { 12, 13 } },
    { /* 10 */ 0, { 13, 14 } },
    { /* 11 */ 0, { 14, 15 } },
    { /* 12 */ 0, { 15, 16 } },
    { /* 13 */ 0, { 16, 17 } },
    { /* 14 */ 0, { 17, 18 } },
    { /* 15 */ 0, { 18, 19 } },
    { /* 16 */ 0, { 19, 20 } },
    { /* 17 */ 0, { 20, 21 } },
    { /* 18 */ 0, { 21, 22 } },
    { /* 19 */ 0, { 22, 23 } },
    { /* 20 */ 0, { 23, 24 } },
    { /* 21 */ 1, { -4, -1 } },
    { /* 22 */ 1, {  0,  4 } },
    { /* 23 */ 1, {  4, -1 } },
    { /* 24 */ 1, { -1, -4 } },
    { /* 25 */ 1, {  1,  4 } },
    { /* 26 */ 1, { -1,  4 } },
    { /* 27 */ 1, { -4,  1 } },
    { /* 28 */ 1, {  1, -4 } },
    { /* 29 */ 1, {  3, -3 } },
    { /* 30 */ 1, { -3, -3 } },
    { /* 31 */ 1, { -3,  3 } },
    { /* 32 */ 1, { -2,  4 } },
    { /* 33 */ 1, { -4, -2 } },
    { /* 34 */ 1, {  4,  2 } },
    { /* 35 */ 1, {  2, -4 } },
    { /* 36 */ 1, {  2,  4 } },
    { /* 37 */ 1, {  3,  3 } },
    { /* 38 */ 1, { -4,  2 } },
    { /* 39 */ 0, { 6, 7 } },
    { /* 40 */ 0, { 7, 8 } },
    { /* 41 */ 0, { 8, 9 } },
    { /* 42 */ 0, { 9, 10 } },
    { /* 43 */ 0, { 10, 11 } },
    { /* 44 */ 0, { 11, 12 } },
    { /* 45 */ 1, { -2, -4 } },
    { /* 46 */ 1, {  4, -2 } },
    { /* 47 */ 1, {  3, -4 } },
    { /* 48 */ 1, { -4, -3 } },
    { /* 49 */ 1, { -4,  3 } },
    { /* 50 */ 1, {  3,  4 } },
    { /* 51 */ 1, { -3,  4 } },
    { /* 52 */ 1, {  4,  3 } },
    { /* 53 */ 1, {  4, -3 } },
    { /* 54 */ 1, { -3, -4 } },
    { /* 55 */ 0, { 2, 3 } },
    { /* 56 */ 0, { 3, 4 } },
    { /* 57 */ 1, {  4, -4 } },
    { /* 58 */ 1, { -4,  4 } },
    { /* 59 */ 1, {  4,  4 } },
    { /* 60 */ 1, { -4, -4 } }
};
