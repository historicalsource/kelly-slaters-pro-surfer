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
 * File:	HVQM4File.h (HVQM4 1.3)
 *
 * (C) 2000-2001 HUDSON SOFT
 */

#ifndef __HVQM4FILE_H__
#define __HVQM4FILE_H__

#include <HVQmd.h>

#define  HVQM4_FILEVERSION	"HVQM4 1.3"

/****************************************************************************
  HVQM4VideoInfo
 ****************************************************************************/
typedef struct {
  u16  width;				/* Image width in pixels */
  u16  height;				/* Image height in pixels */
  u8   h_sampling_rate;		/* Color difference horizontalsubsampling interval */
  u8   v_sampling_rate;		/* Color difference verticalsubsampling interval */
  u8   pad[2];				/* for 32-bit alignment */
} HVQM4VideoInfo;

/****************************************************************************
  HVQM4AudioInfo
 ****************************************************************************/
typedef struct {
  u8  channels;				/* Number of audio channels */
  u8  sample_bits;			/* Number of bits per sample for one channel */
  u8  audio_quantize;		/* Audio quantization step */
  u8  dummy;
  u32 samples_per_sec;		/* Number of audio samples per second */
} HVQM4AudioInfo;

/****************************************************************************
  HVQMovieHeader : File information
 ****************************************************************************/
typedef struct {
  u8   file_version[16];	/* File version */

  u32  hdr_size;			/* Size of HVQMovieHeader [byte] */
  u32  dat_size;			/* HVQM data size [byte] */

  u32  gop_total;			/* Total count of GOPs */
  u32  video_total;			/* Total count of video records */
  u32  audio_total;			/* Total count of audio records */

  u32  usec_per_frame;		/* Image frame interval [usec] */
  u32  max_frame_size;		/* Maximum video record size */
  u32  max_sp_packets;		/* Maximum packet count required for SP FIFO */
  u32  max_audio_record_size;	/* Maximum audio record size (excluding header) */
  
  HVQM4VideoInfo  videoinfo;	/* HVQM4 file header */
  HVQM4AudioInfo  audioinfo;	/* ADPCM file header */
} HVQM4Header;

/****************************************************************************
  HVQMGOPHeader : GOP header 
 ****************************************************************************/
typedef struct {
  u32  gop_offset;		/* Offset from immediately preceding GOP [bytes] */
  u32  size;			/* Size of GOP [bytes] (excluding this header) */
  u32  vidrec_number;	/* Number of video records in GOP */
  u32  audrec_number;	/* Number of audio records in GOP */
  u8   flags;			/* Flags specifying GOP type */
  u8   pad[3];			/* Padding data (to 32-bit boundary) */
} HVQM4GOPHeader;

/* GOP type */
#define  HVQM_GOP_CLOSED  0x01	/* Closed link */
#define  HVQM_GOP_BROKEN  0x02	/* Broken link */

/****************************************************************************
  HVQMRecHeader : Record header
 ****************************************************************************/
typedef struct {
  u16  type;			/* Record type(audio record or video record) */
  u16  format;			/* Data format (depends on record type) */
  u32  size;			/* Record size [bytes] (excluding this header) */
} HVQM4RecHeader;

/* Record type */
#define  HVQM4_AUDIO  0
#define  HVQM4_VIDEO  1

/* Audio record */
#define  HVQM4_AUDIO_KEY      0x01
#define  HVQM4_AUDIO_PREDICT  0x02

/* Video record */
#define  HVQM4_VIDEO_I_PIC    0x10 /* I pictures */
#define  HVQM4_VIDEO_P_PIC    0x20 /* P pictures */
#define  HVQM4_VIDEO_B_PIC    0x30 /* B pictures */

/****************************************************************************
  HVQMAudioHeader : Audio header: only audio records present
 ****************************************************************************/
typedef struct {
  u32  samples;			/* Number of samples */
} HVQM4AudioHeader;

/****************************************************************************
  HVQMVideoHeader : Video header: only video records present
 ****************************************************************************/
typedef struct {
  u32  frame_number;		/* Rendering sequence within GOP */
} HVQM4VideoHeader;

#endif  /* __HVQM4FILE_H__ */
