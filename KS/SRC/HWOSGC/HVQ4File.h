/**********************************************************************
ご注意

1. 株式会社ハドソンが提供する本媒体に含まれる、技術資料、図面、規格、コ
   ンピュータ・ソフトウェア、アルゴリズム及びその他の情報（以下、併せて
   「本秘密情報」といいます）の著作権、知的所有権その他の権利は、株式会
   社ハドソンまたは株式会社ハドソンに使用を許諾した者に帰属します。
2. 本秘密情報は、株式会社ハドソンが許諾する目的以外の目的で使用、複製す
   ることができません。
3. 本秘密情報を、第三者へ開示することはできません。
4. 本秘密情報を、改変して使用することはできません。
5. 株式会社ハドソンは、本秘密情報の使用により生じたいかなる損害に関して
   も、一切責任を負わないものとします。

Notices

1. Copyrights, any other intellectual property right and other rights 
   to and in the technical material, charts, specifications computer 
   software, algorithm and any other information (hereinafter called 
   "the Confidential Information") included in the media which is 
   provided by Hudson Soft Co., Ltd. shall belong to Hudson Soft Co., 
   Ltd. or its licensors.
2. No part of the Confidential Information may be reproducedor used 
   for any purpose other than that licensed by Hudson Soft Co., Ltd.
3. No part of the Confidential Information may be disclosed to any 
   third party.
4. No part of the Confidential Information may be revised in  order to 
   use it.
5. In no case shall Hudson Soft Co., Ltd. be held responsible for any 
   loss or damage arising from the use of the Confidential Information.
**********************************************************************/

/*
 * File:	HVQ4File.h (HVQ4 1.0A)
 *
 * (C) 2000-2001 HUDSON SOFT
 */

#ifndef __HVQ4FILE_H__
#define __HVQ4FILE_H__

#include <HVQmd.h>

#define  HVQ4_FILEVERSION	"HVQ4 1.0A"

/*****************************************************************************
 * TYPE DEFINES (HVQ4)
 *****************************************************************************/
typedef struct _tagHVQ4Header {
  char file_version[16];	/* File version */
  u32 filesize;				/* Size of file */
  u32 headersize;			/* Size of HVQ4 Header */
  u8  h_sampling_rate;		/* Color difference horizontalsubsampling interval */
  u8  v_sampling_rate;		/* Color difference verticalsubsampling interval */
  u16 width;				/* Image width in pixels */
  u16 height;				/* Image height in pixels */
  u16 nest_start_x;
  u16 nest_start_y;
  u8  scl_quantize_shift;
  u8  dcv_quantize_shift;
  u32 sp_packets;
  u32 basisnum_offset[2];
  u32 basisrun_offset[2];
  u32 dcval_offset[3];
  u32 dcrun_offset[3];
  u32 scale_offset[3];
  u32 aotcd_offset[3];
} HVQ4Header;

#endif  /* __HVQ4FILE_H__ */

/* end */
