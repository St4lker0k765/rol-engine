/********************************************************************
 *                                                                  *
 * THIS FILE IS PART OF THE OggTheora SOFTWARE CODEC SOURCE CODE.   *
 * USE, DISTRIBUTION AND REPRODUCTION OF THIS LIBRARY SOURCE IS     *
 * GOVERNED BY A BSD-STYLE SOURCE LICENSE INCLUDED WITH THIS SOURCE *
 * IN 'COPYING'. PLEASE READ THESE TERMS BEFORE DISTRIBUTING.       *
 *                                                                  *
 * THE Theora SOURCE CODE IS COPYRIGHT (C) 2002-2009                *
 * by the Xiph.Org Foundation http://www.xiph.org/                  *
 *                                                                  *
 ********************************************************************

  function:
  last mod: $Id: encode.c 16503 2009-08-22 18:14:02Z giles $

 ********************************************************************/
#include <stdlib.h>
#include <string.h>
#include "encint.h"
#if defined(OC_X86_ASM)
# include "x86/x86enc.h"
#endif


#ifdef _MSC_VER
#pragma warning(disable:4389)
#pragma warning(disable:4018)
#pragma warning(disable:4244)
#pragma warning(disable:4100)
#endif

/*The default quantization parameters used by VP3.1.*/
static const int OC_VP31_RANGE_SIZES[1]={63};
static const th_quant_base OC_VP31_BASES_INTRA_Y[2]={
  {
     16, 11, 10, 16, 24, 40, 51, 61,
     12, 12, 14, 19, 26, 58, 60, 55,
     14, 13, 16, 24, 40, 57, 69, 56,
     14, 17, 22, 29, 51, 87, 80, 62,
     18, 22, 37, 58, 68,109,103, 77,
     24, 35, 55, 64, 81,104,113, 92,
     49, 64, 78, 87,103,121,120,101,
     72, 92, 95, 98,112,100,103, 99
  },
  {
     16, 11, 10, 16, 24, 40, 51, 61,
     12, 12, 14, 19, 26, 58, 60, 55,
     14, 13, 16, 24, 40, 57, 69, 56,
     14, 17, 22, 29, 51, 87, 80, 62,
     18, 22, 37, 58, 68,109,103, 77,
     24, 35, 55, 64, 81,104,113, 92,
     49, 64, 78, 87,103,121,120,101,
     72, 92, 95, 98,112,100,103, 99
  }
};
static const th_quant_base OC_VP31_BASES_INTRA_C[2]={
  {
     17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99
  },
  {
     17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99
  }
};
static const th_quant_base OC_VP31_BASES_INTER[2]={
  {
     16, 16, 16, 20, 24, 28, 32, 40,
     16, 16, 20, 24, 28, 32, 40, 48,
     16, 20, 24, 28, 32, 40, 48, 64,
     20, 24, 28, 32, 40, 48, 64, 64,
     24, 28, 32, 40, 48, 64, 64, 64,
     28, 32, 40, 48, 64, 64, 64, 96,
     32, 40, 48, 64, 64, 64, 96,128,
     40, 48, 64, 64, 64, 96,128,128
  },
  {
     16, 16, 16, 20, 24, 28, 32, 40,
     16, 16, 20, 24, 28, 32, 40, 48,
     16, 20, 24, 28, 32, 40, 48, 64,
     20, 24, 28, 32, 40, 48, 64, 64,
     24, 28, 32, 40, 48, 64, 64, 64,
     28, 32, 40, 48, 64, 64, 64, 96,
     32, 40, 48, 64, 64, 64, 96,128,
     40, 48, 64, 64, 64, 96,128,128
  }
};

const th_quant_info TH_VP31_QUANT_INFO={
  {
    220,200,190,180,170,170,160,160,
    150,150,140,140,130,130,120,120,
    110,110,100,100, 90, 90, 90, 80,
     80, 80, 70, 70, 70, 60, 60, 60,
     60, 50, 50, 50, 50, 40, 40, 40,
     40, 40, 30, 30, 30, 30, 30, 30,
     30, 20, 20, 20, 20, 20, 20, 20,
     20, 10, 10, 10, 10, 10, 10, 10
  },
  {
    500,450,400,370,340,310,285,265,
    245,225,210,195,185,180,170,160,
    150,145,135,130,125,115,110,107,
    100, 96, 93, 89, 85, 82, 75, 74,
     70, 68, 64, 60, 57, 56, 52, 50,
     49, 45, 44, 43, 40, 38, 37, 35,
     33, 32, 30, 29, 28, 25, 24, 22,
     21, 19, 18, 17, 15, 13, 12, 10
  },
  {
    30,25,20,20,15,15,14,14,
    13,13,12,12,11,11,10,10,
     9, 9, 8, 8, 7, 7, 7, 7,
     6, 6, 6, 6, 5, 5, 5, 5,
     4, 4, 4, 4, 3, 3, 3, 3,
     2, 2, 2, 2, 2, 2, 2, 2,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    {
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_Y},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_C},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTRA_C}
    },
    {
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER},
      {1,OC_VP31_RANGE_SIZES,OC_VP31_BASES_INTER}
    }
  }
};

