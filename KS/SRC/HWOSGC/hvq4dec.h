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
 * File:	hvq4dec.h
 *
 * (C) 2000-2001 HUDSON SOFT
 */

#ifndef __HVQ4DEC_H__
#define __HVQ4DEC_H__

#include <HVQ4File.h>

/****************************************************************************
  TYPE DEFINES
 ****************************************************************************/

void HVQ4InitDecoder(void);
u32  HVQ4BuffSize(HVQ4Header *hvq4ptr);
u16  HVQ4GetWidth(HVQ4Header *hvq4ptr);
u16  HVQ4GetHeight(HVQ4Header *hvq4ptr);
void HVQ4Decode(HVQ4Header *hvq4ptr, void *outbuf, u16 *workbuf);

#endif /* __HVQ4DEC_H__ */

/* end */
