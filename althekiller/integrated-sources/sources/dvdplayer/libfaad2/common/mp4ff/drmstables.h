/*****************************************************************************
 * drmstables.h : DRMS tables
 *****************************************************************************
 * Copyright (C) 2004 VideoLAN
 * $Id: drmstables.h,v 1.2 2004/01/11 15:52:18 menno Exp $
 *
 * Author: Jon Lech Johansen <jon-vl@nanocrew.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111, USA.
 *****************************************************************************/


static uint32_t p_drms_tab1[ 10 ] =
{
    0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020,
    0x00000040, 0x00000080, 0x0000001B, 0x00000036
};

static uint32_t p_drms_tab2[ 256 ] =
{
    0x63000000, 0x7C000000, 0x77000000, 0x7B000000, 0xF2000000, 0x6B000000,
    0x6F000000, 0xC5000000, 0x30000000, 0x01000000, 0x67000000, 0x2B000000,
    0xFE000000, 0xD7000000, 0xAB000000, 0x76000000, 0xCA000000, 0x82000000,
    0xC9000000, 0x7D000000, 0xFA000000, 0x59000000, 0x47000000, 0xF0000000,
    0xAD000000, 0xD4000000, 0xA2000000, 0xAF000000, 0x9C000000, 0xA4000000,
    0x72000000, 0xC0000000, 0xB7000000, 0xFD000000, 0x93000000, 0x26000000,
    0x36000000, 0x3F000000, 0xF7000000, 0xCC000000, 0x34000000, 0xA5000000,
    0xE5000000, 0xF1000000, 0x71000000, 0xD8000000, 0x31000000, 0x15000000,
    0x04000000, 0xC7000000, 0x23000000, 0xC3000000, 0x18000000, 0x96000000,
    0x05000000, 0x9A000000, 0x07000000, 0x12000000, 0x80000000, 0xE2000000,
    0xEB000000, 0x27000000, 0xB2000000, 0x75000000, 0x09000000, 0x83000000,
    0x2C000000, 0x1A000000, 0x1B000000, 0x6E000000, 0x5A000000, 0xA0000000,
    0x52000000, 0x3B000000, 0xD6000000, 0xB3000000, 0x29000000, 0xE3000000,
    0x2F000000, 0x84000000, 0x53000000, 0xD1000000, 0x00000000, 0xED000000,
    0x20000000, 0xFC000000, 0xB1000000, 0x5B000000, 0x6A000000, 0xCB000000,
    0xBE000000, 0x39000000, 0x4A000000, 0x4C000000, 0x58000000, 0xCF000000,
    0xD0000000, 0xEF000000, 0xAA000000, 0xFB000000, 0x43000000, 0x4D000000,
    0x33000000, 0x85000000, 0x45000000, 0xF9000000, 0x02000000, 0x7F000000,
    0x50000000, 0x3C000000, 0x9F000000, 0xA8000000, 0x51000000, 0xA3000000,
    0x40000000, 0x8F000000, 0x92000000, 0x9D000000, 0x38000000, 0xF5000000,
    0xBC000000, 0xB6000000, 0xDA000000, 0x21000000, 0x10000000, 0xFF000000,
    0xF3000000, 0xD2000000, 0xCD000000, 0x0C000000, 0x13000000, 0xEC000000,
    0x5F000000, 0x97000000, 0x44000000, 0x17000000, 0xC4000000, 0xA7000000,
    0x7E000000, 0x3D000000, 0x64000000, 0x5D000000, 0x19000000, 0x73000000,
    0x60000000, 0x81000000, 0x4F000000, 0xDC000000, 0x22000000, 0x2A000000,
    0x90000000, 0x88000000, 0x46000000, 0xEE000000, 0xB8000000, 0x14000000,
    0xDE000000, 0x5E000000, 0x0B000000, 0xDB000000, 0xE0000000, 0x32000000,
    0x3A000000, 0x0A000000, 0x49000000, 0x06000000, 0x24000000, 0x5C000000,
    0xC2000000, 0xD3000000, 0xAC000000, 0x62000000, 0x91000000, 0x95000000,
    0xE4000000, 0x79000000, 0xE7000000, 0xC8000000, 0x37000000, 0x6D000000,
    0x8D000000, 0xD5000000, 0x4E000000, 0xA9000000, 0x6C000000, 0x56000000,
    0xF4000000, 0xEA000000, 0x65000000, 0x7A000000, 0xAE000000, 0x08000000,
    0xBA000000, 0x78000000, 0x25000000, 0x2E000000, 0x1C000000, 0xA6000000,
    0xB4000000, 0xC6000000, 0xE8000000, 0xDD000000, 0x74000000, 0x1F000000,
    0x4B000000, 0xBD000000, 0x8B000000, 0x8A000000, 0x70000000, 0x3E000000,
    0xB5000000, 0x66000000, 0x48000000, 0x03000000, 0xF6000000, 0x0E000000,
    0x61000000, 0x35000000, 0x57000000, 0xB9000000, 0x86000000, 0xC1000000,
    0x1D000000, 0x9E000000, 0xE1000000, 0xF8000000, 0x98000000, 0x11000000,
    0x69000000, 0xD9000000, 0x8E000000, 0x94000000, 0x9B000000, 0x1E000000,
    0x87000000, 0xE9000000, 0xCE000000, 0x55000000, 0x28000000, 0xDF000000,
    0x8C000000, 0xA1000000, 0x89000000, 0x0D000000, 0xBF000000, 0xE6000000,
    0x42000000, 0x68000000, 0x41000000, 0x99000000, 0x2D000000, 0x0F000000,
    0xB0000000, 0x54000000, 0xBB000000, 0x16000000
};