/*The current default quantization parameters.*/
static const int OC_DEF_QRANGE_SIZES[3]={32,16,15};
static const th_quant_base OC_DEF_BASES_INTRA_Y[4]={
  {
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
     15, 15, 15, 15, 15, 15, 15, 15,
  },
  {
     15, 12, 12, 15, 18, 20, 20, 21,
     13, 13, 14, 17, 18, 21, 21, 20,
     14, 14, 15, 18, 20, 21, 21, 21,
     14, 16, 17, 19, 20, 21, 21, 21,
     16, 17, 20, 21, 21, 21, 21, 21,
     18, 19, 20, 21, 21, 21, 21, 21,
     20, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21
  },
  {
     16, 12, 11, 16, 20, 25, 27, 28,
     13, 13, 14, 18, 21, 28, 28, 27,
     14, 13, 16, 20, 25, 28, 28, 28,
     14, 16, 19, 22, 27, 29, 29, 28,
     17, 19, 25, 28, 28, 30, 30, 29,
     20, 24, 27, 28, 29, 30, 30, 29,
     27, 28, 29, 29, 30, 30, 30, 30,
     29, 29, 29, 29, 30, 30, 30, 29
  },
  {
     16, 11, 10, 16, 24, 40, 51, 61,
     12, 12, 14, 19, 26, 58, 60, 55,
     14, 13, 16, 24, 40, 57, 69, 56,
     14, 17, 22, 29, 51, 87, 80, 62,
     18, 22, 37, 58, 68,109,103, 77,
     24, 35, 55, 64, 81,104,113, 92,
     49, 64, 78, 87,103,121,120,101,
     72, 92, 95, 98,112,100,103, 99
  }
};
static const th_quant_base OC_DEF_BASES_INTRA_C[4]={
  {
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19,
     19, 19, 19, 19, 19, 19, 19, 19
  },
  {
     18, 18, 21, 25, 26, 26, 26, 26,
     18, 20, 22, 26, 26, 26, 26, 26,
     21, 22, 25, 26, 26, 26, 26, 26,
     25, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26,
     26, 26, 26, 26, 26, 26, 26, 26
  },
  {
     17, 18, 22, 31, 36, 36, 36, 36,
     18, 20, 24, 34, 36, 36, 36, 36,
     22, 24, 33, 36, 36, 36, 36, 36,
     31, 34, 36, 36, 36, 36, 36, 36,
     36, 36, 36, 36, 36, 36, 36, 36,
     36, 36, 36, 36, 36, 36, 36, 36,
     36, 36, 36, 36, 36, 36, 36, 36,
     36, 36, 36, 36, 36, 36, 36, 36
  },
  {
     17, 18, 24, 47, 99, 99, 99, 99,
     18, 21, 26, 66, 99, 99, 99, 99,
     24, 26, 56, 99, 99, 99, 99, 99,
     47, 66, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99,
     99, 99, 99, 99, 99, 99, 99, 99
  }
};
static const th_quant_base OC_DEF_BASES_INTER[4]={
  {
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21,
     21, 21, 21, 21, 21, 21, 21, 21
  },
  {
     18, 18, 18, 21, 23, 24, 25, 27,
     18, 18, 21, 23, 24, 25, 27, 28,
     18, 21, 23, 24, 25, 27, 28, 29,
     21, 23, 24, 25, 27, 28, 29, 29,
     23, 24, 25, 27, 28, 29, 29, 29,
     24, 25, 27, 28, 29, 29, 29, 30,
     25, 27, 28, 29, 29, 29, 30, 30,
     27, 28, 29, 29, 29, 30, 30, 30
  },
  {
     17, 17, 17, 20, 23, 26, 28, 32,
     17, 17, 20, 23, 26, 28, 32, 34,
     17, 20, 23, 26, 28, 32, 34, 37,
     20, 23, 26, 28, 32, 34, 37, 37,
     23, 26, 28, 32, 34, 37, 37, 37,
     26, 28, 32, 34, 37, 37, 37, 41,
     28, 32, 34, 37, 37, 37, 41, 42,
     32, 34, 37, 37, 37, 41, 42, 42
  },
  {
     16, 16, 16, 20, 24, 28, 32, 40,
     16, 16, 20, 24, 28, 32, 40, 48,
     16, 20, 24, 28, 32, 40, 48, 64,
     20, 24, 28, 32, 40, 48, 64, 64,
     24, 28, 32, 40, 48, 64, 64, 64,
     28, 32, 40, 48, 64, 64, 64, 96,
     32, 40, 48, 64, 64, 64, 96,128,
     40, 48, 64, 64, 64, 96,128,128
  }
};

const th_quant_info TH_DEF_QUANT_INFO={
  {
    365,348,333,316,300,287,277,265,
    252,240,229,219,206,197,189,180,
    171,168,160,153,146,139,132,127,
    121,115,110,107,101, 97, 94, 89,
     85, 83, 78, 73, 72, 67, 66, 62,
     60, 59, 56, 53, 52, 48, 47, 43,
     42, 40, 36, 35, 34, 33, 31, 30,
     28, 25, 24, 22, 20, 17, 14, 10
  },
  {
    365,348,333,316,300,287,277,265,
    252,240,229,219,206,197,189,180,
    171,168,160,153,146,139,132,127,
    121,115,110,107,101, 97, 94, 89,
     85, 83, 78, 73, 72, 67, 66, 62,
     60, 59, 56, 53, 52, 48, 47, 43,
     42, 40, 36, 35, 34, 33, 31, 30,
     28, 25, 24, 22, 20, 17, 14, 10
  },
  {
    30,25,20,20,15,15,14,14,
    13,13,12,12,11,11,10,10,
     9, 9, 8, 8, 7, 7, 7, 7,
     6, 6, 6, 6, 5, 5, 5, 5,
     4, 4, 4, 4, 3, 3, 3, 3,
     2, 2, 2, 2, 2, 2, 2, 2,
     0, 0, 0, 0, 0, 0, 0, 0,
     0, 0, 0, 0, 0, 0, 0, 0
  },
  {
    {
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTRA_Y},
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTRA_C},
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTRA_C}
    },
    {
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTER},
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTER},
      {3,OC_DEF_QRANGE_SIZES,OC_DEF_BASES_INTER}
    }
  }
};



/*The Huffman codes used for macro block modes.*/

const unsigned char OC_MODE_BITS[2][OC_NMODES]={
  /*Codebook 0: a maximally skewed prefix code.*/
  {1,2,3,4,5,6,7,7},
  /*Codebook 1: a fixed-length code.*/
  {3,3,3,3,3,3,3,3}
};

static const unsigned char OC_MODE_CODES[2][OC_NMODES]={
  /*Codebook 0: a maximally skewed prefix code.*/
  {0x00,0x02,0x06,0x0E,0x1E,0x3E,0x7E,0x7F},
  /*Codebook 1: a fixed-length code.*/
  {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07}
};


/*The Huffman codes used for motion vectors.*/

const unsigned char OC_MV_BITS[2][64]={
  /*Codebook 0: VLC code.*/
  {
      8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,7,7,7,7,7,7,7,7,6,6,6,6,4,4,3,
    3,
    3,4,4,6,6,6,6,7,7,7,7,7,7,7,7,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
  },
  /*Codebook 1: (5 bit magnitude, 1 bit sign).
    This wastes a code word (0x01, negative zero), or a bit (0x00, positive
     zero, requires only 5 bits to uniquely decode), but is hopefully not used
     very often.*/
  {
      6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
    6,6,6,6,6,6,6,6,6,6,6,6,6,6,6
  }
};

static const unsigned char OC_MV_CODES[2][64]={
  /*Codebook 0: VLC code.*/
  {
         0xFF,0xFD,0xFB,0xF9,0xF7,0xF5,0xF3,
    0xF1,0xEF,0xED,0xEB,0xE9,0xE7,0xE5,0xE3,
    0xE1,0x6F,0x6D,0x6B,0x69,0x67,0x65,0x63,
    0x61,0x2F,0x2D,0x2B,0x29,0x09,0x07,0x02,
    0x00,
    0x01,0x06,0x08,0x28,0x2A,0x2C,0x2E,0x60,
    0x62,0x64,0x66,0x68,0x6A,0x6C,0x6E,0xE0,
    0xE2,0xE4,0xE6,0xE8,0xEA,0xEC,0xEE,0xF0,
    0xF2,0xF4,0xF6,0xF8,0xFA,0xFC,0xFE
  },
  /*Codebook 1: (5 bit magnitude, 1 bit sign).*/
  {
         0x3F,0x3D,0x3B,0x39,0x37,0x35,0x33,
    0x31,0x2F,0x2D,0x2B,0x29,0x27,0x25,0x23,
    0x21,0x1F,0x1D,0x1B,0x19,0x17,0x15,0x13,
    0x11,0x0F,0x0D,0x0B,0x09,0x07,0x05,0x03,
    0x00,
    0x02,0x04,0x06,0x08,0x0A,0x0C,0x0E,0x10,
    0x12,0x14,0x16,0x18,0x1A,0x1C,0x1E,0x20,
    0x22,0x24,0x26,0x28,0x2A,0x2C,0x2E,0x30,
    0x32,0x34,0x36,0x38,0x3A,0x3C,0x3E
  }
};



