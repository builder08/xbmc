/* tftables.cpp                                
 *
 * Copyright (C) 2003 Sawtooth Consulting Ltd.
 *
 * This file is part of yaSSL, an SSL implementation written by Todd A Ouska
 * (todd at yassl.com, see www.yassl.com).
 *
 * yaSSL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * There are special exceptions to the terms and conditions of the GPL as it
 * is applied to yaSSL. View the full text of the exception in the file
 * FLOSS-EXCEPTIONS in the directory of this software distribution.
 *
 * yaSSL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/* based on Wei Dai's tftables.cpp from CryptoPP */

#include "runtime.hpp"
#include "twofish.hpp"


namespace TaoCrypt {


const byte Twofish::q_[2][256] = {
{
   0xA9, 0x67, 0xB3, 0xE8, 0x04, 0xFD, 0xA3, 0x76, 0x9A, 0x92, 0x80, 0x78,
   0xE4, 0xDD, 0xD1, 0x38, 0x0D, 0xC6, 0x35, 0x98, 0x18, 0xF7, 0xEC, 0x6C,
   0x43, 0x75, 0x37, 0x26, 0xFA, 0x13, 0x94, 0x48, 0xF2, 0xD0, 0x8B, 0x30,
   0x84, 0x54, 0xDF, 0x23, 0x19, 0x5B, 0x3D, 0x59, 0xF3, 0xAE, 0xA2, 0x82,
   0x63, 0x01, 0x83, 0x2E, 0xD9, 0x51, 0x9B, 0x7C, 0xA6, 0xEB, 0xA5, 0xBE,
   0x16, 0x0C, 0xE3, 0x61, 0xC0, 0x8C, 0x3A, 0xF5, 0x73, 0x2C, 0x25, 0x0B,
   0xBB, 0x4E, 0x89, 0x6B, 0x53, 0x6A, 0xB4, 0xF1, 0xE1, 0xE6, 0xBD, 0x45,
   0xE2, 0xF4, 0xB6, 0x66, 0xCC, 0x95, 0x03, 0x56, 0xD4, 0x1C, 0x1E, 0xD7,
   0xFB, 0xC3, 0x8E, 0xB5, 0xE9, 0xCF, 0xBF, 0xBA, 0xEA, 0x77, 0x39, 0xAF,
   0x33, 0xC9, 0x62, 0x71, 0x81, 0x79, 0x09, 0xAD, 0x24, 0xCD, 0xF9, 0xD8,
   0xE5, 0xC5, 0xB9, 0x4D, 0x44, 0x08, 0x86, 0xE7, 0xA1, 0x1D, 0xAA, 0xED,
   0x06, 0x70, 0xB2, 0xD2, 0x41, 0x7B, 0xA0, 0x11, 0x31, 0xC2, 0x27, 0x90,
   0x20, 0xF6, 0x60, 0xFF, 0x96, 0x5C, 0xB1, 0xAB, 0x9E, 0x9C, 0x52, 0x1B,
   0x5F, 0x93, 0x0A, 0xEF, 0x91, 0x85, 0x49, 0xEE, 0x2D, 0x4F, 0x8F, 0x3B,
   0x47, 0x87, 0x6D, 0x46, 0xD6, 0x3E, 0x69, 0x64, 0x2A, 0xCE, 0xCB, 0x2F,
   0xFC, 0x97, 0x05, 0x7A, 0xAC, 0x7F, 0xD5, 0x1A, 0x4B, 0x0E, 0xA7, 0x5A,
   0x28, 0x14, 0x3F, 0x29, 0x88, 0x3C, 0x4C, 0x02, 0xB8, 0xDA, 0xB0, 0x17,
   0x55, 0x1F, 0x8A, 0x7D, 0x57, 0xC7, 0x8D, 0x74, 0xB7, 0xC4, 0x9F, 0x72,
   0x7E, 0x15, 0x22, 0x12, 0x58, 0x07, 0x99, 0x34, 0x6E, 0x50, 0xDE, 0x68,
   0x65, 0xBC, 0xDB, 0xF8, 0xC8, 0xA8, 0x2B, 0x40, 0xDC, 0xFE, 0x32, 0xA4,
   0xCA, 0x10, 0x21, 0xF0, 0xD3, 0x5D, 0x0F, 0x00, 0x6F, 0x9D, 0x36, 0x42,
   0x4A, 0x5E, 0xC1, 0xE0
},
{
   0x75, 0xF3, 0xC6, 0xF4, 0xDB, 0x7B, 0xFB, 0xC8, 0x4A, 0xD3, 0xE6, 0x6B,
   0x45, 0x7D, 0xE8, 0x4B, 0xD6, 0x32, 0xD8, 0xFD, 0x37, 0x71, 0xF1, 0xE1,
   0x30, 0x0F, 0xF8, 0x1B, 0x87, 0xFA, 0x06, 0x3F, 0x5E, 0xBA, 0xAE, 0x5B,
   0x8A, 0x00, 0xBC, 0x9D, 0x6D, 0xC1, 0xB1, 0x0E, 0x80, 0x5D, 0xD2, 0xD5,
   0xA0, 0x84, 0x07, 0x14, 0xB5, 0x90, 0x2C, 0xA3, 0xB2, 0x73, 0x4C, 0x54,
   0x92, 0x74, 0x36, 0x51, 0x38, 0xB0, 0xBD, 0x5A, 0xFC, 0x60, 0x62, 0x96,
   0x6C, 0x42, 0xF7, 0x10, 0x7C, 0x28, 0x27, 0x8C, 0x13, 0x95, 0x9C, 0xC7,
   0x24, 0x46, 0x3B, 0x70, 0xCA, 0xE3, 0x85, 0xCB, 0x11, 0xD0, 0x93, 0xB8,
   0xA6, 0x83, 0x20, 0xFF, 0x9F, 0x77, 0xC3, 0xCC, 0x03, 0x6F, 0x08, 0xBF,
   0x40, 0xE7, 0x2B, 0xE2, 0x79, 0x0C, 0xAA, 0x82, 0x41, 0x3A, 0xEA, 0xB9,
   0xE4, 0x9A, 0xA4, 0x97, 0x7E, 0xDA, 0x7A, 0x17, 0x66, 0x94, 0xA1, 0x1D,
   0x3D, 0xF0, 0xDE, 0xB3, 0x0B, 0x72, 0xA7, 0x1C, 0xEF, 0xD1, 0x53, 0x3E,
   0x8F, 0x33, 0x26, 0x5F, 0xEC, 0x76, 0x2A, 0x49, 0x81, 0x88, 0xEE, 0x21,
   0xC4, 0x1A, 0xEB, 0xD9, 0xC5, 0x39, 0x99, 0xCD, 0xAD, 0x31, 0x8B, 0x01,
   0x18, 0x23, 0xDD, 0x1F, 0x4E, 0x2D, 0xF9, 0x48, 0x4F, 0xF2, 0x65, 0x8E,
   0x78, 0x5C, 0x58, 0x19, 0x8D, 0xE5, 0x98, 0x57, 0x67, 0x7F, 0x05, 0x64,
   0xAF, 0x63, 0xB6, 0xFE, 0xF5, 0xB7, 0x3C, 0xA5, 0xCE, 0xE9, 0x68, 0x44,
   0xE0, 0x4D, 0x43, 0x69, 0x29, 0x2E, 0xAC, 0x15, 0x59, 0xA8, 0x0A, 0x9E,
   0x6E, 0x47, 0xDF, 0x34, 0x35, 0x6A, 0xCF, 0xDC, 0x22, 0xC9, 0xC0, 0x9B,
   0x89, 0xD4, 0xED, 0xAB, 0x12, 0xA2, 0x0D, 0x52, 0xBB, 0x02, 0x2F, 0xA9,
   0xD7, 0x61, 0x1E, 0xB4, 0x50, 0x04, 0xF6, 0xC2, 0x16, 0x25, 0x86, 0x56,
   0x55, 0x09, 0xBE, 0x91
}
};


const word32 Twofish::mds_[4][256] = {
    {
	0xbcbc3275, 0xecec21f3, 0x202043c6, 0xb3b3c9f4, 
	0xdada03db, 0x02028b7b, 0xe2e22bfb, 0x9e9efac8, 
	0xc9c9ec4a, 0xd4d409d3, 0x18186be6, 0x1e1e9f6b, 
	0x98980e45, 0xb2b2387d, 0xa6a6d2e8, 0x2626b74b, 
	0x3c3c57d6, 0x93938a32, 0x8282eed8, 0x525298fd, 
	0x7b7bd437, 0xbbbb3771, 0x5b5b97f1, 0x474783e1, 
	0x24243c30, 0x5151e20f, 0xbabac6f8, 0x4a4af31b, 
	0xbfbf4887, 0x0d0d70fa, 0xb0b0b306, 0x7575de3f, 
	0xd2d2fd5e, 0x7d7d20ba, 0x666631ae, 0x3a3aa35b, 
	0x59591c8a, 0x00000000, 0xcdcd93bc, 0x1a1ae09d, 
	0xaeae2c6d, 0x7f7fabc1, 0x2b2bc7b1, 0xbebeb90e, 
	0xe0e0a080, 0x8a8a105d, 0x3b3b52d2, 0x6464bad5, 
	0xd8d888a0, 0xe7e7a584, 0x5f5fe807, 0x1b1b1114, 
	0x2c2cc2b5, 0xfcfcb490, 0x3131272c, 0x808065a3, 
	0x73732ab2, 0x0c0c8173, 0x79795f4c, 0x6b6b4154, 
	0x4b4b0292, 0x53536974, 0x94948f36, 0x83831f51, 
	0x2a2a3638, 0xc4c49cb0, 0x2222c8bd, 0xd5d5f85a, 
	0xbdbdc3fc, 0x48487860, 0xffffce62, 0x4c4c0796, 
	0x4141776c, 0xc7c7e642, 0xebeb24f7, 0x1c1c1410, 
	0x5d5d637c, 0x36362228, 0x6767c027, 0xe9e9af8c, 
	0x4444f913, 0x1414ea95, 0xf5f5bb9c, 0xcfcf18c7, 
	0x3f3f2d24, 0xc0c0e346, 0x7272db3b, 0x54546c70, 
	0x29294cca, 0xf0f035e3, 0x0808fe85, 0xc6c617cb, 
	0xf3f34f11, 0x8c8ce4d0, 0xa4a45993, 0xcaca96b8, 
	0x68683ba6, 0xb8b84d83, 0x38382820, 0xe5e52eff, 
	0xadad569f, 0x0b0b8477, 0xc8c81dc3, 0x9999ffcc, 
	0x5858ed03, 0x19199a6f, 0x0e0e0a08, 0x95957ebf, 
	0x70705040, 0xf7f730e7, 0x6e6ecf2b, 0x1f1f6ee2, 
	0xb5b53d79, 0x09090f0c, 0x616134aa, 0x57571682, 
	0x9f9f0b41, 0x9d9d803a, 0x111164ea, 0x2525cdb9, 
	0xafafdde4, 0x4545089a, 0xdfdf8da4, 0xa3a35c97, 
	0xeaead57e, 0x353558da, 0xededd07a, 0x4343fc17, 
	0xf8f8cb66, 0xfbfbb194, 0x3737d3a1, 0xfafa401d, 
	0xc2c2683d, 0xb4b4ccf0, 0x32325dde, 0x9c9c71b3, 
	0x5656e70b, 0xe3e3da72, 0x878760a7, 0x15151b1c, 
	0xf9f93aef, 0x6363bfd1, 0x3434a953, 0x9a9a853e, 
	0xb1b1428f, 0x7c7cd133, 0x88889b26, 0x3d3da65f, 
	0xa1a1d7ec, 0xe4e4df76, 0x8181942a, 0x91910149, 
	0x0f0ffb81, 0xeeeeaa88, 0x161661ee, 0xd7d77321, 
	0x9797f5c4, 0xa5a5a81a, 0xfefe3feb, 0x6d6db5d9, 
	0x7878aec5, 0xc5c56d39, 0x1d1de599, 0x7676a4cd, 
	0x3e3edcad, 0xcbcb6731, 0xb6b6478b, 0xefef5b01, 
	0x12121e18, 0x6060c523, 0x6a6ab0dd, 0x4d4df61f, 
	0xcecee94e, 0xdede7c2d, 0x55559df9, 0x7e7e5a48, 
	0x2121b24f, 0x03037af2, 0xa0a02665, 0x5e5e198e, 
	0x5a5a6678, 0x65654b5c, 0x62624e58, 0xfdfd4519, 
	0x0606f48d, 0x404086e5, 0xf2f2be98, 0x3333ac57, 
	0x17179067, 0x05058e7f, 0xe8e85e05, 0x4f4f7d64, 
	0x89896aaf, 0x10109563, 0x74742fb6, 0x0a0a75fe, 
	0x5c5c92f5, 0x9b9b74b7, 0x2d2d333c, 0x3030d6a5, 
	0x2e2e49ce, 0x494989e9, 0x46467268, 0x77775544, 
	0xa8a8d8e0, 0x9696044d, 0x2828bd43, 0xa9a92969, 
	0xd9d97929, 0x8686912e, 0xd1d187ac, 0xf4f44a15, 
	0x8d8d1559, 0xd6d682a8, 0xb9b9bc0a, 0x42420d9e, 
	0xf6f6c16e, 0x2f2fb847, 0xdddd06df, 0x23233934, 
	0xcccc6235, 0xf1f1c46a, 0xc1c112cf, 0x8585ebdc, 
	0x8f8f9e22, 0x7171a1c9, 0x9090f0c0, 0xaaaa539b, 
	0x0101f189, 0x8b8be1d4, 0x4e4e8ced, 0x8e8e6fab, 
	0xababa212, 0x6f6f3ea2, 0xe6e6540d, 0xdbdbf252, 
	0x92927bbb, 0xb7b7b602, 0x6969ca2f, 0x3939d9a9, 
	0xd3d30cd7, 0xa7a72361, 0xa2a2ad1e, 0xc3c399b4, 
	0x6c6c4450, 0x07070504, 0x04047ff6, 0x272746c2, 
	0xacaca716, 0xd0d07625, 0x50501386, 0xdcdcf756, 
	0x84841a55, 0xe1e15109, 0x7a7a25be, 0x1313ef91
    },
    {
	0xa9d93939, 0x67901717, 0xb3719c9c, 0xe8d2a6a6, 
	0x04050707, 0xfd985252, 0xa3658080, 0x76dfe4e4, 
	0x9a084545, 0x92024b4b, 0x80a0e0e0, 0x78665a5a, 
	0xe4ddafaf, 0xddb06a6a, 0xd1bf6363, 0x38362a2a, 
	0x0d54e6e6, 0xc6432020, 0x3562cccc, 0x98bef2f2, 
	0x181e1212, 0xf724ebeb, 0xecd7a1a1, 0x6c774141, 
	0x43bd2828, 0x7532bcbc, 0x37d47b7b, 0x269b8888, 
	0xfa700d0d, 0x13f94444, 0x94b1fbfb, 0x485a7e7e, 
	0xf27a0303, 0xd0e48c8c, 0x8b47b6b6, 0x303c2424, 
	0x84a5e7e7, 0x54416b6b, 0xdf06dddd, 0x23c56060, 
	0x1945fdfd, 0x5ba33a3a, 0x3d68c2c2, 0x59158d8d, 
	0xf321ecec, 0xae316666, 0xa23e6f6f, 0x82165757, 
	0x63951010, 0x015befef, 0x834db8b8, 0x2e918686, 
	0xd9b56d6d, 0x511f8383, 0x9b53aaaa, 0x7c635d5d, 
	0xa63b6868, 0xeb3ffefe, 0xa5d63030, 0xbe257a7a, 
	0x16a7acac, 0x0c0f0909, 0xe335f0f0, 0x6123a7a7, 
	0xc0f09090, 0x8cafe9e9, 0x3a809d9d, 0xf5925c5c, 
	0x73810c0c, 0x2c273131, 0x2576d0d0, 0x0be75656, 
	0xbb7b9292, 0x4ee9cece, 0x89f10101, 0x6b9f1e1e, 
	0x53a93434, 0x6ac4f1f1, 0xb499c3c3, 0xf1975b5b, 
	0xe1834747, 0xe66b1818, 0xbdc82222, 0x450e9898, 
	0xe26e1f1f, 0xf4c9b3b3, 0xb62f7474, 0x66cbf8f8, 
	0xccff9999, 0x95ea1414, 0x03ed5858, 0x56f7dcdc, 
	0xd4e18b8b, 0x1c1b1515, 0x1eada2a2, 0xd70cd3d3, 
	0xfb2be2e2, 0xc31dc8c8, 0x8e195e5e, 0xb5c22c2c, 
	0xe9894949, 0xcf12c1c1, 0xbf7e9595, 0xba207d7d, 
	0xea641111, 0x77840b0b, 0x396dc5c5, 0xaf6a8989, 
	0x33d17c7c, 0xc9a17171, 0x62ceffff, 0x7137bbbb, 
	0x81fb0f0f, 0x793db5b5, 0x0951e1e1, 0xaddc3e3e, 
	0x242d3f3f, 0xcda47676, 0xf99d5555, 0xd8ee8282, 
	0xe5864040, 0xc5ae7878, 0xb9cd2525, 0x4d049696, 
	0x44557777, 0x080a0e0e, 0x86135050, 0xe730f7f7, 
	0xa1d33737, 0x1d40fafa, 0xaa346161, 0xed8c4e4e, 
	0x06b3b0b0, 0x706c5454, 0xb22a7373, 0xd2523b3b, 
	0x410b9f9f, 0x7b8b0202, 0xa088d8d8, 0x114ff3f3, 
	0x3167cbcb, 0xc2462727, 0x27c06767, 0x90b4fcfc, 
	0x20283838, 0xf67f0404, 0x60784848, 0xff2ee5e5, 
	0x96074c4c, 0x5c4b6565, 0xb1c72b2b, 0xab6f8e8e, 
	0x9e0d4242, 0x9cbbf5f5, 0x52f2dbdb, 0x1bf34a4a, 
	0x5fa63d3d, 0x9359a4a4, 0x0abcb9b9, 0xef3af9f9, 
	0x91ef1313, 0x85fe0808, 0x49019191, 0xee611616, 
	0x2d7cdede, 0x4fb22121, 0x8f42b1b1, 0x3bdb7272, 
	0x47b82f2f, 0x8748bfbf, 0x6d2caeae, 0x46e3c0c0, 
	0xd6573c3c, 0x3e859a9a, 0x6929a9a9, 0x647d4f4f, 
	0x2a948181, 0xce492e2e, 0xcb17c6c6, 0x2fca6969, 
	0xfcc3bdbd, 0x975ca3a3, 0x055ee8e8, 0x7ad0eded, 
	0xac87d1d1, 0x7f8e0505, 0xd5ba6464, 0x1aa8a5a5, 
	0x4bb72626, 0x0eb9bebe, 0xa7608787, 0x5af8d5d5, 
	0x28223636, 0x14111b1b, 0x3fde7575, 0x2979d9d9, 
	0x88aaeeee, 0x3c332d2d, 0x4c5f7979, 0x02b6b7b7, 
	0xb896caca, 0xda583535, 0xb09cc4c4, 0x17fc4343, 
	0x551a8484, 0x1ff64d4d, 0x8a1c5959, 0x7d38b2b2, 
	0x57ac3333, 0xc718cfcf, 0x8df40606, 0x74695353, 
	0xb7749b9b, 0xc4f59797, 0x9f56adad, 0x72dae3e3, 
	0x7ed5eaea, 0x154af4f4, 0x229e8f8f, 0x12a2abab, 
	0x584e6262, 0x07e85f5f, 0x99e51d1d, 0x34392323, 
	0x6ec1f6f6, 0x50446c6c, 0xde5d3232, 0x68724646, 
	0x6526a0a0, 0xbc93cdcd, 0xdb03dada, 0xf8c6baba, 
	0xc8fa9e9e, 0xa882d6d6, 0x2bcf6e6e, 0x40507070, 
	0xdceb8585, 0xfe750a0a, 0x328a9393, 0xa48ddfdf, 
	0xca4c2929, 0x10141c1c, 0x2173d7d7, 0xf0ccb4b4, 
	0xd309d4d4, 0x5d108a8a, 0x0fe25151, 0x00000000, 
	0x6f9a1919, 0x9de01a1a, 0x368f9494, 0x42e6c7c7, 
	0x4aecc9c9, 0x5efdd2d2, 0xc1ab7f7f, 0xe0d8a8a8
    },
    {
	0xbc75bc32, 0xecf3ec21, 0x20c62043, 0xb3f4b3c9, 
	0xdadbda03, 0x027b028b, 0xe2fbe22b, 0x9ec89efa, 
	0xc94ac9ec, 0xd4d3d409, 0x18e6186b, 0x1e6b1e9f, 
	0x9845980e, 0xb27db238, 0xa6e8a6d2, 0x264b26b7, 
	0x3cd63c57, 0x9332938a, 0x82d882ee, 0x52fd5298, 
	0x7b377bd4, 0xbb71bb37, 0x5bf15b97, 0x47e14783, 
	0x2430243c, 0x510f51e2, 0xbaf8bac6, 0x4a1b4af3, 
	0xbf87bf48, 0x0dfa0d70, 0xb006b0b3, 0x753f75de, 
	0xd25ed2fd, 0x7dba7d20, 0x66ae6631, 0x3a5b3aa3, 
	0x598a591c, 0x00000000, 0xcdbccd93, 0x1a9d1ae0, 
	0xae6dae2c, 0x7fc17fab, 0x2bb12bc7, 0xbe0ebeb9, 
	0xe080e0a0, 0x8a5d8a10, 0x3bd23b52, 0x64d564ba, 
	0xd8a0d888, 0xe784e7a5, 0x5f075fe8, 0x1b141b11, 
	0x2cb52cc2, 0xfc90fcb4, 0x312c3127, 0x80a38065, 
	0x73b2732a, 0x0c730c81, 0x794c795f, 0x6b546b41, 
	0x4b924b02, 0x53745369, 0x9436948f, 0x8351831f, 
	0x2a382a36, 0xc4b0c49c, 0x22bd22c8, 0xd55ad5f8, 
	0xbdfcbdc3, 0x48604878, 0xff62ffce, 0x4c964c07, 
	0x416c4177, 0xc742c7e6, 0xebf7eb24, 0x1c101c14, 
	0x5d7c5d63, 0x36283622, 0x672767c0, 0xe98ce9af, 
	0x441344f9, 0x149514ea, 0xf59cf5bb, 0xcfc7cf18, 
	0x3f243f2d, 0xc046c0e3, 0x723b72db, 0x5470546c, 
	0x29ca294c, 0xf0e3f035, 0x088508fe, 0xc6cbc617, 
	0xf311f34f, 0x8cd08ce4, 0xa493a459, 0xcab8ca96, 
	0x68a6683b, 0xb883b84d, 0x38203828, 0xe5ffe52e, 
	0xad9fad56, 0x0b770b84, 0xc8c3c81d, 0x99cc99ff, 
	0x580358ed, 0x196f199a, 0x0e080e0a, 0x95bf957e, 
	0x70407050, 0xf7e7f730, 0x6e2b6ecf, 0x1fe21f6e, 
	0xb579b53d, 0x090c090f, 0x61aa6134, 0x57825716, 
	0x9f419f0b, 0x9d3a9d80, 0x11ea1164, 0x25b925cd, 
	0xafe4afdd, 0x459a4508, 0xdfa4df8d, 0xa397a35c, 
	0xea7eead5, 0x35da3558, 0xed7aedd0, 0x431743fc, 
	0xf866f8cb, 0xfb94fbb1, 0x37a137d3, 0xfa1dfa40, 
	0xc23dc268, 0xb4f0b4cc, 0x32de325d, 0x9cb39c71, 
	0x560b56e7, 0xe372e3da, 0x87a78760, 0x151c151b, 
	0xf9eff93a, 0x63d163bf, 0x345334a9, 0x9a3e9a85, 
	0xb18fb142, 0x7c337cd1, 0x8826889b, 0x3d5f3da6, 
	0xa1eca1d7, 0xe476e4df, 0x812a8194, 0x91499101, 
	0x0f810ffb, 0xee88eeaa, 0x16ee1661, 0xd721d773, 
	0x97c497f5, 0xa51aa5a8, 0xfeebfe3f, 0x6dd96db5, 
	0x78c578ae, 0xc539c56d, 0x1d991de5, 0x76cd76a4, 
	0x3ead3edc, 0xcb31cb67, 0xb68bb647, 0xef01ef5b, 
	0x1218121e, 0x602360c5, 0x6add6ab0, 0x4d1f4df6, 
	0xce4ecee9, 0xde2dde7c, 0x55f9559d, 0x7e487e5a, 
	0x214f21b2, 0x03f2037a, 0xa065a026, 0x5e8e5e19, 
	0x5a785a66, 0x655c654b, 0x6258624e, 0xfd19fd45, 
	0x068d06f4, 0x40e54086, 0xf298f2be, 0x335733ac, 
	0x17671790, 0x057f058e, 0xe805e85e, 0x4f644f7d, 
	0x89af896a, 0x10631095, 0x74b6742f, 0x0afe0a75, 
	0x5cf55c92, 0x9bb79b74, 0x2d3c2d33, 0x30a530d6, 
	0x2ece2e49, 0x49e94989, 0x46684672, 0x77447755, 
	0xa8e0a8d8, 0x964d9604, 0x284328bd, 0xa969a929, 
	0xd929d979, 0x862e8691, 0xd1acd187, 0xf415f44a, 
	0x8d598d15, 0xd6a8d682, 0xb90ab9bc, 0x429e420d, 
	0xf66ef6c1, 0x2f472fb8, 0xdddfdd06, 0x23342339, 
	0xcc35cc62, 0xf16af1c4, 0xc1cfc112, 0x85dc85eb, 
	0x8f228f9e, 0x71c971a1, 0x90c090f0, 0xaa9baa53, 
	0x018901f1, 0x8bd48be1, 0x4eed4e8c, 0x8eab8e6f, 
	0xab12aba2, 0x6fa26f3e, 0xe60de654, 0xdb52dbf2, 
	0x92bb927b, 0xb702b7b6, 0x692f69ca, 0x39a939d9, 
	0xd3d7d30c, 0xa761a723, 0xa21ea2ad, 0xc3b4c399, 
	0x6c506c44, 0x07040705, 0x04f6047f, 0x27c22746, 
	0xac16aca7, 0xd025d076, 0x50865013, 0xdc56dcf7, 
	0x8455841a, 0xe109e151, 0x7abe7a25, 0x139113ef
    },
    {
	0xd939a9d9, 0x90176790, 0x719cb371, 0xd2a6e8d2, 
	0x05070405, 0x9852fd98, 0x6580a365, 0xdfe476df, 
	0x08459a08, 0x024b9202, 0xa0e080a0, 0x665a7866, 
	0xddafe4dd, 0xb06addb0, 0xbf63d1bf, 0x362a3836, 
	0x54e60d54, 0x4320c643, 0x62cc3562, 0xbef298be, 
	0x1e12181e, 0x24ebf724, 0xd7a1ecd7, 0x77416c77, 
	0xbd2843bd, 0x32bc7532, 0xd47b37d4, 0x9b88269b, 
	0x700dfa70, 0xf94413f9, 0xb1fb94b1, 0x5a7e485a, 
	0x7a03f27a, 0xe48cd0e4, 0x47b68b47, 0x3c24303c, 
	0xa5e784a5, 0x416b5441, 0x06dddf06, 0xc56023c5, 
	0x45fd1945, 0xa33a5ba3, 0x68c23d68, 0x158d5915, 
	0x21ecf321, 0x3166ae31, 0x3e6fa23e, 0x16578216, 
	0x95106395, 0x5bef015b, 0x4db8834d, 0x91862e91, 
	0xb56dd9b5, 0x1f83511f, 0x53aa9b53, 0x635d7c63, 
	0x3b68a63b, 0x3ffeeb3f, 0xd630a5d6, 0x257abe25, 
	0xa7ac16a7, 0x0f090c0f, 0x35f0e335, 0x23a76123, 
	0xf090c0f0, 0xafe98caf, 0x809d3a80, 0x925cf592, 
	0x810c7381, 0x27312c27, 0x76d02576, 0xe7560be7, 
	0x7b92bb7b, 0xe9ce4ee9, 0xf10189f1, 0x9f1e6b9f, 
	0xa93453a9, 0xc4f16ac4, 0x99c3b499, 0x975bf197, 
	0x8347e183, 0x6b18e66b, 0xc822bdc8, 0x0e98450e, 
	0x6e1fe26e, 0xc9b3f4c9, 0x2f74b62f, 0xcbf866cb, 
	0xff99ccff, 0xea1495ea, 0xed5803ed, 0xf7dc56f7, 
	0xe18bd4e1, 0x1b151c1b, 0xada21ead, 0x0cd3d70c, 
	0x2be2fb2b, 0x1dc8c31d, 0x195e8e19, 0xc22cb5c2, 
	0x8949e989, 0x12c1cf12, 0x7e95bf7e, 0x207dba20, 
	0x6411ea64, 0x840b7784, 0x6dc5396d, 0x6a89af6a, 
	0xd17c33d1, 0xa171c9a1, 0xceff62ce, 0x37bb7137, 
	0xfb0f81fb, 0x3db5793d, 0x51e10951, 0xdc3eaddc, 
	0x2d3f242d, 0xa476cda4, 0x9d55f99d, 0xee82d8ee, 
	0x8640e586, 0xae78c5ae, 0xcd25b9cd, 0x04964d04, 
	0x55774455, 0x0a0e080a, 0x13508613, 0x30f7e730, 
	0xd337a1d3, 0x40fa1d40, 0x3461aa34, 0x8c4eed8c, 
	0xb3b006b3, 0x6c54706c, 0x2a73b22a, 0x523bd252, 
	0x0b9f410b, 0x8b027b8b, 0x88d8a088, 0x4ff3114f, 
	0x67cb3167, 0x4627c246, 0xc06727c0, 0xb4fc90b4, 
	0x28382028, 0x7f04f67f, 0x78486078, 0x2ee5ff2e, 
	0x074c9607, 0x4b655c4b, 0xc72bb1c7, 0x6f8eab6f, 
	0x0d429e0d, 0xbbf59cbb, 0xf2db52f2, 0xf34a1bf3, 
	0xa63d5fa6, 0x59a49359, 0xbcb90abc, 0x3af9ef3a, 
	0xef1391ef, 0xfe0885fe, 0x01914901, 0x6116ee61, 
	0x7cde2d7c, 0xb2214fb2, 0x42b18f42, 0xdb723bdb, 
	0xb82f47b8, 0x48bf8748, 0x2cae6d2c, 0xe3c046e3, 
	0x573cd657, 0x859a3e85, 0x29a96929, 0x7d4f647d, 
	0x94812a94, 0x492ece49, 0x17c6cb17, 0xca692fca, 
	0xc3bdfcc3, 0x5ca3975c, 0x5ee8055e, 0xd0ed7ad0, 
	0x87d1ac87, 0x8e057f8e, 0xba64d5ba, 0xa8a51aa8, 
	0xb7264bb7, 0xb9be0eb9, 0x6087a760, 0xf8d55af8, 
	0x22362822, 0x111b1411, 0xde753fde, 0x79d92979, 
	0xaaee88aa, 0x332d3c33, 0x5f794c5f, 0xb6b702b6, 
	0x96cab896, 0x5835da58, 0x9cc4b09c, 0xfc4317fc, 
	0x1a84551a, 0xf64d1ff6, 0x1c598a1c, 0x38b27d38, 
	0xac3357ac, 0x18cfc718, 0xf4068df4, 0x69537469, 
	0x749bb774, 0xf597c4f5, 0x56ad9f56, 0xdae372da, 
	0xd5ea7ed5, 0x4af4154a, 0x9e8f229e, 0xa2ab12a2, 
	0x4e62584e, 0xe85f07e8, 0xe51d99e5, 0x39233439, 
	0xc1f66ec1, 0x446c5044, 0x5d32de5d, 0x72466872, 
	0x26a06526, 0x93cdbc93, 0x03dadb03, 0xc6baf8c6, 
	0xfa9ec8fa, 0x82d6a882, 0xcf6e2bcf, 0x50704050, 
	0xeb85dceb, 0x750afe75, 0x8a93328a, 0x8ddfa48d, 
	0x4c29ca4c, 0x141c1014, 0x73d72173, 0xccb4f0cc, 
	0x09d4d309, 0x108a5d10, 0xe2510fe2, 0x00000000, 
	0x9a196f9a, 0xe01a9de0, 0x8f94368f, 0xe6c742e6, 
	0xecc94aec, 0xfdd25efd, 0xab7fc1ab, 0xd8a8e0d8
    }
};


} // namespace