static uint32_t p_drms_tab3[ 256 ] =
{
    0x5150A7F4, 0x7E536541, 0x1AC3A417, 0x3A965E27, 0x3BCB6BAB, 0x1FF1459D,
    0xACAB58FA, 0x4B9303E3, 0x2055FA30, 0xADF66D76, 0x889176CC, 0xF5254C02,
    0x4FFCD7E5, 0xC5D7CB2A, 0x26804435, 0xB58FA362, 0xDE495AB1, 0x25671BBA,
    0x45980EEA, 0x5DE1C0FE, 0xC302752F, 0x8112F04C, 0x8DA39746, 0x6BC6F9D3,
    0x03E75F8F, 0x15959C92, 0xBFEB7A6D, 0x95DA5952, 0xD42D83BE, 0x58D32174,
    0x492969E0, 0x8E44C8C9, 0x756A89C2, 0xF478798E, 0x996B3E58, 0x27DD71B9,
    0xBEB64FE1, 0xF017AD88, 0xC966AC20, 0x7DB43ACE, 0x63184ADF, 0xE582311A,
    0x97603351, 0x62457F53, 0xB1E07764, 0xBB84AE6B, 0xFE1CA081, 0xF9942B08,
    0x70586848, 0x8F19FD45, 0x94876CDE, 0x52B7F87B, 0xAB23D373, 0x72E2024B,
    0xE3578F1F, 0x662AAB55, 0xB20728EB, 0x2F03C2B5, 0x869A7BC5, 0xD3A50837,
    0x30F28728, 0x23B2A5BF, 0x02BA6A03, 0xED5C8216, 0x8A2B1CCF, 0xA792B479,
    0xF3F0F207, 0x4EA1E269, 0x65CDF4DA, 0x06D5BE05, 0xD11F6234, 0xC48AFEA6,
    0x349D532E, 0xA2A055F3, 0x0532E18A, 0xA475EBF6, 0x0B39EC83, 0x40AAEF60,
    0x5E069F71, 0xBD51106E, 0x3EF98A21, 0x963D06DD, 0xDDAE053E, 0x4D46BDE6,
    0x91B58D54, 0x71055DC4, 0x046FD406, 0x60FF1550, 0x1924FB98, 0xD697E9BD,
    0x89CC4340, 0x67779ED9, 0xB0BD42E8, 0x07888B89, 0xE7385B19, 0x79DBEEC8,
    0xA1470A7C, 0x7CE90F42, 0xF8C91E84, 0x00000000, 0x09838680, 0x3248ED2B,
    0x1EAC7011, 0x6C4E725A, 0xFDFBFF0E, 0x0F563885, 0x3D1ED5AE, 0x3627392D,
    0x0A64D90F, 0x6821A65C, 0x9BD1545B, 0x243A2E36, 0x0CB1670A, 0x930FE757,
    0xB4D296EE, 0x1B9E919B, 0x804FC5C0, 0x61A220DC, 0x5A694B77, 0x1C161A12,
    0xE20ABA93, 0xC0E52AA0, 0x3C43E022, 0x121D171B, 0x0E0B0D09, 0xF2ADC78B,
    0x2DB9A8B6, 0x14C8A91E, 0x578519F1, 0xAF4C0775, 0xEEBBDD99, 0xA3FD607F,
    0xF79F2601, 0x5CBCF572, 0x44C53B66, 0x5B347EFB, 0x8B762943, 0xCBDCC623,
    0xB668FCED, 0xB863F1E4, 0xD7CADC31, 0x42108563, 0x13402297, 0x842011C6,
    0x857D244A, 0xD2F83DBB, 0xAE1132F9, 0xC76DA129, 0x1D4B2F9E, 0xDCF330B2,
    0x0DEC5286, 0x77D0E3C1, 0x2B6C16B3, 0xA999B970, 0x11FA4894, 0x472264E9,
    0xA8C48CFC, 0xA01A3FF0, 0x56D82C7D, 0x22EF9033, 0x87C74E49, 0xD9C1D138,
    0x8CFEA2CA, 0x98360BD4, 0xA6CF81F5, 0xA528DE7A, 0xDA268EB7, 0x3FA4BFAD,
    0x2CE49D3A, 0x500D9278, 0x6A9BCC5F, 0x5462467E, 0xF6C2138D, 0x90E8B8D8,
    0x2E5EF739, 0x82F5AFC3, 0x9FBE805D, 0x697C93D0, 0x6FA92DD5, 0xCFB31225,
    0xC83B99AC, 0x10A77D18, 0xE86E639C, 0xDB7BBB3B, 0xCD097826, 0x6EF41859,
    0xEC01B79A, 0x83A89A4F, 0xE6656E95, 0xAA7EE6FF, 0x2108CFBC, 0xEFE6E815,
    0xBAD99BE7, 0x4ACE366F, 0xEAD4099F, 0x29D67CB0, 0x31AFB2A4, 0x2A31233F,
    0xC63094A5, 0x35C066A2, 0x7437BC4E, 0xFCA6CA82, 0xE0B0D090, 0x3315D8A7,
    0xF14A9804, 0x41F7DAEC, 0x7F0E50CD, 0x172FF691, 0x768DD64D, 0x434DB0EF,
    0xCC544DAA, 0xE4DF0496, 0x9EE3B5D1, 0x4C1B886A, 0xC1B81F2C, 0x467F5165,
    0x9D04EA5E, 0x015D358C, 0xFA737487, 0xFB2E410B, 0xB35A1D67, 0x9252D2DB,
    0xE9335610, 0x6D1347D6, 0x9A8C61D7, 0x377A0CA1, 0x598E14F8, 0xEB893C13,
    0xCEEE27A9, 0xB735C961, 0xE1EDE51C, 0x7A3CB147, 0x9C59DFD2, 0x553F73F2,
    0x1879CE14, 0x73BF37C7, 0x53EACDF7, 0x5F5BAAFD, 0xDF146F3D, 0x7886DB44,
    0xCA81F3AF, 0xB93EC468, 0x382C3424, 0xC25F40A3, 0x1672C31D, 0xBC0C25E2,
    0x288B493C, 0xFF41950D, 0x397101A8, 0x08DEB30C, 0xD89CE4B4, 0x6490C156,
    0x7B6184CB, 0xD570B632, 0x48745C6C, 0xD04257B8
};