/*Super block run coding scheme:
   Codeword             Run Length
   0                       1
   10x                     2-3
   110x                    4-5
   1110xx                  6-9
   11110xxx                10-17
   111110xxxx              18-33
   111111xxxxxxxxxxxx      34-4129*/
const ogg_uint16_t    OC_SB_RUN_VAL_MIN[8]={1,2,4,6,10,18,34,4130};
static const unsigned OC_SB_RUN_CODE_PREFIX[7]={
  0,4,0xC,0x38,0xF0,0x3E0,0x3F000
};
const unsigned char   OC_SB_RUN_CODE_NBITS[7]={1,3,4,6,8,10,18};


/*Writes the bit pattern for the run length of a super block run to the given
   oggpack_buffer.
  _opb:       The buffer to write to.
  _run_count: The length of the run, which must be positive.
  _flag:      The current flag.
  _done:      Whether or not more flags are to be encoded.*/
static void oc_sb_run_pack(oggpack_buffer *_opb,ptrdiff_t _run_count,
 int _flag,int _done){
  int i;
  if(_run_count>=4129){
    do{
      oggpackB_write(_opb,0x3FFFF,18);
      _run_count-=4129;
      if(_run_count>0)oggpackB_write(_opb,_flag,1);
      else if(!_done)oggpackB_write(_opb,!_flag,1);
    }
    while(_run_count>=4129);
    if(_run_count<=0)return;
  }
  for(i=0;_run_count>=OC_SB_RUN_VAL_MIN[i+1];i++);
  oggpackB_write(_opb,OC_SB_RUN_CODE_PREFIX[i]+_run_count-OC_SB_RUN_VAL_MIN[i],
   OC_SB_RUN_CODE_NBITS[i]);
}



/*Block run coding scheme:
   Codeword             Run Length
   0x                      1-2
   10x                     3-4
   110x                    5-6
   1110xx                  7-10
   11110xx                 11-14
   11111xxxx               15-30*/
const unsigned char OC_BLOCK_RUN_CODE_NBITS[30]={
  2,2,3,3,4,4,6,6,6,6,7,7,7,7,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9
};
static const ogg_uint16_t  OC_BLOCK_RUN_CODE_PATTERN[30]={
        0x000,0x001,0x004,0x005,0x00C,0x00D,0x038,
  0x039,0x03A,0x03B,0x078,0x079,0x07A,0x07B,0x1F0,
  0x1F1,0x1F2,0x1F3,0x1F4,0x1F5,0x1F6,0x1F7,0x1F8,
  0x1F9,0x1FA,0x1FB,0x1FC,0x1FD,0x1FE,0x1FF
};


/*Writes the bit pattern for the run length of a block run to the given
   oggpack_buffer.
  _opb:       The buffer to write to.
  _run_count: The length of the run.
              This must be positive, and no more than 30.*/
static void oc_block_run_pack(oggpack_buffer *_opb,int _run_count){
  oggpackB_write(_opb,OC_BLOCK_RUN_CODE_PATTERN[_run_count-1],
   OC_BLOCK_RUN_CODE_NBITS[_run_count-1]);
}



static void oc_enc_frame_header_pack(oc_enc_ctx *_enc){
  /*Mark this as a data packet.*/
  oggpackB_write(&_enc->opb,0,1);
  /*Output the frame type (key frame or delta frame).*/
  oggpackB_write(&_enc->opb,_enc->state.frame_type,1);
  /*Write out the current qi list.*/
  oggpackB_write(&_enc->opb,_enc->state.qis[0],6);
  if(_enc->state.nqis>1){
    oggpackB_write(&_enc->opb,1,1);
    oggpackB_write(&_enc->opb,_enc->state.qis[1],6);
    if(_enc->state.nqis>2){
      oggpackB_write(&_enc->opb,1,1);
      oggpackB_write(&_enc->opb,_enc->state.qis[2],6);
    }
    else oggpackB_write(&_enc->opb,0,1);
  }
  else oggpackB_write(&_enc->opb,0,1);
  if(_enc->state.frame_type==OC_INTRA_FRAME){
    /*Key frames have 3 unused configuration bits, holdovers from the VP3 days.
      Most of the other unused bits in the VP3 headers were eliminated.
      Monty kept these to leave us some wiggle room for future expansion,
       though a single bit in all frames would have been far more useful.*/
    oggpackB_write(&_enc->opb,0,3);
  }
}

/*Writes the bit flags for whether or not each super block is partially coded
   or not.
  These flags are run-length encoded, with the flag value alternating between
   each run.
  Return: The number partially coded SBs.*/
static unsigned oc_enc_partial_sb_flags_pack(oc_enc_ctx *_enc){
  const oc_sb_flags *sb_flags;
  unsigned           nsbs;
  unsigned           sbi;
  unsigned           npartial;
  int                flag;
  sb_flags=_enc->state.sb_flags;
  nsbs=_enc->state.nsbs;
  flag=sb_flags[0].coded_partially;
  oggpackB_write(&_enc->opb,flag,1);
  sbi=npartial=0;
  do{
    unsigned run_count;
    for(run_count=0;sbi<nsbs;sbi++){
      if(sb_flags[sbi].coded_partially!=flag)break;
      run_count++;
      npartial+=flag;
    }
    oc_sb_run_pack(&_enc->opb,run_count,flag,sbi>=nsbs);
    flag=!flag;
  }
  while(sbi<nsbs);
  return npartial;
}

/*Writes the coded/not coded flags for each super block that is not partially
   coded.
  These flags are run-length encoded, with the flag value altenating between
   each run.*/
static void oc_enc_coded_sb_flags_pack(oc_enc_ctx *_enc){
  const oc_sb_flags *sb_flags;
  unsigned           nsbs;
  unsigned           sbi;
  int                flag;
  sb_flags=_enc->state.sb_flags;
  nsbs=_enc->state.nsbs;
  /*Skip partially coded super blocks; their flags have already been coded.*/
  for(sbi=0;sb_flags[sbi].coded_partially;sbi++);
  flag=sb_flags[sbi].coded_fully;
  oggpackB_write(&_enc->opb,flag,1);
  do{
    unsigned run_count;
    for(run_count=0;sbi<nsbs;sbi++){
      if(sb_flags[sbi].coded_partially)continue;
      if(sb_flags[sbi].coded_fully!=flag)break;
      run_count++;
    }
    oc_sb_run_pack(&_enc->opb,run_count,flag,sbi>=nsbs);
    flag=!flag;
  }
  while(sbi<nsbs);
}

