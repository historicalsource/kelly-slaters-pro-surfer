#ifndef DMA_DEBUG_H
#define DMA_DEBUG_H

#ifdef DEBUG
#define dprintf(s) printf s
#define dputs(s) puts(s)
#else
#define dprintf(s) 
#define dputs(s)
#endif

typedef char BOOL;

#define USE_COLOURS

#ifdef USE_COLOURS
#define COLOUR_BLACK        "\033[30m"
#define COLOUR_RED          "\033[31m"
#define COLOUR_GREEN        "\033[32m"
#define COLOUR_YELLOW       "\033[33m"
#define COLOUR_BLUE         "\033[34m"
#define COLOUR_PURPLE       "\033[35m"
#define COLOUR_CYAN         "\033[36m"
#define COLOUR_LIGHT_GREY   "\033[37m"

#define COLOUR_BG_BLACK     "\033[40m"
#define COLOUR_BG_RED       "\033[41m"
#define COLOUR_BG_GREEN     "\033[42m"
#define COLOUR_BG_YELLOW    "\033[43m"
#define COLOUR_BG_BLUE      "\033[44m"
#define COLOUR_BG_PURPLE    "\033[45m"
#define COLOUR_BG_CYAN      "\033[46m"
#define COLOUR_BG_LIGHT_GREY "\033[47m"

#define COLOUR_NORMAL       "\033[0m"
#define COLOUR_BOLD         "\033[1m"
#define COLOUR_UNDERLINE    "\033[4m"
#define COLOUR_BLINK        "\033[5m"
#define COLOUR_REVERSE_VIDEO "\033[7m"

#else

#define COLOUR_BLACK
#define COLOUR_RED
#define COLOUR_GREEN
#define COLOUR_YELLOW
#define COLOUR_BLUE
#define COLOUR_PURPLE
#define COLOUR_CYAN 
#define COLOUR_LIGHT_GREY

#define COLOUR_BG_BLACK
#define COLOUR_BG_RED
#define COLOUR_BG_GREEN
#define COLOUR_BG_YELLOW
#define COLOUR_BG_BLUE
#define COLOUR_BG_PURPLE
#define COLOUR_BG_CYAN
#define COLOUR_BG_LIGHT_GREY

#define COLOUR_NORMAL
#define COLOUR_BOLD 
#define COLOUR_UNDERLINE
#define COLOUR_BLINK
#define COLOUR_REVERSE_VIDEO
#endif


#define NORM            COLOUR_NORMAL
#define DATA            COLOUR_GREEN
#define ADDRESS         COLOUR_YELLOW
#define VERBOSE         COLOUR_CYAN
#define MEMCONTENTS     COLOUR_PURPLE

        // Colour of error messages
#define ERRORCOL        COLOUR_BG_RED COLOUR_BLACK COLOUR_BLINK ""
        // Colour of warnings
#define WARNINGCOL      COLOUR_BG_YELLOW COLOUR_BLACK COLOUR_BLINK ""
        // Colour of the line that announces a DMA tag
#define DMATITLE        COLOUR_BG_GREEN COLOUR_BLACK

        // Colour of the line that announces a VIF code
#define VIFTITLE        COLOUR_BG_YELLOW COLOUR_BLACK
        // Colour of the VIF verbose field names
#define VIFFIELDTITLE   COLOUR_YELLOW

        // Colour of the line that announces a GIFTAG
#define GIFTAGTITLE     COLOUR_BG_CYAN COLOUR_BLACK
        // Colour of the verbose form of the Register name
#define GSREGISTERTITLE     COLOUR_BG_PURPLE COLOUR_BLACK
        // Colour of the verbose field names
#define GSFIELDTITLE    COLOUR_YELLOW
        // Colour of the verbose field names
#define GSVERBOSEFIELDTITLE NORM