static uint32_t p_drms_tab4[ 256 ] =
{
    0x52000000, 0x09000000, 0x6A000000, 0xD5000000, 0x30000000, 0x36000000,
    0xA5000000, 0x38000000, 0xBF000000, 0x40000000, 0xA3000000, 0x9E000000,
    0x81000000, 0xF3000000, 0xD7000000, 0xFB000000, 0x7C000000, 0xE3000000,
    0x39000000, 0x82000000, 0x9B000000, 0x2F000000, 0xFF000000, 0x87000000,
    0x34000000, 0x8E000000, 0x43000000, 0x44000000, 0xC4000000, 0xDE000000,
    0xE9000000, 0xCB000000, 0x54000000, 0x7B000000, 0x94000000, 0x32000000,
    0xA6000000, 0xC2000000, 0x23000000, 0x3D000000, 0xEE000000, 0x4C000000,
    0x95000000, 0x0B000000, 0x42000000, 0xFA000000, 0xC3000000, 0x4E000000,
    0x08000000, 0x2E000000, 0xA1000000, 0x66000000, 0x28000000, 0xD9000000,
    0x24000000, 0xB2000000, 0x76000000, 0x5B000000, 0xA2000000, 0x49000000,
    0x6D000000, 0x8B000000, 0xD1000000, 0x25000000, 0x72000000, 0xF8000000,
    0xF6000000, 0x64000000, 0x86000000, 0x68000000, 0x98000000, 0x16000000,
    0xD4000000, 0xA4000000, 0x5C000000, 0xCC000000, 0x5D000000, 0x65000000,
    0xB6000000, 0x92000000, 0x6C000000, 0x70000000, 0x48000000, 0x50000000,
    0xFD000000, 0xED000000, 0xB9000000, 0xDA000000, 0x5E000000, 0x15000000,
    0x46000000, 0x57000000, 0xA7000000, 0x8D000000, 0x9D000000, 0x84000000,
    0x90000000, 0xD8000000, 0xAB000000, 0x00000000, 0x8C000000, 0xBC000000,
    0xD3000000, 0x0A000000, 0xF7000000, 0xE4000000, 0x58000000, 0x05000000,
    0xB8000000, 0xB3000000, 0x45000000, 0x06000000, 0xD0000000, 0x2C000000,
    0x1E000000, 0x8F000000, 0xCA000000, 0x3F000000, 0x0F000000, 0x02000000,
    0xC1000000, 0xAF000000, 0xBD000000, 0x03000000, 0x01000000, 0x13000000,
    0x8A000000, 0x6B000000, 0x3A000000, 0x91000000, 0x11000000, 0x41000000,
    0x4F000000, 0x67000000, 0xDC000000, 0xEA000000, 0x97000000, 0xF2000000,
    0xCF000000, 0xCE000000, 0xF0000000, 0xB4000000, 0xE6000000, 0x73000000,
    0x96000000, 0xAC000000, 0x74000000, 0x22000000, 0xE7000000, 0xAD000000,
    0x35000000, 0x85000000, 0xE2000000, 0xF9000000, 0x37000000, 0xE8000000,
    0x1C000000, 0x75000000, 0xDF000000, 0x6E000000, 0x47000000, 0xF1000000,
    0x1A000000, 0x71000000, 0x1D000000, 0x29000000, 0xC5000000, 0x89000000,
    0x6F000000, 0xB7000000, 0x62000000, 0x0E000000, 0xAA000000, 0x18000000,
    0xBE000000, 0x1B000000, 0xFC000000, 0x56000000, 0x3E000000, 0x4B000000,
    0xC6000000, 0xD2000000, 0x79000000, 0x20000000, 0x9A000000, 0xDB000000,
    0xC0000000, 0xFE000000, 0x78000000, 0xCD000000, 0x5A000000, 0xF4000000,
    0x1F000000, 0xDD000000, 0xA8000000, 0x33000000, 0x88000000, 0x07000000,
    0xC7000000, 0x31000000, 0xB1000000, 0x12000000, 0x10000000, 0x59000000,
    0x27000000, 0x80000000, 0xEC000000, 0x5F000000, 0x60000000, 0x51000000,
    0x7F000000, 0xA9000000, 0x19000000, 0xB5000000, 0x4A000000, 0x0D000000,
    0x2D000000, 0xE5000000, 0x7A000000, 0x9F000000, 0x93000000, 0xC9000000,
    0x9C000000, 0xEF000000, 0xA0000000, 0xE0000000, 0x3B000000, 0x4D000000,
    0xAE000000, 0x2A000000, 0xF5000000, 0xB0000000, 0xC8000000, 0xEB000000,
    0xBB000000, 0x3C000000, 0x83000000, 0x53000000, 0x99000000, 0x61000000,
    0x17000000, 0x2B000000, 0x04000000, 0x7E000000, 0xBA000000, 0x77000000,
    0xD6000000, 0x26000000, 0xE1000000, 0x69000000, 0x14000000, 0x63000000,
    0x55000000, 0x21000000, 0x0C000000, 0x7D000000
};