static void oc_enc_coded_flags_pack(oc_enc_ctx *_enc){
  const oc_sb_map   *sb_maps;
  const oc_sb_flags *sb_flags;
  unsigned           nsbs;
  const oc_fragment *frags;
  unsigned           npartial;
  int                run_count;
  int                flag;
  int                pli;
  unsigned           sbi;
  npartial=oc_enc_partial_sb_flags_pack(_enc);
  if(npartial<_enc->state.nsbs)oc_enc_coded_sb_flags_pack(_enc);
  sb_maps=(const oc_sb_map *)_enc->state.sb_maps;
  sb_flags=_enc->state.sb_flags;
  nsbs=_enc->state.nsbs;
  frags=_enc->state.frags;
  for(sbi=0;sbi<nsbs&&!sb_flags[sbi].coded_partially;sbi++);
  /*If there's at least one partial SB, store individual coded block flags.*/
  if(sbi<nsbs){
    flag=frags[sb_maps[sbi][0][0]].coded;
    oggpackB_write(&_enc->opb,flag,1);
    run_count=0;
    nsbs=sbi=0;
    for(pli=0;pli<3;pli++){
      nsbs+=_enc->state.fplanes[pli].nsbs;
      for(;sbi<nsbs;sbi++){
        int       quadi;
        int       bi;
        ptrdiff_t fragi;
        if(sb_flags[sbi].coded_partially){
          for(quadi=0;quadi<4;quadi++){
            for(bi=0;bi<4;bi++){
              fragi=sb_maps[sbi][quadi][bi];
              if(fragi>=0){
                if(frags[fragi].coded!=flag){
                  oc_block_run_pack(&_enc->opb,run_count);
                  flag=!flag;
                  run_count=1;
                }
                else run_count++;
              }
            }
          }
        }
      }
    }
    /*Flush any trailing block coded run.*/
    if(run_count>0)oc_block_run_pack(&_enc->opb,run_count);
  }
}

static void oc_enc_mb_modes_pack(oc_enc_ctx *_enc){
  const unsigned char *mode_codes;
  const unsigned char *mode_bits;
  const unsigned char *mode_ranks;
  unsigned            *coded_mbis;
  size_t               ncoded_mbis;
  const signed char   *mb_modes;
  unsigned             mbii;
  int                  scheme;
  int                  mb_mode;
  scheme=_enc->chooser.scheme_list[0];
  /*Encode the best scheme.*/
  oggpackB_write(&_enc->opb,scheme,3);
  /*If the chosen scheme is scheme 0, send the mode frequency ordering.*/
  if(scheme==0){
    for(mb_mode=0;mb_mode<OC_NMODES;mb_mode++){
      oggpackB_write(&_enc->opb,_enc->chooser.scheme0_ranks[mb_mode],3);
    }
  }
  mode_ranks=_enc->chooser.mode_ranks[scheme];
  mode_bits=OC_MODE_BITS[scheme+1>>3];
  mode_codes=OC_MODE_CODES[scheme+1>>3];
  coded_mbis=_enc->coded_mbis;
  ncoded_mbis=_enc->ncoded_mbis;
  mb_modes=_enc->state.mb_modes;
  for(mbii=0;mbii<ncoded_mbis;mbii++){
    int rank;
    rank=mode_ranks[mb_modes[coded_mbis[mbii]]];
    oggpackB_write(&_enc->opb,mode_codes[rank],mode_bits[rank]);
  }
}

static void oc_enc_mv_pack(oc_enc_ctx *_enc,int _mv_scheme,int _dx,int _dy){
  oggpackB_write(&_enc->opb,
   OC_MV_CODES[_mv_scheme][_dx+31],OC_MV_BITS[_mv_scheme][_dx+31]);
  oggpackB_write(&_enc->opb,
   OC_MV_CODES[_mv_scheme][_dy+31],OC_MV_BITS[_mv_scheme][_dy+31]);
}

static void oc_enc_mvs_pack(oc_enc_ctx *_enc){
  const unsigned     *coded_mbis;
  size_t              ncoded_mbis;
  const oc_mb_map    *mb_maps;
  const signed char  *mb_modes;
  const oc_fragment  *frags;
  const oc_mv        *frag_mvs;
  unsigned            mbii;
  int                 mv_scheme;
  /*Choose the coding scheme.*/
  mv_scheme=_enc->mv_bits[1]<_enc->mv_bits[0];
  oggpackB_write(&_enc->opb,mv_scheme,1);
  /*Encode the motion vectors.
    Macro blocks are iterated in Hilbert scan order, but the MVs within the
     macro block are coded in raster order.*/
  coded_mbis=_enc->coded_mbis;
  ncoded_mbis=_enc->ncoded_mbis;
  mb_modes=_enc->state.mb_modes;
  mb_maps=(const oc_mb_map *)_enc->state.mb_maps;
  frags=_enc->state.frags;
  frag_mvs=(const oc_mv *)_enc->state.frag_mvs;
  for(mbii=0;mbii<ncoded_mbis;mbii++){
    ptrdiff_t fragi;
    unsigned  mbi;
    int       bi;
    mbi=coded_mbis[mbii];
    switch(mb_modes[mbi]){
      case OC_MODE_INTER_MV:
      case OC_MODE_GOLDEN_MV:{
        for(bi=0;;bi++){
          fragi=mb_maps[mbi][0][bi];
          if(frags[fragi].coded){
            oc_enc_mv_pack(_enc,mv_scheme,
             frag_mvs[fragi][0],frag_mvs[fragi][1]);
            /*Only code a single MV for this macro block.*/
            break;
          }
        }
      }break;
      case OC_MODE_INTER_MV_FOUR:{
        for(bi=0;bi<4;bi++){
          fragi=mb_maps[mbi][0][bi];
          if(frags[fragi].coded){
            oc_enc_mv_pack(_enc,mv_scheme,
             frag_mvs[fragi][0],frag_mvs[fragi][1]);
            /*Keep coding all the MVs for this macro block.*/
          }
        }
      }break;
    }
  }
}

static void oc_enc_block_qis_pack(oc_enc_ctx *_enc){
  const oc_fragment *frags;
  ptrdiff_t         *coded_fragis;
  ptrdiff_t          ncoded_fragis;
  ptrdiff_t          fragii;
  ptrdiff_t          run_count;
  ptrdiff_t          nqi0;
  int                flag;
  if(_enc->state.nqis<=1)return;
  ncoded_fragis=_enc->state.ntotal_coded_fragis;
  if(ncoded_fragis<=0)return;
  coded_fragis=_enc->state.coded_fragis;
  frags=_enc->state.frags;
  flag=!!frags[coded_fragis[0]].qii;
  oggpackB_write(&_enc->opb,flag,1);
  nqi0=0;
  for(fragii=0;fragii<ncoded_fragis;){
    for(run_count=0;fragii<ncoded_fragis;fragii++){
      if(!!frags[coded_fragis[fragii]].qii!=flag)break;
      run_count++;
      nqi0+=!flag;
    }
    oc_sb_run_pack(&_enc->opb,run_count,flag,fragii>=ncoded_fragis);
    flag=!flag;
  }
  if(_enc->state.nqis<3||nqi0>=ncoded_fragis)return;
  for(fragii=0;!frags[coded_fragis[fragii]].qii;fragii++);
  flag=frags[coded_fragis[fragii]].qii-1;
  oggpackB_write(&_enc->opb,flag,1);
  while(fragii<ncoded_fragis){
    for(run_count=0;fragii<ncoded_fragis;fragii++){
      int qii;
      qii=frags[coded_fragis[fragii]].qii;
      if(!qii)continue;
      if(qii-1!=flag)break;
      run_count++;
    }
    oc_sb_run_pack(&_enc->opb,run_count,flag,fragii>=ncoded_fragis);
    flag=!flag;
  }
}