#define DATA_FLOAT      DATA "%s" NORM
#define DATA_DEC        DATA "%d" NORM
#define DATA_HEX        DATA "%x" NORM
#define DATA_2HEX       DATA "%02x" NORM
#define DATA_4HEX       DATA "%04x" NORM
#define DATA_6HEX       DATA "%06x" NORM
#define DATA_8HEX       DATA "%08x" NORM
#define DATA_STR        DATA "%s" NORM
#define VERBOSE_STR     VERBOSE "%s" NORM
#define MEMCONTENTS_32  MEMCONTENTS "%08x" NORM
#define MEMCONTENTS_64  MEMCONTENTS "%08x.%08x" NORM
#define MEMCONTENTS_128 MEMCONTENTS "%08x.%08x.%08x.%08x" NORM
#define ADDRESS_HEX     COLOUR_YELLOW "%08x" NORM

#define NAME_DATA_FLOAT "%s="DATA_FLOAT
#define NAME_DATA_DEC   "%s="DATA_DEC
#define NAME_DATA_HEX   "%s="DATA_HEX
#define NAME_DATA_2HEX  "%s="DATA_2HEX
#define NAME_DATA_4HEX  "%s="DATA_4HEX
#define NAME_DATA_8HEX  "%s="DATA_8HEX
#define NAME_DATA_STR   "%s="DATA_STR
#define VERBOSEA ":("VERBOSE_STR")"

    // 20 spaces
#define INDENT "                    "


typedef enum {
    NO_ERROR = 0,
    ERROR_FATAL_CONT = 2,       // A fatal error was found, but it's possible to keep going and look for more errors.
    ERROR_FATAL_HALT = 4,       // A fatal error was found, and it means it's not possible to look ahead for more errors.
    ERROR_WARNING = 8
} DMA_ERROR_REPORTING;

#define FATAL(s) (((s) & (ERROR_FATAL_HALT | ERROR_FATAL_CONT)) != 0)
#define WARNING(s) (((s) & ERROR_WARNING) != 0)

typedef enum {
    ERR__start                      = 0,
    ERR_NULL_NEXTTAG                = 10, // attempted to obtain 'nextTag' but it was unexpectedly NULL.
    ERR_RESERVED_PCE                = 20, // PCE field is set to 'reserved' value.
    ERR_DMA_ALIGNMENT               = 30, // dma tag is not quad/word aligned.
    ERR_DMA_INVALID_ADDRESS         = 40, // dma tag does not lie within main memory or scratch pad!
    ERR_CALLSTACK_OVERFLOW          = 60, // CALL tag encountered, but callstack is full!
    ERR_INVALID_TAG_FOR_CHANNEL     = 70, // This tag cannot be used on this DMA channel
    ERR_CANT_FIND_TAG_WITH_DATA     = 80, // Cannot find any DMA tag in the entire list which has data to transfer
    ERR_INSUFFICIENT_DATA           = 90, // Need more data, but the DMA chain has terminated or an error occurred
    ERR_DMA_INVALID_DIR             = 100, // Dn_CHCR.DIR mode is not handled
    ERR_DMA_INVALID_MOD             = 110, // Dn_CHCR.MOD > 1 is not handled
    ERR_DMA_OPENFILE                = 120, // Could not open a DMA virtual file
    ERR_DMA_INVALID_FEATURE         = 130, // This feature does not exist on this DMA channel
    ERR_DMA_INVALID_CHANNEL         = 140, // Cannot disassemble this channel
    ERR_DMA_INVALID_TQWC            = 150, // Interleave: TQWC is 0

        // VIF related errors
    ERR_VIF_CURSOR_AT_END           = 300, // VIF cursor tried to move, but couldn't because there was insufficient data
    ERR_VIF_UNKNOWN_UNPACK_FORMAT   = 310, // Unknown UNPACK format in VIF code.
    ERR_VIF_MPG_ALIGNMENT           = 320, // MPG statement is not properly aligned.
    ERR_VIF_LIST_ALIGNMENT          = 330, // VIF list is not aligned on a 4 byte boundary
    ERR_VIF_UNKNOWN_VIFCODE         = 340, // An unknown VIF code was encountered
    ERR_VIF_INVALID_VIFCODE         = 350, // An valid VIF code was found on a VIF channel (i.e. 0) that does not support it.
    ERR_VIF_INVALID_CHANNEL         = 360, // You are trying to decode via a channel which is not SCE_DMA_VIF[0/1]
    ERR_VIF_INVALID_ADDRESS         = 370, // You are trying to Unpack to an invalid VU memory address
    ERR_VIF_INVALID_MODE            = 380, // MODE register is set to 3, which is 'undefined'.
    ERR_VIF_INVALID_CYCLE           = 390, // The CYCLE register has an invalid setting.


    ERR_GIF_INVALID_PRIM            = 700, // An invalid primitive type was specified in the PRIM field.
    ERR_GIF_INVALID_REG             = 710, // An invalid register number was specified in a REGS field.
    ERR_GIF_INSUFFICIENT_DATA       = 720, // Tried to examine a tag or register setting, but the DMA
                                            // ended prematurely.
    ERR_GIF_LIST_ALIGNMENT          = 740, // GIF list is not aligned on a 4 byte boundary
    ERR_GIF_INVALID_DIR             = 750, // DIR value is not handled
    ERR_GIF_INVALID_ADDR            = 760, // GIFTAG or register is at an invalid address

    ERR___end
} DMA_ERROR;

