/* This header file is in the public domain.
   Most of its content is generated by gperf, with modifications
   Chinese frequency table courtesy of Dept. of CS, Tsinghua Univeristy, Beijing
   See http://fhpi.yingkou.net.cn/bbs/1951/messages/2903.html
*/

/* ANSI-C code produced by gperf version 3.0.1 */
/* Command-line: gperf -L ANSI-C -I -C -E -n -o -c -D -t zh_weight_big5.txt  */
/* Computed positions: -k'1-2' */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

/* maximum key range = 987, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
/*ARGSUSED*/
static unsigned int
hash_big5 (register const unsigned char *str)
{
  static const unsigned short asso_values[] =
    {
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 333, 104, 109, 139, 400, 987,
       98, 224, 306, 468, 214, 987, 381, 233,  93, 240,
       56, 286, 987, 119, 251,  59, 214, 221,   4, 245,
      987, 204, 199, 228,  10,  84, 278, 356,   0, 105,
      366, 371, 174, 381,  79,  98,  19, 169, 356, 450,
      216, 508, 298, 290, 987,  58,  96,  48,  38, 176,
      184,  10, 205, 146, 118, 440, 193, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 987, 987, 987, 987, 987, 987, 987, 987, 987,
      987, 305, 987, 238,  10,   5,   0,  85,  25, 285,
      115,  30, 180,  75, 415, 238,  15, 260,   3, 160,
       68,  60, 270,  18, 265,  90, 130, 360, 245, 485,
      495, 345, 385, 133, 480, 365, 435, 110, 445, 473,
      505, 455, 166, 410, 448, 153,  28,  39, 455, 110,
      500, 215, 340, 343, 164, 243,  96, 470, 325,  91,
      378, 391,  29,  14, 385,   4,   3, 171,  56, 506,
      168, 448,  26, 144, 441, 351, 360, 458,   5, 241,
      225, 311, 303,  46,  11, 493, 418, 285, 255, 438,
      398, 301, 453,   1, 136, 987
    };
  return asso_values[(unsigned char)str[1]] + asso_values[(unsigned char)str[0]];
}