/*Counts the tokens of each type used for the given range of coefficient
   indices in zig-zag order.
  _zzi_start:      The first zig-zag index to include.
  _zzi_end:        The first zig-zag index to not include.
  _token_counts_y: Returns the token counts for the Y' plane.
  _token_counts_c: Returns the token counts for the Cb and Cr planes.*/
static void oc_enc_count_tokens(oc_enc_ctx *_enc,int _zzi_start,int _zzi_end,
 ptrdiff_t _token_counts_y[32],ptrdiff_t _token_counts_c[32]){
  const unsigned char *dct_tokens;
  ptrdiff_t            ndct_tokens;
  int                  pli;
  int                  zzi;
  ptrdiff_t            ti;
  memset(_token_counts_y,0,32*sizeof(*_token_counts_y));
  memset(_token_counts_c,0,32*sizeof(*_token_counts_c));
  for(zzi=_zzi_start;zzi<_zzi_end;zzi++){
    dct_tokens=_enc->dct_tokens[0][zzi];
    ndct_tokens=_enc->ndct_tokens[0][zzi];
    for(ti=_enc->dct_token_offs[0][zzi];ti<ndct_tokens;ti++){
      _token_counts_y[dct_tokens[ti]]++;
    }
  }
  for(pli=1;pli<3;pli++){
    for(zzi=_zzi_start;zzi<_zzi_end;zzi++){
      dct_tokens=_enc->dct_tokens[pli][zzi];
      ndct_tokens=_enc->ndct_tokens[pli][zzi];
      for(ti=_enc->dct_token_offs[pli][zzi];ti<ndct_tokens;ti++){
        _token_counts_c[dct_tokens[ti]]++;
      }
    }
  }
}

/*Computes the number of bits used for each of the potential Huffman code for
   the given list of token counts.
  The bits are added to whatever the current bit counts are.*/
static void oc_enc_count_bits(oc_enc_ctx *_enc,int _hgi,
 const ptrdiff_t _token_counts[32],size_t _bit_counts[16]){
  int huffi;
  int huff_offs;
  int token;
  huff_offs=_hgi<<4;
  for(huffi=0;huffi<16;huffi++){
    for(token=0;token<32;token++){
      _bit_counts[huffi]+=
       _token_counts[token]*_enc->huff_codes[huffi+huff_offs][token].nbits;
    }
  }
}

/*Returns the Huffman index using the fewest number of bits.*/
static int oc_select_huff_idx(size_t _bit_counts[16]){
  int best_huffi;
  int huffi;
  best_huffi=0;
  for(huffi=1;huffi<16;huffi++)if(_bit_counts[huffi]<_bit_counts[best_huffi]){
    best_huffi=huffi;
  }
  return best_huffi;
}

static void oc_enc_huff_group_pack(oc_enc_ctx *_enc,
 int _zzi_start,int _zzi_end,const int _huff_idxs[2]){
  int zzi;
  for(zzi=_zzi_start;zzi<_zzi_end;zzi++){
    int pli;
    for(pli=0;pli<3;pli++){
      const unsigned char *dct_tokens;
      const ogg_uint16_t  *extra_bits;
      ptrdiff_t            ndct_tokens;
      const th_huff_code  *huff_codes;
      ptrdiff_t            ti;
      dct_tokens=_enc->dct_tokens[pli][zzi];
      extra_bits=_enc->extra_bits[pli][zzi];
      ndct_tokens=_enc->ndct_tokens[pli][zzi];
      huff_codes=_enc->huff_codes[_huff_idxs[pli+1>>1]];
      for(ti=_enc->dct_token_offs[pli][zzi];ti<ndct_tokens;ti++){
        int token;
        int neb;
        token=dct_tokens[ti];
        oggpackB_write(&_enc->opb,huff_codes[token].pattern,
         huff_codes[token].nbits);
        neb=OC_DCT_TOKEN_EXTRA_BITS[token];
        if(neb)oggpackB_write(&_enc->opb,extra_bits[ti],neb);
      }
    }
  }
}

static void oc_enc_residual_tokens_pack(oc_enc_ctx *_enc){
  static const unsigned char  OC_HUFF_GROUP_MIN[6]={0,1,6,15,28,64};
  static const unsigned char *OC_HUFF_GROUP_MAX=OC_HUFF_GROUP_MIN+1;
  ptrdiff_t token_counts_y[32];
  ptrdiff_t token_counts_c[32];
  size_t    bits_y[16];
  size_t    bits_c[16];
  int       huff_idxs[2];
  int       frame_type;
  int       hgi;
  frame_type=_enc->state.frame_type;
  /*Choose which Huffman tables to use for the DC token list.*/
  oc_enc_count_tokens(_enc,0,1,token_counts_y,token_counts_c);
  memset(bits_y,0,sizeof(bits_y));
  memset(bits_c,0,sizeof(bits_c));
  oc_enc_count_bits(_enc,0,token_counts_y,bits_y);
  oc_enc_count_bits(_enc,0,token_counts_c,bits_c);
  huff_idxs[0]=oc_select_huff_idx(bits_y);
  huff_idxs[1]=oc_select_huff_idx(bits_c);
  /*Write the DC token list with the chosen tables.*/
  oggpackB_write(&_enc->opb,huff_idxs[0],4);
  oggpackB_write(&_enc->opb,huff_idxs[1],4);
  _enc->huff_idxs[frame_type][0][0]=(unsigned char)huff_idxs[0];
  _enc->huff_idxs[frame_type][0][1]=(unsigned char)huff_idxs[1];
  oc_enc_huff_group_pack(_enc,0,1,huff_idxs);
  /*Choose which Huffman tables to use for the AC token lists.*/
  memset(bits_y,0,sizeof(bits_y));
  memset(bits_c,0,sizeof(bits_c));
  for(hgi=1;hgi<5;hgi++){
    oc_enc_count_tokens(_enc,OC_HUFF_GROUP_MIN[hgi],OC_HUFF_GROUP_MAX[hgi],
     token_counts_y,token_counts_c);
    oc_enc_count_bits(_enc,hgi,token_counts_y,bits_y);
    oc_enc_count_bits(_enc,hgi,token_counts_c,bits_c);
  }
  huff_idxs[0]=oc_select_huff_idx(bits_y);
  huff_idxs[1]=oc_select_huff_idx(bits_c);
  /*Write the AC token lists using the chosen tables.*/
  oggpackB_write(&_enc->opb,huff_idxs[0],4);
  oggpackB_write(&_enc->opb,huff_idxs[1],4);
  _enc->huff_idxs[frame_type][1][0]=(unsigned char)huff_idxs[0];
  _enc->huff_idxs[frame_type][1][1]=(unsigned char)huff_idxs[1];
  for(hgi=1;hgi<5;hgi++){
    huff_idxs[0]+=16;
    huff_idxs[1]+=16;
    oc_enc_huff_group_pack(_enc,
     OC_HUFF_GROUP_MIN[hgi],OC_HUFF_GROUP_MAX[hgi],huff_idxs);
  }
}