typedef enum {
    WARN__start = 1000,
    WARN_UNUSED_TAG_BITS_NON_ZERO   = 1010, // Unused bits in the DMA tag are not zero - possibly
                                            // because >= 1MB is trying to be transferred.
    WARN_ADDR_ALIGNMENT             = 1020, // 'ADDR' field in tag is not aligned to quadword boundary
    WARN_PREMATURE_END              = 1030, // tag encountered specifying the end of the
                                            // DMA transfer, but call stack not empty
    WARN_DMA_LOOKS_SPR              = 1040, // DMA address looks like a SPR address, but bit 31 is not set
    WARN_DMA_NORMAL_QWC_ZERO        = 1050, // 'Normal' DMA started, but QWC == 0
    WARN_DMA_ASP_NOT_ZERO           = 1060, // Source Chain started with CHCR.ASP != 0
    WARN_DMA_TTE_IN_NORM_MODE       = 1070, // TTE is set to 1 in Normal transfer mode
    WARN_DMA_TTE_IN_GIF_CHANNEL     = 1080, // TTE is set to 1 for Source Chain transfer to GIF channel
    WARN_DMA_QWC_NOT_ZERO           = 1090, // Source Chain mode transfer started, but QWC != 0.
    WARN_DMA_INTERLEAVE_UNBALANCED  = 1100, // DMA transfer ended and interleave mode was used, but
                                            // number of QW transferred was not a multiple of TQWC.
    WARN_DMA_INVALID_SQWC           = 1110, // Interleave: SQWC = 0, so there effectively wont be interleaving

    WARN_VIF_INDETERMINATE_WRITE    = 3000, // Indeterminate word is being written by UNPACK
    WARN_VIF_DIRECT_INCOMPLETE      = 3010, // GIF is awaiting more data when DIRECT/DIRECTHL ends

    WARN_GIF_PRE_DISABLED           = 5000, // PRE bit is set in a GIFTAG, but mode is not PACKED.
    WARN_GIF_SMALL_NREG             = 5010, // Registers are set, but NREG is not high enough to use them
    WARN_GIF_INV_FIELD              = 5030, // An invalid field has been found in a GS Register
    WARN_GIF_TTE_ON                 = 5040, // TTE bit is enabled for a transfer thru PATH3
    WARN_GIF_LAST_EOP_ZERO          = 5050, // EOP bit was not set in the last GIF tag.
    WARN_GIF_REGLIST_A_D            = 5060, // A+D field is in one of the REGS fields in REGLIST mode
    WARN_GIF_PREMATURE_END          = 5070, // DMA ends but LOOPCNT is not 0 (GIF is still expecting words).
    WARN_GIF_INVALID_REGISTER       = 5080, // The disassembler encountered a reference to an invalid GS register.
                                            // This will be treated as a NOP
    WARN__end
} DMA_WARNING;

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern char gErrorBuffer[];
extern char gTempString[];
extern char *gOutputBuffer;

