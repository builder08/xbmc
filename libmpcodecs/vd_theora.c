#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>

#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"

#ifdef HAVE_OGGTHEORA

#include "vd_internal.h"

static vd_info_t info = {
   "Theora/VP3",
   "theora",
   "David Kuehling",
   "www.theora.org",
   "Theora project's VP3 codec"
};

LIBVD_EXTERN(theora)

#include <theora/theora.h>

#define THEORA_NUM_HEADER_PACKETS 3

// to set/get/query special features/parameters
static int control(sh_video_t *sh,int cmd,void* arg,...){
    return CONTROL_UNKNOWN;
}

typedef struct theora_struct_st {
    theora_state st;
    theora_comment cc;
    theora_info inf;
} theora_struct_t;

/*
 * init driver
 */
static int init(sh_video_t *sh){
    theora_struct_t *context = NULL;
    int failed = 1;
    int errorCode = 0;
    ogg_packet op;
    int i;

    /* check whether video output format is supported */
    switch(sh->codec->outfmt[sh->outfmtidx])
    {
       case IMGFMT_YV12: /* well, this should work... */ break;
       default: 
	  mp_msg (MSGT_DECVIDEO,MSGL_ERR,"Unsupported out_fmt: 0x%X\n",
		  sh->codec->outfmt[sh->outfmtidx]);
	  return 0;
    }

    /* this is not a loop, just a context, from which we can break on error */
    do
    {
       context = (theora_struct_t *)calloc (sizeof (theora_struct_t), 1);
       sh->context = context;
       if (!context)
	  break;

       theora_info_init(&context->inf);
       theora_comment_init(&context->cc);
       
       /* Read all header packets, pass them to theora_decode_header. */
       for (i = 0; i < THEORA_NUM_HEADER_PACKETS; i++)
       {
          op.bytes = ds_get_packet (sh->ds, &op.packet);
          op.b_o_s = 1;
          if ( (errorCode = theora_decode_header (&context->inf, &context->cc, &op)) )
          {
            mp_msg(MSGT_DECAUDIO, MSGL_ERR, "Broken Theora header; errorCode=%i!\n", errorCode);
            break;
          }
       }
       if (errorCode)
          break;

       /* now init codec */
       errorCode = theora_decode_init (&context->st, &context->inf);
       if (errorCode)
       {
	  mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Theora decode init failed: %i \n",
		 errorCode);
	  break;
       }
       failed = 0;
    } while (0);

    if (failed)
    {
       if (context)
       {
	  free (context);
	  sh->context = NULL;
       }
       return 0;
    }

    mp_msg(MSGT_DECVIDEO,MSGL_V,"INFO: Theora video init ok!\n");

    return mpcodecs_config_vo (sh,sh->disp_w,sh->disp_h,IMGFMT_YV12);
}

/* 
 * uninit driver
 */
static void uninit(sh_video_t *sh)
{
   theora_struct_t *context = (theora_struct_t *)sh->context;

   if (context)
   {
      theora_clear (&context->st);
      free (context);
   }
}

/*
 * decode frame
 */
static mp_image_t* decode(sh_video_t *sh,void* data,int len,int flags) 
{
   theora_struct_t *context = (theora_struct_t *)sh->context;
   int errorCode = 0;
   ogg_packet op;
   yuv_buffer yuv;
   mp_image_t* mpi;
   int i;

   bzero (&op, sizeof (op));
   op.bytes = len;
   op.packet = data;
   op.granulepos = -1;

   errorCode = theora_decode_packetin (&context->st, &op);
   if (errorCode)
   {
      mp_msg(MSGT_DECVIDEO,MSGL_ERR,"Theora decode packetin failed: %i \n",
	     errorCode);
      return NULL;
   }

   errorCode = theora_decode_YUVout (&context->st, &yuv);
   if (errorCode)
   {
      mp_msg(MSGT_DEMUX,MSGL_ERR,"Theora decode YUVout failed: %i \n",
	     errorCode);
      return 0;
   }

    mpi = mpcodecs_get_image(sh, MP_IMGTYPE_EXPORT, 0, sh->disp_w, sh->disp_h);
    if(!mpi) return NULL;
    
    mpi->planes[0]=yuv.y;
    mpi->stride[0]=yuv.y_stride;
    mpi->planes[1]=yuv.u;
    mpi->stride[1]=yuv.uv_stride;
    mpi->planes[2]=yuv.v;
    mpi->stride[2]=yuv.uv_stride;

    return mpi;
}

#endif