static void oc_enc_frame_pack(oc_enc_ctx *_enc){
  oggpackB_reset(&_enc->opb);
  /*Only proceed if we have some coded blocks.
    If there are no coded blocks, we can drop this frame simply by emitting a
     0 byte packet.*/
  if(_enc->state.ntotal_coded_fragis>0){
    oc_enc_frame_header_pack(_enc);
    if(_enc->state.frame_type==OC_INTER_FRAME){
      /*Coded block flags, MB modes, and MVs are only needed for delta frames.*/
      oc_enc_coded_flags_pack(_enc);
      oc_enc_mb_modes_pack(_enc);
      oc_enc_mvs_pack(_enc);
    }
    oc_enc_block_qis_pack(_enc);
    oc_enc_tokenize_finish(_enc);
    oc_enc_residual_tokens_pack(_enc);
  }
  /*Success: Mark the packet as ready to be flushed.*/
  _enc->packet_state=OC_PACKET_READY;
#if defined(OC_COLLECT_METRICS)
  oc_enc_mode_metrics_collect(_enc);
#endif
}


void oc_enc_vtable_init_c(oc_enc_ctx *_enc){
  /*The implementations prefixed with oc_enc_ are encoder-specific.
    The rest we re-use from the decoder.*/
  _enc->opt_vtable.frag_sad=oc_enc_frag_sad_c;
  _enc->opt_vtable.frag_sad_thresh=oc_enc_frag_sad_thresh_c;
  _enc->opt_vtable.frag_sad2_thresh=oc_enc_frag_sad2_thresh_c;
  _enc->opt_vtable.frag_satd_thresh=oc_enc_frag_satd_thresh_c;
  _enc->opt_vtable.frag_satd2_thresh=oc_enc_frag_satd2_thresh_c;
  _enc->opt_vtable.frag_intra_satd=oc_enc_frag_intra_satd_c;
  _enc->opt_vtable.frag_sub=oc_enc_frag_sub_c;
  _enc->opt_vtable.frag_sub_128=oc_enc_frag_sub_128_c;
  _enc->opt_vtable.frag_copy2=oc_enc_frag_copy2_c;
  _enc->opt_vtable.frag_recon_intra=oc_frag_recon_intra_c;
  _enc->opt_vtable.frag_recon_inter=oc_frag_recon_inter_c;
  _enc->opt_vtable.fdct8x8=oc_enc_fdct8x8_c;
}

/*Initialize the macro block neighbor lists for MC analysis.
  This assumes that the entire mb_info memory region has been initialized with
   zeros.*/
static void oc_enc_mb_info_init(oc_enc_ctx *_enc){
  oc_mb_enc_info    *embs;
  const signed char *mb_modes;
  unsigned           nhsbs;
  unsigned           nvsbs;
  unsigned           nhmbs;
  unsigned           nvmbs;
  unsigned           sby;
  mb_modes=_enc->state.mb_modes;
  embs=_enc->mb_info;
  nhsbs=_enc->state.fplanes[0].nhsbs;
  nvsbs=_enc->state.fplanes[0].nvsbs;
  nhmbs=_enc->state.nhmbs;
  nvmbs=_enc->state.nvmbs;
  for(sby=0;sby<nvsbs;sby++){
    unsigned sbx;
    for(sbx=0;sbx<nhsbs;sbx++){
      int quadi;
      for(quadi=0;quadi<4;quadi++){
        /*Because of the Hilbert curve ordering the macro blocks are
           visited in, the available neighbors change depending on where in
           a super block the macro block is located.
          Only the first three vectors are used in the median calculation
           for the optimal predictor, and so the most important should be
           listed first.
          Additional vectors are used, so there will always be at least 3,
           except for in the upper-left most macro block.*/
        /*The number of current neighbors for each macro block position.*/
        static const unsigned char NCNEIGHBORS[4]={4,3,2,4};
        /*The offset of each current neighbor in the X direction.*/
        static const signed char   CDX[4][4]={
          {-1,0,1,-1},
          {-1,0,-1,},
          {-1,-1},
          {-1,0,0,1}
        };
        /*The offset of each current neighbor in the Y direction.*/
        static const signed char   CDY[4][4]={
          {0,-1,-1,-1},
          {0,-1,-1},
          {0,-1},
          {0,-1,1,-1}
        };
        /*The offset of each previous neighbor in the X direction.*/
        static const signed char   PDX[4]={-1,0,1,0};
        /*The offset of each previous neighbor in the Y direction.*/
        static const signed char   PDY[4]={0,-1,0,1};
        unsigned mbi;
        int      mbx;
        int      mby;
        unsigned nmbi;
        int      nmbx;
        int      nmby;
        int      ni;
        mbi=(sby*nhsbs+sbx<<2)+quadi;
        if(mb_modes[mbi]==OC_MODE_INVALID)continue;
        mbx=2*sbx+(quadi>>1);
        mby=2*sby+(quadi+1>>1&1);
        /*Fill in the neighbors with current motion vectors available.*/
        for(ni=0;ni<NCNEIGHBORS[quadi];ni++){
          nmbx=mbx+CDX[quadi][ni];
          nmby=mby+CDY[quadi][ni];
          if(nmbx<0||nmbx>=nhmbs||nmby<0||nmby>=nvmbs)continue;
          nmbi=(nmby&~1)*nhmbs+((nmbx&~1)<<1)+OC_MB_MAP[nmby&1][nmbx&1];
          if(mb_modes[nmbi]==OC_MODE_INVALID)continue;
          embs[mbi].cneighbors[embs[mbi].ncneighbors++]=nmbi;
        }
        /*Fill in the neighbors with previous motion vectors available.*/
        for(ni=0;ni<4;ni++){
          nmbx=mbx+PDX[ni];
          nmby=mby+PDY[ni];
          if(nmbx<0||nmbx>=nhmbs||nmby<0||nmby>=nvmbs)continue;
          nmbi=(nmby&~1)*nhmbs+((nmbx&~1)<<1)+OC_MB_MAP[nmby&1][nmbx&1];
          if(mb_modes[nmbi]==OC_MODE_INVALID)continue;
          embs[mbi].pneighbors[embs[mbi].npneighbors++]=nmbi;
        }
      }
    }
  }
}

static int oc_enc_set_huffman_codes(oc_enc_ctx *_enc,
 const th_huff_code _codes[TH_NHUFFMAN_TABLES][TH_NDCT_TOKENS]){
  int ret;
  if(_enc==NULL)return TH_EFAULT;
  if(_enc->packet_state>OC_PACKET_SETUP_HDR)return TH_EINVAL;
  if(_codes==NULL)_codes=TH_VP31_HUFF_CODES;
  /*Validate the codes.*/
  oggpackB_reset(&_enc->opb);
  ret=oc_huff_codes_pack(&_enc->opb,_codes);
  if(ret<0)return ret;
  memcpy(_enc->huff_codes,_codes,sizeof(_enc->huff_codes));
  return 0;
}