void DEBUG_parseConfig(char *initString);
int DEBUG_addError(int errNum, int severity, char *errString);
int DEBUG_addToOutputBuffer(char *s);
char *DEBUG_padWithSpaces(char *s, int totalWidth);
int DEBUG_strlen(char *s);
void DEBUG_addField(char *final, char *fmt, ...);
void DEBUG_saveBuffers(char *filename, BOOL append);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
#ifndef DMAEMU_H
#define DMAEMU_H

#define MEM_MAX_32 (0x01ffffff)
#define MEM_MAX_128 (0x07ffffff)

typedef enum {
    DMA_REFE = 0,
    DMA_CNT,
    DMA_NEXT,
    DMA_REF,
    DMA_REFS,
    DMA_CALL,
    DMA_RET,
    DMA_END
} DMAID;

// ============================================================
// ============================================================

// =====================================================
#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

    // Globally accessible QWord which is what the DMA emulator puts onto the 'bus'.
extern int gChan;
extern u_int *gDmaTransferQWord;
extern BOOL gDmaTransferQWordIsTag;     // Is this word a DMA tag?

        // Upper limit on memory size, used for bounds checking
extern u_int gUpperMainMemLimit;

int DMA_getCRC(u_int *result);

BOOL DMA_isValidAddress(void *addr);
void *DMA_convertToMainMemAddress(void *addr);
void DMA_parseConfig(char *cs);
int DMA_getDmaChannelNumber(sceDmaChan *d);
int DMA_emulateDMACycle(void);
int DMA_startDmaTransfer(int channelNum, u_int chcr);
int DMA_sceDmaSend(sceDmaChan *dc, void *tag);
int DMA_sceDmaSendN(sceDmaChan *dc, void *addr, int size);

void DMA_init(char *cs);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
#ifndef GIFCURSOR_H
#define GIFCURSOR_H



#define GIFTAG_PACKED (0)
#define GIFTAG_REGLIST (1)
#define GIFTAG_IMAGE (2)
#define GIFTAG_DISABLE (3)

#define NUMGIFREGS (71)

typedef enum {
    REGDESC_PRIM    = 0,
    REGDESC_RGBAQ   = 1,
    REGDESC_ST      = 2,
    REGDESC_UV      = 3,
    REGDESC_XYZF2   = 4,
    REGDESC_XYZ2    = 5,
    REGDESC_TEX0_1  = 6,
    REGDESC_TEX0_2  = 7,
    REGDESC_CLAMP_1 = 8,
    REGDESC_CLAMP_2 = 9,
    REGDESC_FOG     = 0xa,
    REGDESC_RESERVED= 0xb,
    REGDESC_XYZF3   = 0xc,
    REGDESC_XYZ3    = 0xd,
    REGDESC_A_D     = 0xe,
    REGDESC_NOP     = 0xf
} REGDESC;

    // -------------------------------------

typedef struct {
    u_int *tagaddr;         // RAM address of this GIF tag

    u_int nloop;
    BOOL eop;
    BOOL pre;
    u_int prim;
    u_int flg;
    u_int nreg;
    u_int regs[2];
    char regidx[16];
} GIFTAG;

// ============================================================
// ============================================================


// ============================================================
#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

extern u_int *gGifInput;

void GIF_parseConfig(char *cs);
void GIF_saveGifRegisters(void);
BOOL GIF_isIdle(void);
int GIF_emulateGifCycle(void);
int GIF_disassemble(void);
int GIF_disassembleDmaList(sceDmaChan *d, void *tag);
int GIF_disassembleDmaListN(sceDmaChan *d, void *tag, int numqwords);
int GIF_disassembleXGKICK(void *addr);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
#ifndef VIFCURSOR_H
#define VIFCURSOR_H