static const struct zh_weight *in_big5 (register const unsigned char *str)
{
  enum
    {
      TOTAL_KEYWORDS = 500,
      MIN_WORD_LENGTH = 2,
      MAX_WORD_LENGTH = 2,
      MIN_HASH_VALUE = 0,
      MAX_HASH_VALUE = 986
    };

  static const struct zh_weight wordlist[] =
    {
      {"\246b",0.0082045},
      {"\246\375",0.0010253},
      {"\246\342",0.0005973},
      {"\246X",0.0023691},
      {"\245\246",0.0005788},
      {"\245\375",0.0010976},
      {"\245\262",0.0006812},
      {"\245X",0.0042527},
      {"\246^",0.0006653},
      {"\244\375",0.0006086},
      {"\244\342",0.0009187},
      {"\244\341",0.0005598},
      {"\245\364",0.0012122},
      {"\260\342",0.0004988},
      {"\245\337",0.0015778},
      {"\244\244",0.0076843},
      {"\244\364",0.0016446},
      {"\267\245",0.0006288},
      {"\244\337",0.0014967},
      {"\246\250",0.0037219},
      {"\246\350",0.0012152},
      {"\250\342",0.0013376},
      {"\244j",0.0066078},
      {"\250\356",0.0026895},
      {"\262\316",0.0011475},
      {"\245\316",0.002668},
      {"\253\341",0.0021836},
      {"\245\253",0.0046659},
      {"\244\350",0.002853},
      {"\244\316",0.0012255},
      {"\244\336",0.0006632},
      {"\260\250",0.0005587},
      {"\262v",0.0005754},
      {"\245v",0.0004954},
      {"\253\337",0.0005375},
      {"\253\260",0.0010251},
      {"\246\363",0.0005162},
      {"\244v",0.0006239},
      {"\244\317",0.0006176},
      {"\245\363",0.0009711},
      {"\245u",0.0008284},
      {"\260\317",0.0022413},
      {"\250\253",0.0006468},
      {"\246P",0.0026107},
      {"\244u",0.0050114},
      {"\246U",0.001779},
      {"\265\246",0.0006082},
      {"\265\375",0.0007621},
      {"\265\262",0.0011242},
      {"\250\317",0.001449},
      {"\265\245",0.0021605},
      {"\244\344",0.0007224},
      {"\244s",0.0011044},
      {"\244U",0.0021357},
      {"\244\265",0.0013712},
      {"\250\363",0.0007439},
      {"\264X",0.0008733},
      {"\245\264",0.0005953},
      {"\267P",0.0005274},
      {"\260\265",0.0007036},
      {"\267s",0.002897},
      {"\255\262",0.0012506},
      {"\246h",0.0032382},
      {"\245\255",0.0016647},
      {"\250\344",0.0017313},
      {"\250s",0.0007264},
      {"\245h",0.0013444},
      {"\244\255",0.0007467},
      {"\253\344",0.0006569},
      {"\247\342",0.0010343},
      {"\245_",0.0015279},
      {"\246\271",0.0011388},
      {"\246\333",0.0022824},
      {"\255\267",0.0007326},
      {"\244_",0.0026086},
      {"\244\247",0.001919},
      {"\245\333",0.0004908},
      {"\245N",0.0016641},
      {"\260_",0.0015911},
      {"\250\255",0.0006168},
      {"\245\330",0.0015919},
      {"\245i",0.0018478},
      {"\246A",0.0005426},
      {"\255\253",0.0023459},
      {"\244F",0.0079237},
      {"\253h",0.0005586},
      {"\246\321",0.0008237},
      {"\267N",0.0011669},
      {"\247\316",0.0009425},
      {"\247\336",0.0016091},
      {"\245\321",0.0013801},
      {"\267F",0.0009596},
      {"\271\316",0.0009523},
      {"\252\341",0.0005451},
      {"\244\321",0.0016483},
      {"\250t",0.0013262},
      {"\245|",0.0009759},
      {"\264\344",0.0005587},
      {"\260\321",0.0006661},
      {"\253\330",0.0032052},
      {"\264\265",0.0006023},
      {"\244S",0.0009509},
      {"\260\252",0.0025748},
      {"\247\363",0.0008908},
      {"\260|",0.0010515},
      {"\250B",0.0010507},
      {"\250\321",0.000632},
      {"\267|",0.0051579},
      {"\245\301",0.0035945},
      {"\246C",0.0005658},
      {"\244\272",0.0016149},
      {"\245\376",0.0032345},
      {"\250|",0.0009705},
      {"\250S",0.0008714},
      {"\252\253",0.0010483},
      {"\246{",0.000633},
      {"\305v",0.0009164},
      {"\262{",0.0025147},
      {"\265\271",0.0007847},
      {"\265\333",0.0014365},
      {"\252v",0.0009927},
      {"\244\351",0.0036327},
      {"\250\272",0.0007224},
      {"\265\330",0.0012136},
      {"\245\315",0.0039844},
      {"\272\336",0.001622},
      {"\246\263",0.0077624},
      {"\264N",0.0021619},
      {"\262\263",0.0006362},
      {"\250C",0.000715},
      {"\255\271",0.0004878},
      {"\246\312",0.0006978},
      {"\246\346",0.0041784},
      {"\253C",0.0005662},
      {"\263y",0.0009483},
      {"\255t",0.0005072},
      {"\245\346",0.0011534},
      {"\244\326",0.0009361},
      {"\247\271",0.0006563},
      {"\246w",0.00114},
      {"\265|",0.0007012},
      {"\244k",0.0006045},
      {"\246\254",0.0012488},
      {"\260\312",0.0023694},
      {"\247i",0.0005829},
      {"\244f",0.0012146},
      {"\245\254",0.0005445},
      {"\244w",0.0015716},
      {"\271F",0.0012618},
      {"\245x",0.0008597},
      {"\254y",0.0008055},
      {"\250\312",0.0012137},
      {"\246~",0.0071346},
      {"\271A",0.001983},
      {"\254\260",0.0054216},
      {"\250\343",0.0006981},
      {"\245~",0.0022498},
      {"\271B",0.0009758},
      {"\252\247",0.0007016},
      {"\264\301",0.0012406},
      {"\244~",0.0006942},
      {"\245\\",0.0005073},
      {"\255\272",0.0006626},
      {"\265{",0.0012378},
      {"\262z",0.0029849},
      {"\245[",0.0023829},
      {"\253\254",0.0006771},
      {"\267~",0.0065423},
      {"\252F",0.0011892},
      {"\246V",0.0014073},
      {"\246\323",0.0018881},
      {"\246n",0.0017451},
      {"\263s",0.0004976},
      {"\252A",0.0008639},
      {"\271\272",0.0007059},
      {"\246W",0.0014078},
      {"\253~",0.0023513},
      {"\244J",0.0015621},
      {"\252\321",0.0005495},
      {"\272\330",0.0018411},
      {"\246]",0.0010577},
      {"\260\323",0.0016298},
      {"\244W",0.0048331},
      {"\245]",0.0006314},
      {"\244G",0.001247},
      {"\272c",0.0007005},
      {"\262M",0.0005405},
      {"\244]",0.001974},
      {"\250\323",0.0034731},
      {"\262\243",0.0043545},
      {"\260]",0.0005022},
      {"\252\272",0.0341277},
      {"\253n",0.00114},
      {"\244\243",0.0063303},
      {"\247\326",0.0006737},
      {"\244O",0.0022943},
      {"\301|",0.0006836},
      {"\263N",0.0014501},
      {"\305\351",0.0021551},
      {"\260O",0.0013592},
      {"\263t",0.0005376},
      {"\250M",0.0011549},
      {"\255x",0.0009727},
      {"\245\370",0.0027556},
      {"\244T",0.0017945},
      {"\250\243",0.0007272},
      {"\246\270",0.0014362},
      {"\260T",0.000496},
      {"\257\253",0.0005762},
      {"\263B",0.0009814},
      {"\253O",0.0014976},
      {"\254\333",0.0010087},
      {"\253\327",0.0015281},
      {"\244\270",0.002067},
      {"\270\364",0.001188},
      {"\254F",0.002679},
      {"\261j",0.0013442},
      {"\266\244",0.0009591},
      {"\262`",0.0008945},
      {"\274v",0.000514},
      {"\252k",0.0025935},
      {"\266\260",0.0010417},
      {"\257u",0.0006076},
      {"\251\262",0.0009796},
      {"\255\323",0.0043881},
      {"\255n",0.0045003},
      {"\265M",0.001088},
      {"\245q",0.0017401},
      {"\244Q",0.0016297},
      {"\246p",0.0012367},
      {"\247J",0.0005433},
      {"\247\323",0.0008129},
      {"\246\373",0.0005246},
      {"\274s",0.0011772},
      {"\267Q",0.0008423},
      {"\246\241",0.0008493},
      {"\264\243",0.0016334},
      {"\244p",0.0016135},
      {"\245\241",0.0004698},
      {"\245H",0.0038364},
      {"\251\316",0.0006972},
      {"\265\370",0.000674},
      {"\244H",0.0080773},
      {"\260\362",0.0014197},
      {"\261\265",0.0007936},
      {"\244\361",0.001161},
      {"\270s",0.0005419},
      {"\270U",0.0020929},
      {"\247O",0.0007577},
      {"\247\357",0.0018059},
      {"\263\346",0.0008341},
      {"\266\265",0.0011181},
      {"\271\357",0.0032427},
      {"\246@",0.0010536},
      {"\263f",0.0005011},
      {"\255\261",0.0020167},
      {"\253H",0.000977},
      {"\245@",0.0010821},
      {"\252G",0.0008799},
      {"\247\370",0.0009976},
      {"\274t",0.0012368},
      {"\244@",0.0112731},
      {"\246\277",0.0009106},
      {"\262\325",0.0011894},
      {"\257\305",0.0009423},
      {"\245\277",0.0013077},
      {"\254\343",0.0009783},
      {"\261N",0.0014281},
      {"\277\244",0.0009752},
      {"\246a",0.0047688},
      {"\257S",0.0012592},
      {"\261i",0.0007204},
      {"\263\\",0.0005616},
      {"\246\354",0.0016007},
      {"\244\353",0.0024389},
      {"\273\342",0.0011289},
      {"\245\273",0.0024993},
      {"\244l",0.0015414},
      {"\266i",0.0032419},
      {"\252\370",0.0025652},
      {"\247Q",0.0018856},
      {"\255p",0.00135},
      {"\270\321",0.0011085},
      {"\255\373",0.0018995},
      {"\246\334",0.0007778},
      {"\271q",0.001873},
      {"\263W",0.0013724},
      {"\245\334",0.0004949},
      {"\250\354",0.0036926},
      {"\245L",0.0031435},
      {"\263]",0.0022181},
      {"\250\303",0.0012383},
      {"\244g",0.0005437},
      {"\264\332",0.0005576},
      {"\244\300",0.002317},
      {"\300\364",0.0006218},
      {"\263\243",0.0014503},
      {"\252\367",0.001743},
      {"\253e",0.0020531},
      {"\245\372",0.0005461},
      {"\245D",0.0027957},
      {"\260\335",0.0015404},
      {"\253\334",0.000797},
      {"\247\332",0.0034646},
      {"\301`",0.0018397},
      {"\252p",0.0005979},
      {"\263\370",0.0027135},
      {"\273P",0.0019506},
      {"\247@",0.0040521},
      {"\254O",0.0076122},
      {"\274w",0.0005846},
      {"\250\372",0.0009919},
      {"\250D",0.0009944},
      {"\252\361",0.0009063},
      {"\261\346",0.0005027},
      {"\253D",0.000531},
      {"\301p",0.0009379},
      {"\300u",0.0008197},
      {"\255\354",0.0009897},
      {"\247\353",0.0011592},
      {"\262\304",0.0017413},
      {"\250\256",0.0010894},
      {"\265L",0.0010303},
      {"\305@",0.0005659},
      {"\247\354",0.0004875},
      {"\263Q",0.000825},
      {"\253\366",0.0004965},
      {"\263q",0.0015692},
      {"\251\312",0.0011996},
      {"\256v",0.00052},
      {"\244\306",0.0023838},
      {"\260\352",0.0106651},
      {"\246\355",0.000485},
      {"\260\306",0.0006302},
      {"\251w",0.0024269},
      {"\256u",0.0004695},
      {"\263\241",0.0033026},
      {"\274W",0.0014225},
      {"\253\371",0.0010342},
      {"\250\306",0.0018604},
      {"\271L",0.0020606},
      {"\260\355",0.0004694},
      {"\303\321",0.0005749},
      {"\267\355",0.001657},
      {"\250\374",0.0008222},
      {"\270\323",0.0006501},
      {"\272\353",0.0007159},
      {"\253\374",0.0007731},
      {"\266V",0.0004797},
      {"\254\241",0.0011783},
      {"\252e",0.0005451},
      {"\305\334",0.0007406},
      {"\271D",0.0012275},
      {"\267\307",0.0007058},
      {"\261M",0.0009373},
      {"\244\275",0.0031331},
      {"\252L",0.0007135},
      {"\250\307",0.0013308},
      {"\252\300",0.0020607},
      {"\303\376",0.0005014},
      {"\267\275",0.0005666},
      {"\277\263",0.0005636},
      {"\273{",0.0008819},
      {"\246o",0.0005373},
      {"\266O",0.0009271},
      {"\275\350",0.0009775},
      {"\256i",0.0030534},
      {"\261\370",0.0012831},
      {"\244\345",0.0018717},
      {"\251M",0.0067636},
      {"\260\310",0.0018869},
      {"\266T",0.0006242},
      {"\255\314",0.0030767},
      {"\256\321",0.001001},
      {"\263d",0.0006355},
      {"\257q",0.0008483},
      {"\250\310",0.0006162},
      {"\271\352",0.002744},
      {"\275u",0.0005442},
      {"\253\310",0.0004833},
      {"\264I",0.0004872},
      {"\261`",0.0008196},
      {"\254\354",0.0014751},
      {"\270`",0.000558},
      {"\300\263",0.0014049},
      {"\254d",0.000721},
      {"\302\264",0.0007599},
      {"\261q",0.001703},
      {"\276P",0.0008187},
      {"\252\371",0.0014059},
      {"\270q",0.0011046},
      {"\305\347",0.0005062},
      {"\266q",0.0015425},
      {"\277n",0.0006935},
      {"\252\314",0.0022472},
      {"\261\241",0.0011166},
      {"\270\373",0.0005458},
      {"\265o",0.0053412},
      {"\247\275",0.0009665},
      {"\254\335",0.0008716},
      {"\252\355",0.0013294},
      {"\266\241",0.0010701},
      {"\266H",0.0004946},
      {"\247\365",0.0006669},
      {"\255\310",0.0006514},
      {"\256\346",0.0008735},
      {"\261\332",0.0005358},
      {"\277\357",0.0006163},
      {"\301\311",0.0004837},
      {"\271\322",0.0005699},
      {"\247\345",0.0007277},
      {"\276i",0.0004942},
      {"\304\263",0.0011071},
      {"\251\361",0.0010846},
      {"\254\366",0.0004974},
      {"\252\275",0.0005522},
      {"\301\331",0.0014906},
      {"\263\306",0.0006344},
      {"\263\314",0.0012694},
      {"\252\276",0.0006257},
      {"\266\325",0.0006866},
      {"\263\320",0.0007723},
      {"\261a",0.0006134},
      {"\254\371",0.000501},
      {"\303\370",0.0007756},
      {"\257\340",0.0022237},
      {"\251\324",0.0005527},
      {"\300Y",0.0007873},
      {"\254\374",0.0013899},
      {"\254\311",0.0010474},
      {"\275\315",0.0006043},
      {"\256\360",0.0005779},
      {"\251l",0.0004855},
      {"\270\334",0.000621},
      {"\261\300",0.0006574},
      {"\270g",0.0049223},
      {"\254I",0.0009187},
      {"\306[",0.0007736},
      {"\254\331",0.0016366},
      {"\263\365",0.0030636},
      {"\274\313",0.0008296},
      {"\251e",0.001456},
      {"\256\327",0.0005503},
      {"\263\322",0.0005726},
      {"\273\362",0.0006158},
      {"\273\241",0.0019478},
      {"\276\343",0.0006271},
      {"\263o",0.0043446},
      {"\256\370",0.0006543},
      {"\270\313",0.000643},
      {"\304Y",0.0006039},
      {"\251\372",0.0014369},
      {"\273\332",0.0012275},
      {"\300\362",0.0004915},
      {"\274\306",0.0008388},
      {"\261\304",0.0006097},
      {"\274\320",0.0009393},
      {"\251\366",0.0006749},
      {"\277\354",0.0013224},
      {"\270\352",0.0027448},
      {"\266}",0.0031058},
      {"\270\314",0.0014032},
      {"\261\320",0.0013183},
      {"\266m",0.0007924},
      {"\275\327",0.0007584},
      {"\275T",0.0006259},
      {"\304\362",0.0005217},
      {"\256\332",0.000545},
      {"\266\307",0.0007021},
      {"\274\322",0.0004864},
      {"\303\300",0.0004998},
      {"\273\335",0.0007022},
      {"\273D",0.0004727},
      {"\303D",0.0013688},
      {"\261o",0.0019729},
      {"\256a",0.0039648},
      {"\266\310",0.0005794},
      {"\276\367",0.0023189},
      {"\303\366",0.0023261},
      {"\251\322",0.0015791},
      {"\273\371",0.0010855},
      {"\276\332",0.0009205},
      {"\275\325",0.0012018},
      {"\300\347",0.0011203},
      {"\276\324",0.0008112},
      {"\256\304",0.0009217},
      {"\273\365",0.000824},
      {"\300\331",0.0026701},
      {"\256\306",0.0005776},
      {"\302\340",0.0007108},
      {"\256\374",0.0015815},
      {"\256\311",0.0030057},
      {"\276\372",0.0005352},
      {"\304\322",0.0011413},
      {"\302\355",0.000663},
      {"\302I",0.0013538},
      {"\276\311",0.0011932},
      {"\276\307",0.0027139},
      {"\302\276",0.000867},
      {"\302\345",0.0005415}
    };

  static const short lookup[] =
    {
        0,   1,  -1,   2,   3,   4,   5,  -1,   6,   7,
        8,   9,  -1,  10,  11,  -1,  12,  -1,  13,  14,
       15,  16,  -1,  17,  18,  19,  20,  -1,  21,  22,
       23,  24,  -1,  25,  26,  27,  28,  -1,  29,  30,
       31,  32,  -1,  33,  34,  35,  36,  -1,  37,  38,
       -1,  39,  -1,  40,  41,  42,  43,  -1,  44,  45,
       46,  47,  -1,  48,  49,  50,  51,  -1,  52,  53,
       54,  55,  56,  57,  58,  59,  60,  -1,  61,  62,
       63,  64,  -1,  65,  66,  67,  68,  -1,  69,  70,
       71,  72,  -1,  73,  74,  75,  76,  -1,  77,  78,
       79,  80,  -1,  81,  82,  83,  -1,  -1,  84,  85,
       86,  87,  -1,  88,  89,  90,  91,  -1,  92,  93,
       94,  95,  -1,  96,  97,  98,  99,  -1, 100, 101,
      102, 103,  -1, 104, 105, 106, 107,  -1, 108, 109,
      110, 111,  -1, 112, 113, 114, 115,  -1, 116, 117,
      118, 119,  -1, 120, 121, 122, 123,  -1, 124, 125,
      126, 127,  -1, 128, 129, 130, 131,  -1, 132, 133,
      134, 135,  -1, 136, 137, 138, 139,  -1, 140, 141,
      142, 143,  -1, 144, 145, 146, 147,  -1, 148, 149,
      150, 151,  -1, 152, 153, 154, 155,  -1, 156, 157,
      158, 159,  -1, 160, 161, 162, 163,  -1, 164, 165,
      166, 167,  -1, 168, 169, 170, 171,  -1, 172, 173,
      174, 175,  -1, 176, 177, 178, 179,  -1, 180,  -1,
      181, 182,  -1, 183, 184, 185, 186,  -1, 187,  -1,
      188, 189,  -1, 190,  -1, 191, 192,  -1, 193, 194,
      195, 196,  -1, 197, 198, 199, 200,  -1, 201, 202,
      203, 204,  -1, 205,  -1, 206, 207,  -1, 208, 209,
      210, 211,  -1, 212,  -1, 213, 214,  -1, 215, 216,
      217, 218,  -1, 219, 220, 221, 222,  -1, 223,  -1,
      224, 225,  -1, 226,  -1, 227, 228,  -1, 229, 230,
      231, 232,  -1, 233, 234, 235, 236,  -1, 237,  -1,
      238, 239,  -1, 240,  -1, 241, 242,  -1, 243,  -1,
      244, 245,  -1, 246, 247, 248, 249,  -1, 250,  -1,
      251, 252,  -1, 253, 254, 255, 256,  -1, 257, 258,
      259, 260,  -1, 261,  -1, 262, 263,  -1, 264,  -1,
      265, 266,  -1, 267,  -1, 268, 269, 270, 271, 272,
      273, 274,  -1, 275,  -1, 276, 277,  -1, 278,  -1,
      279, 280,  -1, 281,  -1, 282, 283,  -1, 284,  -1,
      285, 286,  -1, 287,  -1, 288, 289,  -1, 290,  -1,
      291, 292,  -1, 293,  -1, 294, 295,  -1, 296,  -1,
      297, 298,  -1, 299,  -1, 300, 301,  -1, 302,  -1,
      303, 304,  -1, 305,  -1, 306, 307,  -1, 308,  -1,
      309, 310,  -1, 311,  -1, 312, 313,  -1, 314,  -1,
      315, 316,  -1, 317,  -1, 318, 319,  -1, 320,  -1,
      321, 322,  -1, 323,  -1, 324, 325,  -1, 326,  -1,
      327, 328,  -1, 329,  -1, 330, 331,  -1, 332,  -1,
      333, 334,  -1, 335,  -1, 336, 337,  -1, 338,  -1,
      339, 340,  -1, 341,  -1, 342, 343,  -1, 344,  -1,
      345, 346,  -1, 347, 348, 349, 350,  -1, 351,  -1,
      352, 353,  -1, 354,  -1, 355, 356,  -1, 357,  -1,
      358, 359,  -1, 360,  -1, 361, 362,  -1, 363,  -1,
      364, 365,  -1, 366,  -1, 367, 368,  -1, 369,  -1,
      370, 371,  -1, 372,  -1, 373, 374,  -1, 375,  -1,
      376, 377,  -1, 378,  -1, 379, 380,  -1, 381,  -1,
      382,  -1,  -1, 383,  -1, 384, 385,  -1, 386,  -1,
      387, 388,  -1, 389,  -1, 390,  -1,  -1, 391,  -1,
      392, 393,  -1, 394,  -1, 395, 396,  -1, 397,  -1,
      398, 399,  -1, 400,  -1, 401, 402,  -1, 403,  -1,
      404,  -1,  -1, 405,  -1, 406, 407,  -1, 408,  -1,
      409, 410,  -1, 411,  -1, 412, 413,  -1, 414,  -1,
      415,  -1,  -1, 416,  -1, 417,  -1,  -1, 418,  -1,
      419,  -1,  -1, 420,  -1, 421, 422,  -1, 423,  -1,
      424,  -1,  -1, 425,  -1, 426,  -1,  -1,  -1,  -1,
      427,  -1,  -1, 428,  -1, 429,  -1,  -1, 430,  -1,
      431, 432,  -1, 433,  -1, 434, 435,  -1, 436, 437,
      438,  -1,  -1, 439,  -1, 440, 441,  -1, 442,  -1,
      443,  -1,  -1, 444,  -1, 445, 446,  -1, 447,  -1,
      448,  -1,  -1,  -1,  -1, 449,  -1,  -1,  -1,  -1,
      450,  -1,  -1, 451,  -1, 452,  -1,  -1, 453,  -1,
      454,  -1,  -1,  -1,  -1, 455,  -1,  -1,  -1,  -1,
      456,  -1,  -1, 457,  -1, 458, 459,  -1,  -1,  -1,
      460,  -1,  -1, 461,  -1, 462,  -1,  -1,  -1,  -1,
      463,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 464,  -1,
       -1,  -1,  -1,  -1,  -1,  -1, 465,  -1, 466,  -1,
      467,  -1,  -1, 468,  -1, 469,  -1,  -1,  -1,  -1,
      470, 471,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      472,  -1,  -1,  -1,  -1, 473,  -1,  -1, 474,  -1,
       -1, 475,  -1,  -1,  -1, 476,  -1,  -1,  -1,  -1,
      477,  -1,  -1, 478,  -1, 479,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 480,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      481,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 482,  -1,
       -1,  -1,  -1, 483,  -1, 484,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
      485,  -1,  -1, 486,  -1, 487,  -1,  -1,  -1,  -1,
      488,  -1,  -1,  -1,  -1, 489,  -1,  -1, 490,  -1,
      491,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1, 492,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1, 493,  -1,  -1, 494,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 495,  -1,
      496,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 497,  -1,
       -1,  -1,  -1,  -1,  -1, 498,  -1,  -1,  -1,  -1,
       -1,  -1,  -1,  -1,  -1,  -1, 499
    };

    register int key = hash_big5 (str);

    if (key <= MAX_HASH_VALUE && key >= 0) {
        register int index = lookup[key];

        if (index >= 0) {
            register const unsigned char *s = wordlist[index].name;

            if (*str == *s && str[1] == s[1])
                return &wordlist[index];
            }
        }
    return 0;
}

/**
 * is_big5:
 * @str: Pointer to a 16-bit wide character.
 *
 * Returns: One if the wide character is within Big5 charset range, zero otherwise
 **/

static int is_big5(const unsigned char *str)
{
  unsigned char high = str[0];
  unsigned char low = str[1];

  return high >=0xa1 && high <= 0xf9
         && ((low >= 0x40 && low <= 0x7e)
             || (low >= 0xa1 && low <= 0xfe));
}