/*Sets the quantization parameters to use.
  This may only be called before the setup header is written.
  If it is called multiple times, only the last call has any effect.
  _qinfo: The quantization parameters.
          These are described in more detail in theoraenc.h.
          This can be NULL, in which case the default quantization parameters
           will be used.*/
static int oc_enc_set_quant_params(oc_enc_ctx *_enc,
 const th_quant_info *_qinfo){
  int qi;
  int pli;
  int qti;
  if(_enc==NULL)return TH_EFAULT;
  if(_enc->packet_state>OC_PACKET_SETUP_HDR)return TH_EINVAL;
  if(_qinfo==NULL)_qinfo=&TH_DEF_QUANT_INFO;
  /*TODO: Analyze for packing purposes instead of just doing a shallow copy.*/
  memcpy(&_enc->qinfo,_qinfo,sizeof(_enc->qinfo));
  for(qi=0;qi<64;qi++)for(pli=0;pli<3;pli++)for(qti=0;qti<2;qti++){
    _enc->state.dequant_tables[qi][pli][qti]=
     _enc->state.dequant_table_data[qi][pli][qti];
    _enc->enquant_tables[qi][pli][qti]=_enc->enquant_table_data[qi][pli][qti];
  }
  oc_enquant_tables_init(_enc->state.dequant_tables,
   _enc->enquant_tables,_qinfo);
  memcpy(_enc->state.loop_filter_limits,_qinfo->loop_filter_limits,
   sizeof(_enc->state.loop_filter_limits));
  oc_enquant_qavg_init(_enc->log_qavg,_enc->state.dequant_tables,
   _enc->state.info.pixel_fmt);
  return 0;
}

static void oc_enc_clear(oc_enc_ctx *_enc);

static int oc_enc_init(oc_enc_ctx *_enc,const th_info *_info){
  th_info   info;
  size_t    mcu_nmbs;
  ptrdiff_t mcu_nfrags;
  int       hdec;
  int       vdec;
  int       ret;
  int       pli;
  /*Clean up the requested settings.*/
  memcpy(&info,_info,sizeof(info));
  info.version_major=TH_VERSION_MAJOR;
  info.version_minor=TH_VERSION_MINOR;
  info.version_subminor=TH_VERSION_SUB;
  if(info.quality>63)info.quality=63;
  if(info.quality<0)info.quality=32;
  if(info.target_bitrate<0)info.target_bitrate=0;
  /*Initialize the shared encoder/decoder state.*/
  ret=oc_state_init(&_enc->state,&info,4);
  if(ret<0)return ret;
  _enc->mb_info=_ogg_calloc(_enc->state.nmbs,sizeof(*_enc->mb_info));
  _enc->frag_dc=_ogg_calloc(_enc->state.nfrags,sizeof(*_enc->frag_dc));
  _enc->coded_mbis=
   (unsigned *)_ogg_malloc(_enc->state.nmbs*sizeof(*_enc->coded_mbis));
  hdec=!(_enc->state.info.pixel_fmt&1);
  vdec=!(_enc->state.info.pixel_fmt&2);
  /*If chroma is sub-sampled in the vertical direction, we have to encode two
     super block rows of Y' for each super block row of Cb and Cr.*/
  _enc->mcu_nvsbs=1<<vdec;
  mcu_nmbs=_enc->mcu_nvsbs*_enc->state.fplanes[0].nhsbs*(size_t)4;
  mcu_nfrags=4*mcu_nmbs+(8*mcu_nmbs>>hdec+vdec);
  _enc->mcu_skip_ssd=(unsigned *)_ogg_malloc(
   mcu_nfrags*sizeof(*_enc->mcu_skip_ssd));
  for(pli=0;pli<3;pli++){
    _enc->dct_tokens[pli]=(unsigned char **)oc_malloc_2d(64,
     _enc->state.fplanes[pli].nfrags,sizeof(**_enc->dct_tokens));
    _enc->extra_bits[pli]=(ogg_uint16_t **)oc_malloc_2d(64,
     _enc->state.fplanes[pli].nfrags,sizeof(**_enc->extra_bits));
  }
#if defined(OC_COLLECT_METRICS)
  _enc->frag_satd=_ogg_calloc(_enc->state.nfrags,sizeof(*_enc->frag_satd));
  _enc->frag_ssd=_ogg_calloc(_enc->state.nfrags,sizeof(*_enc->frag_ssd));
#endif
#if defined(OC_X86_ASM)
  oc_enc_vtable_init_x86(_enc);
#else
  oc_enc_vtable_init_c(_enc);
#endif
  _enc->keyframe_frequency_force=1<<_enc->state.info.keyframe_granule_shift;
  _enc->state.qis[0]=_enc->state.info.quality;
  _enc->state.nqis=1;
  oc_rc_state_init(&_enc->rc,_enc);
  oggpackB_writeinit(&_enc->opb);
  if(_enc->mb_info==NULL||_enc->frag_dc==NULL||_enc->coded_mbis==NULL||
   _enc->mcu_skip_ssd==NULL||_enc->dct_tokens[0]==NULL||
   _enc->dct_tokens[1]==NULL||_enc->dct_tokens[2]==NULL||
   _enc->extra_bits[0]==NULL||_enc->extra_bits[1]==NULL||
   _enc->extra_bits[2]==NULL
#if defined(OC_COLLECT_METRICS)
   ||_enc->frag_satd==NULL||_enc->frag_ssd==NULL
#endif
   ){
    oc_enc_clear(_enc);
    return TH_EFAULT;
  }
  oc_mode_scheme_chooser_init(&_enc->chooser);
  oc_enc_mb_info_init(_enc);
  memset(_enc->huff_idxs,0,sizeof(_enc->huff_idxs));
  /*Reset the packet-out state machine.*/
  _enc->packet_state=OC_PACKET_INFO_HDR;
  _enc->dup_count=0;
  _enc->nqueued_dups=0;
  _enc->prev_dup_count=0;
  /*Enable speed optimizations up through early skip by default.*/
  _enc->sp_level=OC_SP_LEVEL_EARLY_SKIP;
  /*Disable VP3 compatibility by default.*/
  _enc->vp3_compatible=0;
  /*No INTER frames coded yet.*/
  _enc->coded_inter_frame=0;
  memcpy(_enc->huff_codes,TH_VP31_HUFF_CODES,sizeof(_enc->huff_codes));
  oc_enc_set_quant_params(_enc,NULL);
  return 0;
}

static void oc_enc_clear(oc_enc_ctx *_enc){
  int pli;
  oc_rc_state_clear(&_enc->rc);
#if defined(OC_COLLECT_METRICS)
  oc_enc_mode_metrics_dump(_enc);
#endif
  oggpackB_writeclear(&_enc->opb);
#if defined(OC_COLLECT_METRICS)
  _ogg_free(_enc->frag_ssd);
  _ogg_free(_enc->frag_satd);
#endif
  for(pli=3;pli-->0;){
    oc_free_2d(_enc->extra_bits[pli]);
    oc_free_2d(_enc->dct_tokens[pli]);
  }
  _ogg_free(_enc->mcu_skip_ssd);
  _ogg_free(_enc->coded_mbis);
  _ogg_free(_enc->frag_dc);
  _ogg_free(_enc->mb_info);
  oc_state_clear(&_enc->state);
}

