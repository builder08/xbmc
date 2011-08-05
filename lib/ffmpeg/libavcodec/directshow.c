/* 
 *  Copyright (C) 2006-2009 mplayerc
 *  Copyright (C) 2010 Team XBMC
 *  http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */



/*#include "avcodec.h"
#include "h264.h"
#include "vc1.h"

#undef NDEBUG
#include <assert.h>
*/

#include "directshow_internal.h"
#include "directshow.h"
#include "h264.h"
#include "h264data.h"
/*int FFH264BuildPicParams (DXVA_PicParams_H264 *pDXVAPicParams, DXVA_Qmatrix_H264 *pDXVAScalingMatrix, int *nFieldType, int *nSliceType, struct AVCodecContext *pAVCtx, int nPCIVendor)*/

const byte ZZ_SCAN[16]  =
{  0,  1,  4,  8,  5,  2,  3,  6,  9, 12, 13, 10,  7, 11, 14, 15
};

const byte ZZ_SCAN8[64] =
{  0,  1,  8, 16,  9,  2,  3, 10, 17, 24, 32, 25, 18, 11,  4,  5,
   12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13,  6,  7, 14, 21, 28,
   35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
   58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static void fill_picentry(DXVA_PicEntry_H264 *pic,
                               unsigned index, unsigned flag)
{
    assert((index&0x7f) == index && (flag&0x01) == flag);
    pic->bPicEntry = index | (flag << 7);
}

void ff_directshow_h264_setpoc(MpegEncContext *s, int poc, int64_t start)
{
	directshow_dxva_h264 *pict;
	pict = (directshow_dxva_h264 *)s->current_picture_ptr->data[0];
	pict->frame_poc = poc;
	pict->frame_start = start;
}
void ff_directshow_h264_fill_slice_long(MpegEncContext *s)
{
	H264Context *h = s->avctx->priv_data;
	directshow_dxva_h264 *pict;
	int						field_pic_flag;
	unsigned int			i,j,k;
	DXVA_Slice_H264_Long*	pSlice;
	if (!s->current_picture_ptr) {
	    av_log(s->avctx, AV_LOG_DEBUG, "current_picture_ptr is null!\n");
	    return;
	}
	pict = (directshow_dxva_h264 *)s->current_picture_ptr->data[0];
	assert(pict);
	
	pSlice = &((DXVA_Slice_H264_Long*) pict->slice_long)[h->current_slice-1];
	memset(pSlice, 0, sizeof(*pSlice));
    av_log(s->avctx, AV_LOG_DEBUG, "ff_directshow_h264_fill_slice_long!\n");
	field_pic_flag = (h->s.picture_structure != PICT_FRAME);

	pSlice->slice_id						= h->current_slice-1;
	pSlice->first_mb_in_slice				= (s->mb_y >> FIELD_OR_MBAFF_PICTURE) * s->mb_width + s->mb_x;/*= h->first_mb_in_slice;*/
	pSlice->NumMbsForSlice					= 0; // h->s.mb_num;				// TODO : to be checked !
	pSlice->BitOffsetToSliceData			= get_bits_count(&s->gb) + 8;
	pSlice->slice_type						= ff_h264_get_slice_type(h);/*h->raw_slice_type; */
	if (h->slice_type_fixed)
        pSlice->slice_type += 5;
	pSlice->luma_log2_weight_denom			= h->luma_log2_weight_denom;
	pSlice->chroma_log2_weight_denom		= h->chroma_log2_weight_denom;
	pSlice->num_ref_idx_l0_active_minus1	= h->ref_count[0]-1;	// num_ref_idx_l0_active_minus1;
	pSlice->num_ref_idx_l1_active_minus1	= h->ref_count[1]-1;	// num_ref_idx_l1_active_minus1;
	pSlice->slice_alpha_c0_offset_div2		= h->slice_alpha_c0_offset / 2;
	pSlice->slice_beta_offset_div2			= h->slice_beta_offset / 2;
	pSlice->Reserved8Bits					= 0;
	
	// Fill prediction weights
	memset (pSlice->Weights, 0, sizeof(pSlice->Weights));
	for(i=0; i<2; i++){
		for(j=0; j<h->ref_count[i]; j++){
			//         L0&L1          Y,Cb,Cr  Weight,Offset
			// Weights  [2]    [32]     [3]         [2]
			pSlice->Weights[i][j][0][0] = h->luma_weight[j][i][0];
			pSlice->Weights[i][j][0][1] = h->luma_weight[j][i][1];

			for(k=0; k<2; k++){
				pSlice->Weights[i][j][k+1][0] = h->chroma_weight[j][i][k][0];
				pSlice->Weights[i][j][k+1][1] = h->chroma_weight[j][i][k][1];
			}
		}
	}

	pSlice->slice_qs_delta    = 0; /* XXX not implemented by FFmpeg */
    pSlice->slice_qp_delta    = s->qscale - h->pps.init_qp;
	pSlice->redundant_pic_cnt				= h->redundant_pic_count;
	pSlice->direct_spatial_mv_pred_flag		= h->direct_spatial_mv_pred;
	pSlice->cabac_init_idc					= h->cabac_init_idc;
	pSlice->disable_deblocking_filter_idc	= h->deblocking_filter;

	for(i=0; i<32; i++)
	{ 
	  /*DXVA_PicEntry_H264*/
	  fill_picentry(&pSlice->RefPicList[0][i],127,1);
	  fill_picentry(&pSlice->RefPicList[1][i],127,1);
	  /*pSlice->RefPicList[0][i].AssociatedFlag = 1;
	  pSlice->RefPicList[0][i].bPicEntry = 255; 
	  pSlice->RefPicList[0][i].Index7Bits = 127;
	  pSlice->RefPicList[1][i].AssociatedFlag = 1; 
	  pSlice->RefPicList[1][i].bPicEntry = 255;
	  pSlice->RefPicList[1][i].Index7Bits = 127;*/
	}
}

void ff_directshow_h264_picture_complete(MpegEncContext *s)
{
/*this function is based on FFH264BuildPicParams and this is done at the end of decoding sequence*/
/*updating the ref frame slice long*/
    H264Context *h = s->avctx->priv_data;
	directshow_dxva_h264 *pict;
    unsigned int            i,j;

	int field_pic_flag;
	SPS* cur_sps;
    PPS* cur_pps;
	DXVA_Qmatrix_H264* qmatrix_source;
	
	pict = (directshow_dxva_h264 *)s->current_picture_ptr->data[0];
	assert(pict);
	
    av_log(s->avctx, AV_LOG_DEBUG, "ff_directshow_h264_picture_complete!\n");
    field_pic_flag = (h->s.picture_structure != PICT_FRAME);

    cur_sps = &h->sps;
    cur_pps = &h->pps;
    if (cur_sps && cur_pps)
    {
        pict->field_type = h->s.picture_structure;
        if (h->sps.pic_struct_present_flag)
        {
		    if ( h->sei_pic_struct == SEI_PIC_STRUCT_TOP_FIELD || h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM || h->sei_pic_struct == SEI_PIC_STRUCT_TOP_BOTTOM_TOP )
				pict->field_type = PICT_TOP_FIELD;
			else 
			if ( h->sei_pic_struct == SEI_PIC_STRUCT_BOTTOM_FIELD || h->sei_pic_struct == SEI_PIC_STRUCT_BOTTOM_TOP || h->sei_pic_struct == SEI_PIC_STRUCT_BOTTOM_TOP_BOTTOM )
			    pict->field_type = PICT_BOTTOM_FIELD;
		    else
			if ( h->sei_pic_struct == SEI_PIC_STRUCT_FRAME_DOUBLING || h->sei_pic_struct == SEI_PIC_STRUCT_FRAME_TRIPLING || h->sei_pic_struct == SEI_PIC_STRUCT_FRAME )
				pict->field_type = PICT_FRAME;
        }

        pict->slice_type = h->slice_type;

        if (cur_sps->mb_width==0 || cur_sps->mb_height==0) 
		    return;
        pict->picture_params.wFrameWidthInMbsMinus1            = s->mb_width  - 1;        // pic_width_in_mbs_minus1;
        pict->picture_params.wFrameHeightInMbsMinus1           = s->mb_height - 1;// pic_height_in_map_units_minus1;
		/*cur_sps->mb_height * (2 - cur_sps->frame_mbs_only_flag) - 1;     <--- this is one is better for wFrameHeightInMbsMinus1?*/        
        pict->picture_params.num_ref_frames                    = cur_sps->ref_frame_count;        // num_ref_frames;
        /* DXVA_PicParams_H264 */
        pict->picture_params.wBitFields                        = ((s->picture_structure != PICT_FRAME)    <<  0) | /*field_pic_flag*/
															     (h->sps.mb_aff                          <<  1) | /*MbaffFrameFlag*/
																 (cur_sps->residual_color_transform_flag <<  2) | /*residual_colour_transform_flag*/
																 (0                                      <<  3) | /*sp_for_switch_flag*/
																  (cur_sps->chroma_format_idc            <<  4) | /*(2) chroma_format_idc*/
																  ((h->nal_ref_idc != 0)                 <<  6) | /*RefPicFlag */
																  (cur_pps->constrained_intra_pred       <<  7) | /*constrained_intra_pred_flag*/
																  (cur_pps->weighted_pred                <<  8) | /*weighted_pred_flag*/
																  (cur_pps->weighted_bipred_idc          <<  9) | /*(2) weighted_bipred_idc*/
																  (1                                     << 11) | /*MbsConsecutiveFlag*/
																  (cur_sps->frame_mbs_only_flag          << 12) | /*frame_mbs_only_flag*/
																  (cur_pps->transform_8x8_mode           << 13) | /*transform_8x8_mode_flag*/
																  (1                                     << 14) | /*MinLumaBipredSize8x8Flag*/
																  /*(h->sps.level_idc >= 31                << 14) | it seem that setting it to 1 improve performance*/
															  /* IntraPicFlag (Modified if we detect a non
															   * intra slice in decode_slice) Only used with ffmpeg dxva2*/
															  ((h->slice_type == FF_I_TYPE )                                    << 15);

        pict->picture_params.bit_depth_luma_minus8            = cur_sps->bit_depth_luma   - 8;    // bit_depth_luma_minus8
        pict->picture_params.bit_depth_chroma_minus8            = cur_sps->bit_depth_chroma - 8;    // bit_depth_chroma_minus8
        pict->picture_params.frame_num                        = h->frame_num;
		
        pict->picture_params.log2_max_frame_num_minus4                = cur_sps->log2_max_frame_num - 4;                    // log2_max_frame_num_minus4;
        pict->picture_params.pic_order_cnt_type                        = cur_sps->poc_type;                                // pic_order_cnt_type;
        pict->picture_params.log2_max_pic_order_cnt_lsb_minus4        = cur_sps->log2_max_poc_lsb - 4;                    // log2_max_pic_order_cnt_lsb_minus4;
        pict->picture_params.delta_pic_order_always_zero_flag        = cur_sps->delta_pic_order_always_zero_flag;
        pict->picture_params.direct_8x8_inference_flag                = cur_sps->direct_8x8_inference_flag;
        pict->picture_params.entropy_coding_mode_flag                = cur_pps->cabac;                                    // entropy_coding_mode_flag;
        pict->picture_params.pic_order_present_flag                    = cur_pps->pic_order_present;                        // pic_order_present_flag;
        pict->picture_params.num_slice_groups_minus1                    = cur_pps->slice_group_count - 1;                    // num_slice_groups_minus1;
        pict->picture_params.slice_group_map_type                    = cur_pps->mb_slice_group_map_type;                    // slice_group_map_type;
        pict->picture_params.deblocking_filter_control_present_flag    = cur_pps->deblocking_filter_parameters_present;    // deblocking_filter_control_present_flag;
        pict->picture_params.redundant_pic_cnt_present_flag            = cur_pps->redundant_pic_cnt_present;                // redundant_pic_cnt_present_flag;

        pict->picture_params.slice_group_change_rate_minus1          = 0;  /* XXX not implemented by FFmpeg */

        pict->picture_params.chroma_qp_index_offset                  = cur_pps->chroma_qp_index_offset[0];
        pict->picture_params.second_chroma_qp_index_offset           = cur_pps->chroma_qp_index_offset[1];
		pict->picture_params.ContinuationFlag                        = 1;
        pict->picture_params.num_ref_idx_l0_active_minus1            = cur_pps->ref_count[0]-1;                            // num_ref_idx_l0_active_minus1;
        pict->picture_params.num_ref_idx_l1_active_minus1            = cur_pps->ref_count[1]-1;                            // num_ref_idx_l1_active_minus1;
        pict->picture_params.pic_init_qp_minus26                        = cur_pps->init_qp - 26;
        pict->picture_params.pic_init_qs_minus26                        = cur_pps->init_qs - 26;

        if (field_pic_flag)
        {
          unsigned cur_associated_flag = (h->s.picture_structure == PICT_BOTTOM_FIELD);
			pict->picture_params.CurrPic.bPicEntry = 0 | (cur_associated_flag << 7) ;

            if (cur_associated_flag){
                // Bottom field
                pict->picture_params.CurrFieldOrderCnt[0] = 0;
                pict->picture_params.CurrFieldOrderCnt[1] = h->poc_lsb + h->poc_msb;
            }
			else
			{
                // Top field
                pict->picture_params.CurrFieldOrderCnt[0] = h->poc_lsb + h->poc_msb;
                pict->picture_params.CurrFieldOrderCnt[1] = 0;
            }
        }
        else
        {
			pict->picture_params.CurrPic.bPicEntry = 0 | (0 << 7);
            pict->picture_params.CurrFieldOrderCnt[0]    = h->poc_lsb + h->poc_msb;
            pict->picture_params.CurrFieldOrderCnt[1]    = h->poc_lsb + h->poc_msb;
        }

        /*CopyScalingMatrix (pict->picture_qmatrix, (DXVA_Qmatrix_H264*)cur_pps->scaling_matrix4, nPCIVendor);*/
		/*TODO add the copy to handle something else than nvidia video card*/
		qmatrix_source = (DXVA_Qmatrix_H264*)cur_pps->scaling_matrix4;
        
		for (i=0; i<6; i++)
            for (j=0; j<16; j++)
                pict->picture_qmatrix.bScalingLists4x4[i][j] = qmatrix_source->bScalingLists4x4[i][ZZ_SCAN[j]];

        for (i=0; i<2; i++)
            for (j=0; j<64; j++)
                pict->picture_qmatrix.bScalingLists8x8[i][j] = qmatrix_source->bScalingLists8x8[i][ZZ_SCAN8[j]];
    }
	
	/* setting the index of the surface give by the getbuffer*/
	pict->picture_params.CurrPic.bPicEntry = pict->decoder_surface_index << 0;
    h->s.current_picture_ptr->opaque = (void*)pict->decoder_surface_index;
}

void ff_directshow_h264_set_reference_frames(MpegEncContext *s)
{
    H264Context *h = s->avctx->priv_data;
	directshow_dxva_h264 *pict;
	UINT            nUsedForReferenceFlags = 0;
    int                i;
    Picture*        pic;
    UCHAR            AssociatedFlag;
	av_log(s->avctx, AV_LOG_DEBUG, "ff_directshow_h264_set_reference_frames!\n");
    pict = (directshow_dxva_h264 *)s->current_picture_ptr->data[0];
    for(i=0; i<16; i++)
    {
        if (i < h->short_ref_count)
        {
            // Short list reference frames
            pic                = h->short_ref[h->short_ref_count - i - 1];
            AssociatedFlag    = pic->long_ref != 0;
        }
        else if (i >= h->short_ref_count && i < h->long_ref_count)
        {
            // Long list reference frames
            pic            = h->short_ref[h->short_ref_count + h->long_ref_count - i - 1];
            AssociatedFlag    = 1;
        }
        else
            pic = NULL;


        if (pic != NULL)
        {
            pict->picture_params.FrameNumList[i]    = pic->long_ref ? pic->pic_id : pic->frame_num;

            if (pic->field_poc[0] != INT_MAX)
            {
                pict->picture_params.FieldOrderCntList[i][0]        = pic->field_poc [0];
                nUsedForReferenceFlags                       |= 1<<(i*2);
            }
            else
                pict->picture_params.FieldOrderCntList[i][0]        = 0;

            if (pic->field_poc[1] != INT_MAX)
            {
                pict->picture_params.FieldOrderCntList[i][1]        = pic->field_poc [1];
                nUsedForReferenceFlags                       |= 2<<(i*2);
            }
            else
            {
                pict->picture_params.FieldOrderCntList[i][1]        = 0;
            }

            pict->picture_params.RefFrameList[i].bPicEntry       = (unsigned)pic->opaque | (AssociatedFlag << 7);

        }
        else
        {
            pict->picture_params.FrameNumList[i]                    = 0;
            pict->picture_params.FieldOrderCntList[i][0]            = 0;
            pict->picture_params.FieldOrderCntList[i][1]            = 0;
            pict->picture_params.RefFrameList[i].bPicEntry       = 127 | (1 << 7);

        }
    }

    pict->picture_params.UsedForReferenceFlags    = nUsedForReferenceFlags;
}

void ff_directshow_h264_picture_start(MpegEncContext *s)
{
/* is there something to do here ?*/
    /*H264Context *h = s->avctx->priv_data;
	struct directshow_dxva_h264 *pict;
	int i;
	int pictureindex;*/
	
	//pict = ( struct directshow_dxva_h264 *)s->current_picture_ptr->data[0];
	//assert(pict);
	//pictureindex = pict->decoder_surface_index;
    
	
    av_log(s->avctx, AV_LOG_DEBUG, "ff_directshow_h264_picture_start!\n");
}