static int32_t p_drms_tab_taos[ 64 ] =
{
    -0x28955B88, -0x173848AA, +0x242070DB, -0x3E423112, -0x0A83F051,
    +0x4787C62A, -0x57CFB9ED, -0x02B96AFF, +0x698098D8, -0x74BB0851,
    -0x0000A44F, -0x76A32842, +0x6B901122, -0x02678E6D, -0x5986BC72,
    +0x49B40821, -0x09E1DA9E, -0x3FBF4CC0, +0x265E5A51, -0x16493856,
    -0x29D0EFA3, +0x02441453, -0x275E197F, -0x182C0438, +0x21E1CDE6,
    -0x3CC8F82A, -0x0B2AF279, +0x455A14ED, -0x561C16FB, -0x03105C08,
    +0x676F02D9, -0x72D5B376, -0x0005C6BE, -0x788E097F, +0x6D9D6122,
    -0x021AC7F4, -0x5B4115BC, +0x4BDECFA9, -0x0944B4A0, -0x41404390,
    +0x289B7EC6, -0x155ED806, -0x2B10CF7B, +0x04881D05, -0x262B2FC7,
    -0x1924661B, +0x1FA27CF8, -0x3B53A99B, -0x0BD6DDBC, +0x432AFF97,
    -0x546BDC59, -0x036C5FC7, +0x655B59C3, -0x70F3336E, -0x00100B83,
    -0x7A7BA22F, +0x6FA87E4F, -0x01D31920, -0x5CFEBCEC, +0x4E0811A1,
    -0x08AC817E, -0x42C50DCB, +0x2AD7D2BB, -0x14792C6F
};