static void oc_enc_drop_frame(th_enc_ctx *_enc){
  /*Use the previous frame's reconstruction.*/
  _enc->state.ref_frame_idx[OC_FRAME_SELF]=
   _enc->state.ref_frame_idx[OC_FRAME_PREV];
  /*Flag motion vector analysis about the frame drop.*/
  _enc->prevframe_dropped=1;
  /*Zero the packet.*/
  oggpackB_reset(&_enc->opb);
}

static void oc_enc_compress_keyframe(oc_enc_ctx *_enc,int _recode){
  if(_enc->state.info.target_bitrate>0){
    _enc->state.qis[0]=oc_enc_select_qi(_enc,OC_INTRA_FRAME,
     _enc->state.curframe_num>0);
    _enc->state.nqis=1;
  }
  oc_enc_calc_lambda(_enc,OC_INTRA_FRAME);
  oc_enc_analyze_intra(_enc,_recode);
  oc_enc_frame_pack(_enc);
  /*On the first frame, the previous call was an initial dry-run to prime
     feed-forward statistics.*/
  if(!_recode&&_enc->state.curframe_num==0){
    if(_enc->state.info.target_bitrate>0){
      oc_enc_update_rc_state(_enc,oggpackB_bytes(&_enc->opb)<<3,
                             OC_INTRA_FRAME,_enc->state.qis[0],1,0);
    }
    oc_enc_compress_keyframe(_enc,1);
  }
}

static void oc_enc_compress_frame(oc_enc_ctx *_enc,int _recode){
  if(_enc->state.info.target_bitrate>0){
    _enc->state.qis[0]=oc_enc_select_qi(_enc,OC_INTER_FRAME,1);
    _enc->state.nqis=1;
  }
  oc_enc_calc_lambda(_enc,OC_INTER_FRAME);
  if(oc_enc_analyze_inter(_enc,_enc->rc.twopass!=2,_recode)){
    /*Mode analysis thinks this should have been a keyframe; start over.*/
    oc_enc_compress_keyframe(_enc,1);
  }
  else{
    oc_enc_frame_pack(_enc);
    if(!_enc->coded_inter_frame){
      /*On the first INTER frame, the previous call was an initial dry-run to
         prime feed-forward statistics.*/
      _enc->coded_inter_frame=1;
      if(_enc->state.info.target_bitrate>0){
        /*Rate control also needs to prime.*/
        oc_enc_update_rc_state(_enc,oggpackB_bytes(&_enc->opb)<<3,
         OC_INTER_FRAME,_enc->state.qis[0],1,0);
      }
      oc_enc_compress_frame(_enc,1);
    }
  }
}

/*Set the granule position for the next packet to output based on the current
   internal state.*/
static void oc_enc_set_granpos(oc_enc_ctx *_enc){
  unsigned dup_offs;
  /*Add an offset for the number of duplicate frames we've emitted so far.*/
  dup_offs=_enc->prev_dup_count-_enc->nqueued_dups;
  /*If the current frame was a keyframe, use it for the high part.*/
  if(_enc->state.frame_type==OC_INTRA_FRAME){
    _enc->state.granpos=(_enc->state.curframe_num+_enc->state.granpos_bias<<
     _enc->state.info.keyframe_granule_shift)+dup_offs;
  }
  /*Otherwise use the last keyframe in the high part and put the current frame
     in the low part.*/
  else{
    _enc->state.granpos=
     (_enc->state.keyframe_num+_enc->state.granpos_bias<<
     _enc->state.info.keyframe_granule_shift)
     +_enc->state.curframe_num-_enc->state.keyframe_num+dup_offs;
  }
}

static void oc_img_plane_copy_pad(th_img_plane *_dst,th_img_plane *_src,
 ogg_int32_t _pic_x,ogg_int32_t _pic_y,
 ogg_int32_t _pic_width,ogg_int32_t _pic_height){
  unsigned char *dst;
  int            dstride;
  ogg_uint32_t   frame_width;
  ogg_uint32_t   frame_height;
  ogg_uint32_t   y;
  frame_width=_dst->width;
  frame_height=_dst->height;
  /*If we have _no_ data, just encode a dull green.*/
  if(_pic_width==0||_pic_height==0){
    dst=_dst->data;
    dstride=_dst->stride;
    for(y=0;y<frame_height;y++){
      memset(dst,0,frame_width*sizeof(*dst));
      dst+=dstride;
    }
  }
  /*Otherwise, copy what we do have, and add our own padding.*/
  else{
    unsigned char *dst_data;
    unsigned char *src_data;
    unsigned char *src;
    int            sstride;
    ogg_uint32_t   x;
    /*Step 1: Copy the data we do have.*/
    dstride=_dst->stride;
    sstride=_src->stride;
    dst_data=_dst->data;
    src_data=_src->data;
    dst=dst_data+_pic_y*(ptrdiff_t)dstride+_pic_x;
    src=src_data+_pic_y*(ptrdiff_t)sstride+_pic_x;
    for(y=0;y<_pic_height;y++){
      memcpy(dst,src,_pic_width);
      dst+=dstride;
      src+=sstride;
    }
    /*Step 2: Perform a low-pass extension into the padding region.*/
    /*Left side.*/
    for(x=_pic_x;x-->0;){
      dst=dst_data+_pic_y*(ptrdiff_t)dstride+x;
      for(y=0;y<_pic_height;y++){
        dst[0]=(dst[1]<<1)+(dst-(dstride&-(y>0)))[1]
         +(dst+(dstride&-(y+1<_pic_height)))[1]+2>>2;
        dst+=dstride;
      }
    }
    /*Right side.*/
    for(x=_pic_x+_pic_width;x<frame_width;x++){
      dst=dst_data+_pic_y*(ptrdiff_t)dstride+x-1;
      for(y=0;y<_pic_height;y++){
        dst[1]=(dst[0]<<1)+(dst-(dstride&-(y>0)))[0]
         +(dst+(dstride&-(y+1<_pic_height)))[0]+2>>2;
        dst+=dstride;
      }
    }
    /*Top.*/
    dst=dst_data+_pic_y*(ptrdiff_t)dstride;
    for(y=_pic_y;y-->0;){
      for(x=0;x<frame_width;x++){
        (dst-dstride)[x]=(dst[x]<<1)+dst[x-(x>0)]
         +dst[x+(x+1<frame_width)]+2>>2;
      }
      dst-=dstride;
    }
    /*Bottom.*/
    dst=dst_data+(_pic_y+_pic_height)*(ptrdiff_t)dstride;
    for(y=_pic_y+_pic_height;y<frame_height;y++){
      for(x=0;x<frame_width;x++){
        dst[x]=((dst-dstride)[x]<<1)+(dst-dstride)[x-(x>0)]
         +(dst-dstride)[x+(x+1<frame_width)]+2>>2;
      }
      dst+=dstride;
    }
  }
}