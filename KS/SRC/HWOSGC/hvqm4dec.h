/**********************************************************************
������

1. ������Ѓn�h�\�����񋟂���{�}�̂Ɋ܂܂��A�Z�p�����A�}�ʁA�K�i�A�R
   ���s���[�^�E�\�t�g�E�F�A�A�A���S���Y���y�т��̑��̏��i�ȉ��A������
   �u�{�閧���v�Ƃ����܂��j�̒��쌠�A�m�I���L�����̑��̌����́A������
   �Ѓn�h�\���܂��͊�����Ѓn�h�\���Ɏg�p�����������҂ɋA�����܂��B
2. �{�閧���́A������Ѓn�h�\������������ړI�ȊO�̖ړI�Ŏg�p�A������
   �邱�Ƃ��ł��܂���B
3. �{�閧�����A��O�҂֊J�����邱�Ƃ͂ł��܂���B
4. �{�閧�����A���ς��Ďg�p���邱�Ƃ͂ł��܂���B
5. ������Ѓn�h�\���́A�{�閧���̎g�p�ɂ�萶���������Ȃ鑹�Q�Ɋւ���
   ���A��ؐӔC�𕉂�Ȃ����̂Ƃ��܂��B

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