typedef enum {
    VIF_NOPc =      0x00,
    VIF_STCYCLc =   0x01,
    VIF_OFFSETc =   0x02,
    VIF_BASEc =     0x03,
    VIF_ITOPc =     0x04,
    VIF_STMODc =    0x05,
    VIF_MSKPATH3c = 0x06,
    VIF_MARKc =     0x07,
    VIF_FLUSHEc =   0x10,
    VIF_FLUSHc =    0x11,
    VIF_FLUSHAc =   0x13,
    VIF_MSCALc =    0x14,
    VIF_MSCNTc =    0x17,
    VIF_MSCALFc =   0x15,
    VIF_STMASKc =   0x20,
    VIF_STROWc =    0x30,
    VIF_STCOLc =    0x31,
    VIF_MPGc =      0x4A,
    VIF_DIRECTc =   0x50,
    VIF_DIRECTHLc = 0x51,
    VIF_UNPACKc =   0x60
} VIFCODESc;

typedef enum {
    VIF_NOP = 0,
    VIF_STCYCL,
    VIF_OFFSET,
    VIF_BASE,
    VIF_ITOP,
    VIF_STMOD,
    VIF_MSKPATH3,
    VIF_MARK,
    VIF_FLUSHE,
    VIF_FLUSH,
    VIF_FLUSHA,
    VIF_MSCAL,
    VIF_MSCNT,
    VIF_MSCALF,
    VIF_STMASK,
    VIF_STROW,
    VIF_STCOL,
    VIF_MPG,
    VIF_DIRECT,
    VIF_DIRECTHL,
    VIF_UNPACK,
    NUMVIFCODES
} VIFCODESi;

typedef enum {
    UNPACK_S_32c    = 0x0,
    UNPACK_S_16c    = 0x1,
    UNPACK_S_8c     = 0x2,
    UNPACK_V2_32c   = 0x4,
    UNPACK_V2_16c   = 0x5,
    UNPACK_V2_8c    = 0x6,
    UNPACK_V3_32c   = 0x8,
    UNPACK_V3_16c   = 0x9,
    UNPACK_V3_8c    = 0xA,
    UNPACK_V4_32c   = 0xC,
    UNPACK_V4_16c   = 0xD,
    UNPACK_V4_8c    = 0xE,
    UNPACK_V4_5c    = 0xF
} UNPACKCODESc;

typedef enum {
    UNPACK_S_32 = 0,
    UNPACK_S_16,
    UNPACK_S_8,
    UNPACK_V2_32,
    UNPACK_V2_16,
    UNPACK_V2_8,
    UNPACK_V3_32,
    UNPACK_V3_16,
    UNPACK_V3_8,
    UNPACK_V4_32,
    UNPACK_V4_16,
    UNPACK_V4_8,
    UNPACK_V4_5,
} UNPACKCODESi;


    // -------------------------------------

typedef struct {
    u_int *addr;            // RAM address of this code

    u_char rawCmd;          // The full CMD field
//  u_char cmd;             // The processed CMD field (ie with UNPACK format + IRQ masked)
    int cmdIndex;           // The index of the command

    char interruptBit;      // Status of the interrupt bit
    int unpackFormatIndex;  // If an UNPACK command, this is the UNPACK format.

    int num;                // The num field
    u_int imm;              // The imm field

    int length;             // The number of words in this VIF packet.
} VIFCODE;

// ============================================================
// ============================================================
#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

void VIF_saveVifRegisters(void);
void VIF_parseConfig(char *cs);
int VIF_disassemble(void);
int VIF_disassembleDmaList(sceDmaChan *d, void *tag);
int VIF_disassembleDmaListN(sceDmaChan *d, void *tag, int numqwords);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif


#endif
#ifndef DISASSEMBLE_H
#define DISASSEMBLE_H

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int DIS_sceDmaSend(sceDmaChan *d, void *tag);
int DIS_sceDmaSendN(sceDmaChan *d, void *tag, u_int numqwords);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif
