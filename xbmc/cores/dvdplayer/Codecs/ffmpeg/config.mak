# Automatically generated by configure - do not modify!
ifndef FFMPEG_CONFIG_MAK
FFMPEG_CONFIG_MAK=1
FFMPEG_CONFIGURATION=--extra-cflags='-D_XBOX -fno-common' --cpu=pentium3 --enable-shared --enable-memalign-hack --enable-gpl --enable-w32threads --enable-postproc --enable-zlib --disable-static --disable-altivec --disable-muxers --disable-encoders --disable-indevs --disable-ipv6 --disable-debug
prefix=/usr/local
LIBDIR=$(DESTDIR)${prefix}/lib
SHLIBDIR=$(DESTDIR)${prefix}/bin
INCDIR=$(DESTDIR)${prefix}/include
BINDIR=$(DESTDIR)${prefix}/bin
DATADIR=$(DESTDIR)${prefix}/share/ffmpeg
MANDIR=$(DESTDIR)${prefix}/share/man
SRC_PATH="/d/dev/xbmc/xbox-branch4merge/xbmc/cores/dvdplayer/Codecs/ffmpeg"
SRC_PATH_BARE=/d/dev/xbmc/xbox-branch4merge/xbmc/cores/dvdplayer/Codecs/ffmpeg
BUILD_ROOT="/d/dev/xbmc/xbox-branch4merge/xbmc/cores/dvdplayer/Codecs/ffmpeg"
ARCH=x86
CC=gcc
AS=gcc
LD=gcc
DEPCC=gcc
YASM=yasm
AR=ar
RANLIB=ranlib
LN_S=ln -sf
STRIP=strip
CPPFLAGS= -D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE
CFLAGS= -D_XBOX -fno-common -march=pentium3 -std=c99 -fno-common -fomit-frame-pointer -Wdeclaration-after-statement -Wall -Wno-switch -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wno-pointer-sign -Wcast-qual -Wwrite-strings -Wundef -O3 -fno-math-errno -fno-tree-vectorize
ASFLAGS= -D_XBOX -fno-common -march=pentium3
CC_O=-o $@
LDFLAGS=  -Wl,--warn-common -Wl,--as-needed -Wl,-rpath-link,$(BUILD_ROOT)/libpostproc -Wl,-rpath-link,$(BUILD_ROOT)/libswscale -Wl,-rpath-link,$(BUILD_ROOT)/libavfilter -Wl,-rpath-link,$(BUILD_ROOT)/libavdevice -Wl,-rpath-link,$(BUILD_ROOT)/libavformat -Wl,-rpath-link,$(BUILD_ROOT)/libavcodec -Wl,-rpath-link,$(BUILD_ROOT)/libavutil -Wl,-Bsymbolic
FFSERVERLDFLAGS=-Wl,-E
SHFLAGS=-shared -Wl,--output-def,$$(@:$(SLIBSUF)=.def) -Wl,--out-implib,$(SUBDIR)lib$(SLIBNAME:$(SLIBSUF)=.dll.a) -Wl,--enable-runtime-pseudo-reloc -Wl,--enable-auto-image-base
YASMFLAGS=-f win32 -DARCH_X86_32 -DPREFIX
BUILDSUF=
FULLNAME=$(NAME)$(BUILDSUF)
LIBPREF=lib
LIBSUF=.a
LIBNAME=
SLIBPREF=
SLIBSUF=.dll
EXESUF=.exe
EXTRA_VERSION=
DEPFLAGS=$(CPPFLAGS) $(CFLAGS) -MM
CCDEP=
ASDEP=
CC_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
AS_DEPFLAGS=-MMD -MF $(@:.o=.d) -MT $@
HOSTCC=gcc
HOSTCFLAGS=-D_ISOC99_SOURCE -D_POSIX_C_SOURCE=200112 -O3 -g -Wall -std=c99
HOSTEXESUF=.exe
HOSTLDFLAGS=
HOSTLIBS=-lm
TARGET_EXEC=
TARGET_PATH=.
SDL_LIBS=
SDL_CFLAGS=
LIB_INSTALL_EXTRA_CMD=$(RANLIB) "$(LIBDIR)/$(LIBNAME)"
EXTRALIBS=  -lz -lm -lws2_32
libswscale_VERSION=0.7.2
libswscale_VERSION_MAJOR=0
libpostproc_VERSION=51.2.0
libpostproc_VERSION_MAJOR=51
libavcodec_VERSION=52.43.0
libavcodec_VERSION_MAJOR=52
libavdevice_VERSION=52.2.0
libavdevice_VERSION_MAJOR=52
libavformat_VERSION=52.44.0
libavformat_VERSION_MAJOR=52
libavutil_VERSION=50.7.0
libavutil_VERSION_MAJOR=50
libavfilter_VERSION=1.12.0
libavfilter_VERSION_MAJOR=1
LIBTARGET=i386
SLIBNAME=$(SLIBPREF)$(FULLNAME)$(SLIBSUF)
SLIBNAME_WITH_VERSION=$(SLIBPREF)$(FULLNAME)-$(LIBVERSION)$(SLIBSUF)
SLIBNAME_WITH_MAJOR=$(SLIBPREF)$(FULLNAME)-$(LIBMAJOR)$(SLIBSUF)
SLIB_CREATE_DEF_CMD=
SLIB_EXTRA_CMD=-lib.exe /machine:$(LIBTARGET) /def:$$(@:$(SLIBSUF)=.def) /out:$(SUBDIR)$(SLIBNAME_WITH_MAJOR:$(SLIBSUF)=.lib)
SLIB_INSTALL_EXTRA_CMD=-install -m 644 $(SUBDIR)$(SLIBNAME_WITH_MAJOR:$(SLIBSUF)=.lib) "$(SHLIBDIR)/$(SLIBNAME:$(SLIBSUF)=.lib)"; \
            install -m 644 $(SUBDIR)$(SLIBNAME_WITH_MAJOR:$(SLIBSUF)=.lib) "$(SHLIBDIR)/$(SLIBNAME_WITH_MAJOR:$(SLIBSUF)=.lib)"; \
            install -d "$(LIBDIR)"; \
            install -m 644 $(SUBDIR)lib$(SLIBNAME:$(SLIBSUF)=.dll.a) "$(LIBDIR)/lib$(SLIBNAME:$(SLIBSUF)=.dll.a)"
SLIB_UNINSTALL_EXTRA_CMD=rm -f "$(SHLIBDIR)/$(SLIBNAME:$(SLIBSUF)=.lib)"
ARCH_X86=yes
ARCH_X86_32=yes
HAVE_AMD3DNOW=yes
HAVE_AMD3DNOWEXT=yes
HAVE_MMX=yes
HAVE_MMX2=yes
HAVE_SSE=yes
HAVE_SSSE3=yes
HAVE_W32THREADS=yes
HAVE_ATTRIBUTE_PACKED=yes
HAVE_BSWAP=yes
HAVE_CLOSESOCKET=yes
HAVE_CMOV=yes
HAVE_CONIO_H=yes
HAVE_DLFCN_H=yes
HAVE_DLOPEN=yes
HAVE_DOS_PATHS=yes
HAVE_EBP_AVAILABLE=yes
HAVE_EBX_AVAILABLE=yes
HAVE_FAST_CMOV=yes
HAVE_FAST_UNALIGNED=yes
HAVE_GETPROCESSTIMES=yes
HAVE_INLINE_ASM=yes
HAVE_ISATTY=yes
HAVE_LLRINT=yes
HAVE_LOG2=yes
HAVE_LRINT=yes
HAVE_LRINTF=yes
HAVE_MALLOC_H=yes
HAVE_ROUND=yes
HAVE_ROUNDF=yes
HAVE_SETMODE=yes
HAVE_SOCKLEN_T=yes
HAVE_TEN_OPERANDS=yes
HAVE_THREADS=yes
HAVE_TRUNCF=yes
HAVE_VIRTUALALLOC=yes
HAVE_WINSOCK2_H=yes
CONFIG_BSFS=yes
CONFIG_DECODERS=yes
CONFIG_DEMUXERS=yes
CONFIG_FILTERS=yes
CONFIG_HWACCELS=yes
CONFIG_OUTDEVS=yes
CONFIG_PROTOCOLS=yes
CONFIG_AANDCT=yes
CONFIG_FASTDIV=yes
CONFIG_FFMPEG=yes
CONFIG_FFT=yes
CONFIG_GOLOMB=yes
CONFIG_GPL=yes
CONFIG_MDCT=yes
CONFIG_MEMALIGN_HACK=yes
CONFIG_MPEGAUDIO_HP=yes
CONFIG_NETWORK=yes
CONFIG_POSTPROC=yes
CONFIG_RDFT=yes
CONFIG_SHARED=yes
CONFIG_SWSCALE_ALPHA=yes
CONFIG_ZLIB=yes
CONFIG_AASC_DECODER=yes
CONFIG_AMV_DECODER=yes
CONFIG_ASV1_DECODER=yes
CONFIG_ASV2_DECODER=yes
CONFIG_AVS_DECODER=yes
CONFIG_BETHSOFTVID_DECODER=yes
CONFIG_BFI_DECODER=yes
CONFIG_BMP_DECODER=yes
CONFIG_C93_DECODER=yes
CONFIG_CAVS_DECODER=yes
CONFIG_CDGRAPHICS_DECODER=yes
CONFIG_CINEPAK_DECODER=yes
CONFIG_CLJR_DECODER=yes
CONFIG_CSCD_DECODER=yes
CONFIG_CYUV_DECODER=yes
CONFIG_DNXHD_DECODER=yes
CONFIG_DPX_DECODER=yes
CONFIG_DSICINVIDEO_DECODER=yes
CONFIG_DVVIDEO_DECODER=yes
CONFIG_DXA_DECODER=yes
CONFIG_EACMV_DECODER=yes
CONFIG_EAMAD_DECODER=yes
CONFIG_EATGQ_DECODER=yes
CONFIG_EATGV_DECODER=yes
CONFIG_EATQI_DECODER=yes
CONFIG_EIGHTBPS_DECODER=yes
CONFIG_EIGHTSVX_EXP_DECODER=yes
CONFIG_EIGHTSVX_FIB_DECODER=yes
CONFIG_ESCAPE124_DECODER=yes
CONFIG_FFV1_DECODER=yes
CONFIG_FFVHUFF_DECODER=yes
CONFIG_FLASHSV_DECODER=yes
CONFIG_FLIC_DECODER=yes
CONFIG_FLV_DECODER=yes
CONFIG_FOURXM_DECODER=yes
CONFIG_FRAPS_DECODER=yes
CONFIG_FRWU_DECODER=yes
CONFIG_GIF_DECODER=yes
CONFIG_H261_DECODER=yes
CONFIG_H263_DECODER=yes
CONFIG_H263I_DECODER=yes
CONFIG_H264_DECODER=yes
CONFIG_HUFFYUV_DECODER=yes
CONFIG_IDCIN_DECODER=yes
CONFIG_INDEO2_DECODER=yes
CONFIG_INDEO3_DECODER=yes
CONFIG_INTERPLAY_VIDEO_DECODER=yes
CONFIG_JPEGLS_DECODER=yes
CONFIG_KMVC_DECODER=yes
CONFIG_LOCO_DECODER=yes
CONFIG_MDEC_DECODER=yes
CONFIG_MIMIC_DECODER=yes
CONFIG_MJPEG_DECODER=yes
CONFIG_MJPEGB_DECODER=yes
CONFIG_MMVIDEO_DECODER=yes
CONFIG_MOTIONPIXELS_DECODER=yes
CONFIG_MPEG1VIDEO_DECODER=yes
CONFIG_MPEG2VIDEO_DECODER=yes
CONFIG_MPEG4_DECODER=yes
CONFIG_MPEGVIDEO_DECODER=yes
CONFIG_MSMPEG4V1_DECODER=yes
CONFIG_MSMPEG4V2_DECODER=yes
CONFIG_MSMPEG4V3_DECODER=yes
CONFIG_MSRLE_DECODER=yes
CONFIG_MSVIDEO1_DECODER=yes
CONFIG_MSZH_DECODER=yes
CONFIG_NUV_DECODER=yes
CONFIG_PAM_DECODER=yes
CONFIG_PBM_DECODER=yes
CONFIG_PCX_DECODER=yes
CONFIG_PGM_DECODER=yes
CONFIG_PGMYUV_DECODER=yes
CONFIG_PNG_DECODER=yes
CONFIG_PPM_DECODER=yes
CONFIG_PTX_DECODER=yes
CONFIG_QDRAW_DECODER=yes
CONFIG_QPEG_DECODER=yes
CONFIG_QTRLE_DECODER=yes
CONFIG_R210_DECODER=yes
CONFIG_RAWVIDEO_DECODER=yes
CONFIG_RL2_DECODER=yes
CONFIG_ROQ_DECODER=yes
CONFIG_RPZA_DECODER=yes
CONFIG_RV10_DECODER=yes
CONFIG_RV20_DECODER=yes
CONFIG_RV30_DECODER=yes
CONFIG_RV40_DECODER=yes
CONFIG_SGI_DECODER=yes
CONFIG_SMACKER_DECODER=yes
CONFIG_SMC_DECODER=yes
CONFIG_SNOW_DECODER=yes
CONFIG_SP5X_DECODER=yes
CONFIG_SUNRAST_DECODER=yes
CONFIG_SVQ1_DECODER=yes
CONFIG_SVQ3_DECODER=yes
CONFIG_TARGA_DECODER=yes
CONFIG_THEORA_DECODER=yes
CONFIG_THP_DECODER=yes
CONFIG_TIERTEXSEQVIDEO_DECODER=yes
CONFIG_TIFF_DECODER=yes
CONFIG_TMV_DECODER=yes
CONFIG_TRUEMOTION1_DECODER=yes
CONFIG_TRUEMOTION2_DECODER=yes
CONFIG_TSCC_DECODER=yes
CONFIG_TXD_DECODER=yes
CONFIG_ULTI_DECODER=yes
CONFIG_V210_DECODER=yes
CONFIG_V210X_DECODER=yes
CONFIG_VB_DECODER=yes
CONFIG_VC1_DECODER=yes
CONFIG_VCR1_DECODER=yes
CONFIG_VMDVIDEO_DECODER=yes
CONFIG_VMNC_DECODER=yes
CONFIG_VP3_DECODER=yes
CONFIG_VP5_DECODER=yes
CONFIG_VP6_DECODER=yes
CONFIG_VP6A_DECODER=yes
CONFIG_VP6F_DECODER=yes
CONFIG_VQA_DECODER=yes
CONFIG_WMV1_DECODER=yes
CONFIG_WMV2_DECODER=yes
CONFIG_WMV3_DECODER=yes
CONFIG_WNV1_DECODER=yes
CONFIG_XAN_WC3_DECODER=yes
CONFIG_XL_DECODER=yes
CONFIG_ZLIB_DECODER=yes
CONFIG_ZMBV_DECODER=yes
CONFIG_AAC_DECODER=yes
CONFIG_AC3_DECODER=yes
CONFIG_ALAC_DECODER=yes
CONFIG_ALS_DECODER=yes
CONFIG_APE_DECODER=yes
CONFIG_ATRAC1_DECODER=yes
CONFIG_ATRAC3_DECODER=yes
CONFIG_COOK_DECODER=yes
CONFIG_DCA_DECODER=yes
CONFIG_DSICINAUDIO_DECODER=yes
CONFIG_EAC3_DECODER=yes
CONFIG_FLAC_DECODER=yes
CONFIG_IMC_DECODER=yes
CONFIG_MACE3_DECODER=yes
CONFIG_MACE6_DECODER=yes
CONFIG_MLP_DECODER=yes
CONFIG_MP1_DECODER=yes
CONFIG_MP2_DECODER=yes
CONFIG_MP3_DECODER=yes
CONFIG_MP3ADU_DECODER=yes
CONFIG_MP3ON4_DECODER=yes
CONFIG_MPC7_DECODER=yes
CONFIG_MPC8_DECODER=yes
CONFIG_NELLYMOSER_DECODER=yes
CONFIG_QCELP_DECODER=yes
CONFIG_QDM2_DECODER=yes
CONFIG_RA_144_DECODER=yes
CONFIG_RA_288_DECODER=yes
CONFIG_SHORTEN_DECODER=yes
CONFIG_SMACKAUD_DECODER=yes
CONFIG_SONIC_DECODER=yes
CONFIG_TRUEHD_DECODER=yes
CONFIG_TRUESPEECH_DECODER=yes
CONFIG_TTA_DECODER=yes
CONFIG_TWINVQ_DECODER=yes
CONFIG_VMDAUDIO_DECODER=yes
CONFIG_VORBIS_DECODER=yes
CONFIG_WAVPACK_DECODER=yes
CONFIG_WMAPRO_DECODER=yes
CONFIG_WMAV1_DECODER=yes
CONFIG_WMAV2_DECODER=yes
CONFIG_WS_SND1_DECODER=yes
CONFIG_PCM_ALAW_DECODER=yes
CONFIG_PCM_BLURAY_DECODER=yes
CONFIG_PCM_DVD_DECODER=yes
CONFIG_PCM_F32BE_DECODER=yes
CONFIG_PCM_F32LE_DECODER=yes
CONFIG_PCM_F64BE_DECODER=yes
CONFIG_PCM_F64LE_DECODER=yes
CONFIG_PCM_MULAW_DECODER=yes
CONFIG_PCM_S8_DECODER=yes
CONFIG_PCM_S16BE_DECODER=yes
CONFIG_PCM_S16LE_DECODER=yes
CONFIG_PCM_S16LE_PLANAR_DECODER=yes
CONFIG_PCM_S24BE_DECODER=yes
CONFIG_PCM_S24DAUD_DECODER=yes
CONFIG_PCM_S24LE_DECODER=yes
CONFIG_PCM_S32BE_DECODER=yes
CONFIG_PCM_S32LE_DECODER=yes
CONFIG_PCM_U8_DECODER=yes
CONFIG_PCM_U16BE_DECODER=yes
CONFIG_PCM_U16LE_DECODER=yes
CONFIG_PCM_U24BE_DECODER=yes
CONFIG_PCM_U24LE_DECODER=yes
CONFIG_PCM_U32BE_DECODER=yes
CONFIG_PCM_U32LE_DECODER=yes
CONFIG_PCM_ZORK_DECODER=yes
CONFIG_INTERPLAY_DPCM_DECODER=yes
CONFIG_ROQ_DPCM_DECODER=yes
CONFIG_SOL_DPCM_DECODER=yes
CONFIG_XAN_DPCM_DECODER=yes
CONFIG_ADPCM_4XM_DECODER=yes
CONFIG_ADPCM_ADX_DECODER=yes
CONFIG_ADPCM_CT_DECODER=yes
CONFIG_ADPCM_EA_DECODER=yes
CONFIG_ADPCM_EA_MAXIS_XA_DECODER=yes
CONFIG_ADPCM_EA_R1_DECODER=yes
CONFIG_ADPCM_EA_R2_DECODER=yes
CONFIG_ADPCM_EA_R3_DECODER=yes
CONFIG_ADPCM_EA_XAS_DECODER=yes
CONFIG_ADPCM_G726_DECODER=yes
CONFIG_ADPCM_IMA_AMV_DECODER=yes
CONFIG_ADPCM_IMA_DK3_DECODER=yes
CONFIG_ADPCM_IMA_DK4_DECODER=yes
CONFIG_ADPCM_IMA_EA_EACS_DECODER=yes
CONFIG_ADPCM_IMA_EA_SEAD_DECODER=yes
CONFIG_ADPCM_IMA_ISS_DECODER=yes
CONFIG_ADPCM_IMA_QT_DECODER=yes
CONFIG_ADPCM_IMA_SMJPEG_DECODER=yes
CONFIG_ADPCM_IMA_WAV_DECODER=yes
CONFIG_ADPCM_IMA_WS_DECODER=yes
CONFIG_ADPCM_MS_DECODER=yes
CONFIG_ADPCM_SBPRO_2_DECODER=yes
CONFIG_ADPCM_SBPRO_3_DECODER=yes
CONFIG_ADPCM_SBPRO_4_DECODER=yes
CONFIG_ADPCM_SWF_DECODER=yes
CONFIG_ADPCM_THP_DECODER=yes
CONFIG_ADPCM_XA_DECODER=yes
CONFIG_ADPCM_YAMAHA_DECODER=yes
CONFIG_DVBSUB_DECODER=yes
CONFIG_DVDSUB_DECODER=yes
CONFIG_PGSSUB_DECODER=yes
CONFIG_XSUB_DECODER=yes
CONFIG_AAC_PARSER=yes
CONFIG_AC3_PARSER=yes
CONFIG_CAVSVIDEO_PARSER=yes
CONFIG_DCA_PARSER=yes
CONFIG_DIRAC_PARSER=yes
CONFIG_DNXHD_PARSER=yes
CONFIG_DVBSUB_PARSER=yes
CONFIG_DVDSUB_PARSER=yes
CONFIG_H261_PARSER=yes
CONFIG_H263_PARSER=yes
CONFIG_H264_PARSER=yes
CONFIG_MJPEG_PARSER=yes
CONFIG_MLP_PARSER=yes
CONFIG_MPEG4VIDEO_PARSER=yes
CONFIG_MPEGAUDIO_PARSER=yes
CONFIG_MPEGVIDEO_PARSER=yes
CONFIG_PNM_PARSER=yes
CONFIG_VC1_PARSER=yes
CONFIG_VP3_PARSER=yes
CONFIG_AAC_ADTSTOASC_BSF=yes
CONFIG_DUMP_EXTRADATA_BSF=yes
CONFIG_H264_MP4TOANNEXB_BSF=yes
CONFIG_IMX_DUMP_HEADER_BSF=yes
CONFIG_MJPEGA_DUMP_HEADER_BSF=yes
CONFIG_MP3_HEADER_COMPRESS_BSF=yes
CONFIG_MP3_HEADER_DECOMPRESS_BSF=yes
CONFIG_MOV2TEXTSUB_BSF=yes
CONFIG_NOISE_BSF=yes
CONFIG_REMOVE_EXTRADATA_BSF=yes
CONFIG_TEXT2MOVSUB_BSF=yes
CONFIG_AAC_DEMUXER=yes
CONFIG_AC3_DEMUXER=yes
CONFIG_AEA_DEMUXER=yes
CONFIG_AIFF_DEMUXER=yes
CONFIG_AMR_DEMUXER=yes
CONFIG_APC_DEMUXER=yes
CONFIG_APE_DEMUXER=yes
CONFIG_ASF_DEMUXER=yes
CONFIG_ASS_DEMUXER=yes
CONFIG_AU_DEMUXER=yes
CONFIG_AVI_DEMUXER=yes
CONFIG_AVS_DEMUXER=yes
CONFIG_BETHSOFTVID_DEMUXER=yes
CONFIG_BFI_DEMUXER=yes
CONFIG_C93_DEMUXER=yes
CONFIG_CAF_DEMUXER=yes
CONFIG_CAVSVIDEO_DEMUXER=yes
CONFIG_CDG_DEMUXER=yes
CONFIG_DAUD_DEMUXER=yes
CONFIG_DIRAC_DEMUXER=yes
CONFIG_DNXHD_DEMUXER=yes
CONFIG_DSICIN_DEMUXER=yes
CONFIG_DTS_DEMUXER=yes
CONFIG_DV_DEMUXER=yes
CONFIG_DXA_DEMUXER=yes
CONFIG_EA_DEMUXER=yes
CONFIG_EA_CDATA_DEMUXER=yes
CONFIG_EAC3_DEMUXER=yes
CONFIG_FFM_DEMUXER=yes
CONFIG_FLAC_DEMUXER=yes
CONFIG_FLIC_DEMUXER=yes
CONFIG_FLV_DEMUXER=yes
CONFIG_FOURXM_DEMUXER=yes
CONFIG_GSM_DEMUXER=yes
CONFIG_GXF_DEMUXER=yes
CONFIG_H261_DEMUXER=yes
CONFIG_H263_DEMUXER=yes
CONFIG_H264_DEMUXER=yes
CONFIG_IDCIN_DEMUXER=yes
CONFIG_IFF_DEMUXER=yes
CONFIG_IMAGE2_DEMUXER=yes
CONFIG_IMAGE2PIPE_DEMUXER=yes
CONFIG_INGENIENT_DEMUXER=yes
CONFIG_IPMOVIE_DEMUXER=yes
CONFIG_ISS_DEMUXER=yes
CONFIG_IV8_DEMUXER=yes
CONFIG_LMLM4_DEMUXER=yes
CONFIG_M4V_DEMUXER=yes
CONFIG_MATROSKA_DEMUXER=yes
CONFIG_MJPEG_DEMUXER=yes
CONFIG_MLP_DEMUXER=yes
CONFIG_MM_DEMUXER=yes
CONFIG_MMF_DEMUXER=yes
CONFIG_MOV_DEMUXER=yes
CONFIG_MP3_DEMUXER=yes
CONFIG_MPC_DEMUXER=yes
CONFIG_MPC8_DEMUXER=yes
CONFIG_MPEGPS_DEMUXER=yes
CONFIG_MPEGTS_DEMUXER=yes
CONFIG_MPEGTSRAW_DEMUXER=yes
CONFIG_MPEGVIDEO_DEMUXER=yes
CONFIG_MSNWC_TCP_DEMUXER=yes
CONFIG_MTV_DEMUXER=yes
CONFIG_MVI_DEMUXER=yes
CONFIG_MXF_DEMUXER=yes
CONFIG_NC_DEMUXER=yes
CONFIG_NSV_DEMUXER=yes
CONFIG_NUT_DEMUXER=yes
CONFIG_NUV_DEMUXER=yes
CONFIG_OGG_DEMUXER=yes
CONFIG_OMA_DEMUXER=yes
CONFIG_PCM_ALAW_DEMUXER=yes
CONFIG_PCM_MULAW_DEMUXER=yes
CONFIG_PCM_F64BE_DEMUXER=yes
CONFIG_PCM_F64LE_DEMUXER=yes
CONFIG_PCM_F32BE_DEMUXER=yes
CONFIG_PCM_F32LE_DEMUXER=yes
CONFIG_PCM_S32BE_DEMUXER=yes
CONFIG_PCM_S32LE_DEMUXER=yes
CONFIG_PCM_S24BE_DEMUXER=yes
CONFIG_PCM_S24LE_DEMUXER=yes
CONFIG_PCM_S16BE_DEMUXER=yes
CONFIG_PCM_S16LE_DEMUXER=yes
CONFIG_PCM_S8_DEMUXER=yes
CONFIG_PCM_U32BE_DEMUXER=yes
CONFIG_PCM_U32LE_DEMUXER=yes
CONFIG_PCM_U24BE_DEMUXER=yes
CONFIG_PCM_U24LE_DEMUXER=yes
CONFIG_PCM_U16BE_DEMUXER=yes
CONFIG_PCM_U16LE_DEMUXER=yes
CONFIG_PCM_U8_DEMUXER=yes
CONFIG_PVA_DEMUXER=yes
CONFIG_QCP_DEMUXER=yes
CONFIG_R3D_DEMUXER=yes
CONFIG_RAWVIDEO_DEMUXER=yes
CONFIG_RL2_DEMUXER=yes
CONFIG_RM_DEMUXER=yes
CONFIG_ROQ_DEMUXER=yes
CONFIG_RPL_DEMUXER=yes
CONFIG_RTSP_DEMUXER=yes
CONFIG_SDP_DEMUXER=yes
CONFIG_SEGAFILM_DEMUXER=yes
CONFIG_SHORTEN_DEMUXER=yes
CONFIG_SIFF_DEMUXER=yes
CONFIG_SMACKER_DEMUXER=yes
CONFIG_SOL_DEMUXER=yes
CONFIG_SOX_DEMUXER=yes
CONFIG_STR_DEMUXER=yes
CONFIG_SWF_DEMUXER=yes
CONFIG_THP_DEMUXER=yes
CONFIG_TIERTEXSEQ_DEMUXER=yes
CONFIG_TMV_DEMUXER=yes
CONFIG_TRUEHD_DEMUXER=yes
CONFIG_TTA_DEMUXER=yes
CONFIG_TXD_DEMUXER=yes
CONFIG_VC1_DEMUXER=yes
CONFIG_VC1T_DEMUXER=yes
CONFIG_VMD_DEMUXER=yes
CONFIG_VOC_DEMUXER=yes
CONFIG_VQF_DEMUXER=yes
CONFIG_W64_DEMUXER=yes
CONFIG_WAV_DEMUXER=yes
CONFIG_WC3_DEMUXER=yes
CONFIG_WSAUD_DEMUXER=yes
CONFIG_WSVQA_DEMUXER=yes
CONFIG_WV_DEMUXER=yes
CONFIG_XA_DEMUXER=yes
CONFIG_YUV4MPEGPIPE_DEMUXER=yes
CONFIG_CROP_FILTER=yes
CONFIG_FORMAT_FILTER=yes
CONFIG_NOFORMAT_FILTER=yes
CONFIG_NULL_FILTER=yes
CONFIG_SCALE_FILTER=yes
CONFIG_SLICIFY_FILTER=yes
CONFIG_VFLIP_FILTER=yes
CONFIG_FILE_PROTOCOL=yes
CONFIG_GOPHER_PROTOCOL=yes
CONFIG_HTTP_PROTOCOL=yes
CONFIG_PIPE_PROTOCOL=yes
CONFIG_RTMP_PROTOCOL=yes
CONFIG_RTP_PROTOCOL=yes
CONFIG_TCP_PROTOCOL=yes
CONFIG_UDP_PROTOCOL=yes
endif # FFMPEG_CONFIG_MAK
