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
 * File:	hvqm4dec.h
 *
 * (C) 2000-2001 HUDSON SOFT
 */

#ifndef __HVQM4DEC_H__
#define __HVQM4DEC_H__

#include <HVQM4File.h>

/****************************************************************************
  HVQM4SeqObj
 ****************************************************************************/
typedef struct {
  void *ws;
  u16 frame_width;		/* Image width in pixels */
  u16 frame_height;		/* Image height in pixels */
  u8  h_samp;			/* Color difference horizontalsubsampling interval */
  u8  v_samp;			/* Color difference verticalsubsampling interval */
} HVQM4SeqObj;

/****************************************************************************
  FUNCTION PROTOTYPES
 ****************************************************************************/

void HVQM4InitDecoder(void);
void HVQM4InitSeqObj(HVQM4SeqObj *obj, HVQM4VideoInfo *header);
u32 HVQM4BuffSize(HVQM4SeqObj *obj);
void HVQM4SetBuffer(HVQM4SeqObj *obj, void *buf);
void HVQM4DecodeIpic(HVQM4SeqObj *obj, void *code, void *outbuf);
void HVQM4DecodePpic(HVQM4SeqObj *obj, void *code, void *outbuf, void *ref1);
void HVQM4DecodeBpic(HVQM4SeqObj *obj, void *code, void *outbuf, void *ref2, void *ref1);

/****************************************************************************
  MACRO
 ****************************************************************************/

#define HVQM4GetWidth(obj)  ((obj)->frame_width)
#define HVQM4GetHeight(obj) ((obj)->frame_height)

#define HVQM4GetSamplingRateH(obj) ((obj)->h_samp) /* ALWAYS 2 for GAMECUBE */
#define HVQM4GetSamplingRateV(obj) ((obj)->v_samp) /* ALWAYS 2 for GAMECUBE */

#endif  /* __HVQM4DEC_H__ */

/* end */