static uint8_t p_drms_tab_tend[ 64 ] =
{
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static uint16_t p_drms_tab_xor[ 16 ][ 16 ] =
{
    {
        0x00D1, 0x0315, 0x1A32, 0x19EC, 0x1BBB, 0x1D6F, 0x14FE, 0x0E9E,
        0x029E, 0x1B8F, 0x0B70, 0x033A, 0x188E, 0x1D18, 0x0BD8, 0x0EDB
    },

    {
        0x0C64, 0x1C2B, 0x149C, 0x047B, 0x1064, 0x1C7C, 0x118D, 0x1355,
        0x0AE5, 0x0F18, 0x016F, 0x17D6, 0x1595, 0x0084, 0x0616, 0x1CCD
    },

    {
        0x1D94, 0x0618, 0x182C, 0x195B, 0x196D, 0x0394, 0x07DB, 0x0287,
        0x1636, 0x0B81, 0x1519, 0x0DF9, 0x1BA3, 0x1CC3, 0x0EE2, 0x1434
    },

    {
        0x1457, 0x0CED, 0x0F7D, 0x0D7B, 0x0B9E, 0x0D13, 0x13D7, 0x18D0,
        0x1259, 0x1977, 0x0606, 0x1E80, 0x05F2, 0x06B8, 0x1F07, 0x1365
    },

    {
        0x0334, 0x0E30, 0x195F, 0x15F1, 0x058E, 0x0AA8, 0x045A, 0x0465,
        0x0B3E, 0x071E, 0x0A36, 0x105C, 0x01AC, 0x1A1E, 0x04E4, 0x056B
    },

    {
        0x12BF, 0x0DA2, 0x0B41, 0x0EAF, 0x034F, 0x0181, 0x04E2, 0x002B,
        0x12E6, 0x01BE, 0x10E8, 0x128F, 0x0EB2, 0x1369, 0x05BE, 0x1A59
    },

    {
        0x117E, 0x047C, 0x1E86, 0x056A, 0x0DA7, 0x0D61, 0x03FC, 0x1E6E,
        0x1D0C, 0x1E6D, 0x14BF, 0x0C50, 0x063A, 0x1B47, 0x17AE, 0x1321
    },

    {
        0x041B, 0x0A24, 0x0D4D, 0x1F2B, 0x1CB6, 0x1BED, 0x1549, 0x03A7,
        0x0254, 0x006C, 0x0C9E, 0x0F73, 0x006C, 0x0008, 0x11F9, 0x0DD5
    },

    {
        0x0BCF, 0x0AF9, 0x1DFE, 0x0341, 0x0E49, 0x0D38, 0x17CB, 0x1513,
        0x0E96, 0x00ED, 0x0556, 0x1B28, 0x100C, 0x19D8, 0x14FA, 0x028C
    },

    {
        0x1C60, 0x1232, 0x13D3, 0x0D00, 0x1534, 0x192C, 0x14B5, 0x1CF2,
        0x0504, 0x0B5B, 0x1ECF, 0x0423, 0x183B, 0x06B0, 0x169E, 0x1066
    },

    {
        0x04CB, 0x08A2, 0x1B4A, 0x1254, 0x198D, 0x1044, 0x0236, 0x1BD8,
        0x18A1, 0x03FF, 0x1A0D, 0x0277, 0x0C2D, 0x17C9, 0x007C, 0x116E
    },

    {
        0x048A, 0x1EAF, 0x0922, 0x0C45, 0x0766, 0x1E5F, 0x1A28, 0x0120,
        0x1C15, 0x034C, 0x0508, 0x0E73, 0x0879, 0x0441, 0x09AE, 0x132F
    },

    {
        0x14FE, 0x0413, 0x0A9D, 0x1727, 0x01D7, 0x1A2B, 0x0474, 0x18F0,
        0x1F3B, 0x14F5, 0x1071, 0x0895, 0x1071, 0x18FF, 0x18E3, 0x0EB9
    },

    {
        0x0BA9, 0x0961, 0x1599, 0x019E, 0x1D12, 0x1BAA, 0x1E94, 0x1921,
        0x14DC, 0x124E, 0x0A25, 0x03AB, 0x1CC0, 0x1EBB, 0x0B4B, 0x16E5
    },

    {
        0x11EA, 0x0D78, 0x1BB3, 0x1BA7, 0x1510, 0x1B7B, 0x0C64, 0x1995,
        0x1A58, 0x1651, 0x1964, 0x147A, 0x15F2, 0x11BB, 0x1654, 0x166E
    },

    {
        0x0EA9, 0x1DE1, 0x1443, 0x13C5, 0x00E1, 0x0B2F, 0x0B6F, 0x0A37,
        0x18AC, 0x08E6, 0x06F0, 0x136E, 0x0853, 0x0B2E, 0x0813, 0x10D6
    }
};

static uint16_t p_drms_tab_sub[ 16 ][ 16 ] =
{
    {
        0x067A, 0x0C7D, 0x0B4F, 0x127D, 0x0BD6, 0x04AC, 0x16E0, 0x1730,
        0x0587, 0x0AFB, 0x1AC3, 0x0120, 0x14B5, 0x0F67, 0x11DE, 0x0961
    },

    {
        0x1127, 0x1A68, 0x07F0, 0x17D0, 0x1A6F, 0x1F3B, 0x01EF, 0x0919,
        0x131E, 0x0F90, 0x19E9, 0x18A8, 0x0CB2, 0x1AD0, 0x0C66, 0x0378
    },

    {
        0x03B0, 0x01BE, 0x1866, 0x1159, 0x197C, 0x1105, 0x010B, 0x0353,
        0x1ABB, 0x09A6, 0x028A, 0x1BAD, 0x1B20, 0x0455, 0x0F57, 0x0588
    },

    {
        0x1491, 0x0A1D, 0x0F04, 0x0650, 0x191E, 0x1E0E, 0x174B, 0x016B,
        0x051F, 0x0532, 0x00DF, 0x1AEA, 0x0005, 0x0E1B, 0x0FF6, 0x08D8
    },

    {
        0x14B4, 0x086A, 0x0C20, 0x0149, 0x1971, 0x0F26, 0x1852, 0x017D,
        0x1228, 0x0352, 0x0A44, 0x1330, 0x18DF, 0x1E38, 0x01BC, 0x0BAC
    },

    {
        0x1A48, 0x021F, 0x02F7, 0x0C31, 0x0BC4, 0x1E75, 0x105C, 0x13E3,
        0x0B20, 0x03A1, 0x1AF3, 0x1A36, 0x0E34, 0x181F, 0x09BD, 0x122B
    },

    {
        0x0EE0, 0x163B, 0x0BE7, 0x103D, 0x1075, 0x1E9D, 0x02AF, 0x0BA2,
        0x1DAA, 0x0CF1, 0x04B6, 0x0598, 0x06A1, 0x0D33, 0x1CFE, 0x04EE
    },

    {
        0x1BAD, 0x07C8, 0x1A48, 0x05E6, 0x031F, 0x0E0A, 0x0326, 0x1650,
        0x0526, 0x0B4E, 0x08FC, 0x0E4D, 0x0832, 0x06EA, 0x09BF, 0x0993
    },

    {
        0x09EB, 0x0F31, 0x071B, 0x14D5, 0x11CA, 0x0722, 0x120D, 0x014C,
        0x1993, 0x0AE4, 0x1CCB, 0x04E9, 0x0AEE, 0x1708, 0x0C3D, 0x12F2
    },

    {
        0x1A19, 0x07C1, 0x05A7, 0x0744, 0x1606, 0x1A9B, 0x042D, 0x1BFC,
        0x1841, 0x0C3C, 0x0FFE, 0x1AB1, 0x1416, 0x18A9, 0x0320, 0x1EC2
    },

    {
        0x0AE7, 0x11C6, 0x124A, 0x11DF, 0x0F81, 0x06CF, 0x0ED9, 0x0253,
        0x1D2B, 0x0349, 0x0805, 0x08B3, 0x1052, 0x12CF, 0x0A44, 0x0EA6
    },

    {
        0x03BF, 0x1D90, 0x0EF8, 0x0657, 0x156D, 0x0405, 0x10BE, 0x091F,
        0x1C82, 0x1725, 0x19EF, 0x0B8C, 0x04D9, 0x02C7, 0x025A, 0x1B89
    },

    {
        0x0F5C, 0x013D, 0x02F7, 0x12E3, 0x0BC5, 0x1B56, 0x0848, 0x0239,
        0x0FCF, 0x03A4, 0x092D, 0x1354, 0x1D83, 0x01BD, 0x071A, 0x0AF1
    },

    {
        0x0875, 0x0793, 0x1B41, 0x1782, 0x0DEF, 0x1D20, 0x13BE, 0x0095,
        0x1650, 0x19D4, 0x0DE3, 0x0980, 0x18F2, 0x0CA3, 0x0098, 0x149A
    },

    {
        0x0B81, 0x0AD2, 0x1BBA, 0x1A02, 0x027B, 0x1906, 0x07F5, 0x1CAE,
        0x0C3F, 0x02F6, 0x1298, 0x175E, 0x15B2, 0x13D8, 0x14CC, 0x161A
    },

    {
        0x0A42, 0x15F3, 0x0870, 0x1C1D, 0x1203, 0x18B1, 0x1738, 0x1954,
        0x1143, 0x1AE8, 0x1D9D, 0x155B, 0x11E8, 0x0ED9, 0x06F7, 0x04CA
    }
};

static uint16_t p_drms_tab_add[ 16 ][ 16 ] =
{
    {
        0x0706, 0x175A, 0x0DEF, 0x1E72, 0x0297, 0x1B0E, 0x1D5A, 0x15B8,
        0x13E2, 0x1347, 0x10C6, 0x0B4F, 0x0629, 0x0A75, 0x0A9B, 0x0F55
    },

    {
        0x1A69, 0x09BF, 0x0BA6, 0x1582, 0x1086, 0x1921, 0x01CB, 0x1C6A,
        0x0FF5, 0x00F7, 0x0A67, 0x0A1E, 0x1838, 0x0196, 0x10D6, 0x0C7A
    },

    {
        0x180E, 0x038D, 0x1ADD, 0x0684, 0x154A, 0x0AB0, 0x18A4, 0x0D73,
        0x1641, 0x0EC6, 0x09F1, 0x1A62, 0x0414, 0x162A, 0x194E, 0x1EC9
    },

    {
        0x022F, 0x0296, 0x1104, 0x14FC, 0x096C, 0x1D02, 0x09BD, 0x027C,
        0x080E, 0x1324, 0x128C, 0x0DC1, 0x00B9, 0x17F2, 0x0CBC, 0x0F97
    },

    {
        0x1B93, 0x1C3C, 0x0415, 0x0395, 0x0C7A, 0x06CC, 0x0D4B, 0x16E2,
        0x04A2, 0x0DAB, 0x1228, 0x012B, 0x0896, 0x0012, 0x1CD6, 0x1DAC
    },

    {
        0x080D, 0x0446, 0x047A, 0x00AD, 0x029E, 0x0686, 0x17C3, 0x1466,
        0x0D16, 0x1896, 0x076E, 0x00CD, 0x17DC, 0x1E9F, 0x1A7C, 0x02BB
    },

    {
        0x0D06, 0x112B, 0x14CB, 0x0A03, 0x1541, 0x1290, 0x0F6D, 0x1503,
        0x084B, 0x0382, 0x1A3F, 0x0371, 0x1977, 0x0B67, 0x0CAD, 0x1DF8
    },

    {
        0x1CE3, 0x1306, 0x13F8, 0x1163, 0x1B0B, 0x00BD, 0x0BF0, 0x1A4F,
        0x16F7, 0x0B4F, 0x0CF8, 0x1254, 0x0541, 0x100D, 0x0296, 0x0410
    },

    {
        0x1A2B, 0x1169, 0x17D9, 0x0819, 0x03D6, 0x0D03, 0x194D, 0x184A,
        0x07CA, 0x1989, 0x0FAD, 0x011C, 0x1C71, 0x0EF6, 0x0DC8, 0x0F2F
    },

    {
        0x0FA5, 0x11BE, 0x0F3B, 0x1D52, 0x0DE2, 0x016E, 0x1AD1, 0x0C4A,
        0x1BC2, 0x0AC9, 0x1485, 0x1BEE, 0x0949, 0x1A79, 0x1894, 0x12BB
    },

    {
        0x17B6, 0x14F5, 0x16B1, 0x142C, 0x1301, 0x03EF, 0x16FF, 0x0D37,
        0x0D78, 0x01FF, 0x00D6, 0x1053, 0x1A2A, 0x0F61, 0x1352, 0x0C7F
    },

    {
        0x137F, 0x09C4, 0x1D96, 0x021D, 0x1037, 0x1B19, 0x10EF, 0x14E4,
        0x02A0, 0x0236, 0x0A5D, 0x1519, 0x141C, 0x1399, 0x007E, 0x1E74
    },

    {
        0x0941, 0x1B3C, 0x0062, 0x0371, 0x09AD, 0x08E8, 0x0A24, 0x0B97,
        0x1ED2, 0x0889, 0x136B, 0x0006, 0x1C4C, 0x0444, 0x06F8, 0x0DFB
    },

    {
        0x1D0F, 0x198D, 0x0700, 0x0AFC, 0x1781, 0x12F3, 0x10DA, 0x1F19,
        0x1055, 0x0DC9, 0x1860, 0x012B, 0x05BF, 0x082D, 0x0C17, 0x1941
    },

    {
        0x0359, 0x1232, 0x104C, 0x0762, 0x0897, 0x1D6C, 0x030F, 0x1A36,
        0x16B0, 0x094D, 0x1782, 0x036F, 0x0EEA, 0x06E6, 0x0D00, 0x0187
    },

    {
        0x17E2, 0x05E5, 0x19FA, 0x1950, 0x146A, 0x0B2A, 0x0512, 0x0EE0,
        0x1E27, 0x112D, 0x1DF0, 0x0B13, 0x0378, 0x1DD0, 0x00C1, 0x01E6
    }
};

