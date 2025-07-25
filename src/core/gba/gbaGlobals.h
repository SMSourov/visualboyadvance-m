#ifndef VBAM_CORE_GBA_GBAGLOBALS_H_
#define VBAM_CORE_GBA_GBAGLOBALS_H_

#include "core/gba/gba.h"

#define VERBOSE_SWI 1
#define VERBOSE_UNALIGNED_MEMORY 2
#define VERBOSE_ILLEGAL_WRITE 4
#define VERBOSE_ILLEGAL_READ 8
#define VERBOSE_DMA0 16
#define VERBOSE_DMA1 32
#define VERBOSE_DMA2 64
#define VERBOSE_DMA3 128
#define VERBOSE_UNDEFINED 256
#define VERBOSE_AGBPRINT 512
#define VERBOSE_SOUNDOUTPUT 1024

#define GBA_MATRIX_MAPPINGS_MAX 16

typedef struct GBAMatrix {
    uint32_t cmd;
    uint32_t paddr;
    uint32_t vaddr;
    uint32_t size;

    uint32_t mappings[GBA_MATRIX_MAPPINGS_MAX];
} GBAMatrix_t;

extern GBAMatrix_t GBAMatrix;

extern void GBAMatrixReset(GBAMatrix_t *matrix);
extern void GBAMatrixWrite(GBAMatrix_t *matrix, uint32_t address, uint32_t value);
extern void GBAMatrixWrite16(GBAMatrix_t *matrix, uint32_t address, uint16_t value);

extern reg_pair reg[45];
extern bool ioReadable[0x400];
extern bool N_FLAG;
extern bool C_FLAG;
extern bool Z_FLAG;
extern bool V_FLAG;
extern bool armState;
extern bool armIrqEnable;
extern uint32_t armNextPC;
extern int armMode;
extern uint32_t stop;
extern bool gba_joybus_enabled;
extern bool gba_joybus_active;
extern int customBackdropColor;

extern uint8_t* g_bios;
extern uint8_t* g_rom;
extern uint8_t* g_rom2;
extern uint8_t* g_internalRAM;
extern uint8_t* g_workRAM;
extern uint8_t* g_paletteRAM;
extern uint8_t* g_vram;
extern uint8_t* g_pix;
extern uint8_t* g_oam;
extern uint8_t* g_ioMem;

extern uint16_t DISPCNT;
extern uint16_t DISPSTAT;
extern uint16_t VCOUNT;
extern uint16_t BG0CNT;
extern uint16_t BG1CNT;
extern uint16_t BG2CNT;
extern uint16_t BG3CNT;
extern uint16_t BG0HOFS;
extern uint16_t BG0VOFS;
extern uint16_t BG1HOFS;
extern uint16_t BG1VOFS;
extern uint16_t BG2HOFS;
extern uint16_t BG2VOFS;
extern uint16_t BG3HOFS;
extern uint16_t BG3VOFS;
extern uint16_t BG2PA;
extern uint16_t BG2PB;
extern uint16_t BG2PC;
extern uint16_t BG2PD;
extern uint16_t BG2X_L;
extern uint16_t BG2X_H;
extern uint16_t BG2Y_L;
extern uint16_t BG2Y_H;
extern uint16_t BG3PA;
extern uint16_t BG3PB;
extern uint16_t BG3PC;
extern uint16_t BG3PD;
extern uint16_t BG3X_L;
extern uint16_t BG3X_H;
extern uint16_t BG3Y_L;
extern uint16_t BG3Y_H;
extern uint16_t WIN0H;
extern uint16_t WIN1H;
extern uint16_t WIN0V;
extern uint16_t WIN1V;
extern uint16_t WININ;
extern uint16_t WINOUT;
extern uint16_t MOSAIC;
extern uint16_t BLDMOD;
extern uint16_t COLEV;
extern uint16_t COLY;
extern uint16_t DM0SAD_L;
extern uint16_t DM0SAD_H;
extern uint16_t DM0DAD_L;
extern uint16_t DM0DAD_H;
extern uint16_t DM0CNT_L;
extern uint16_t DM0CNT_H;
extern uint16_t DM1SAD_L;
extern uint16_t DM1SAD_H;
extern uint16_t DM1DAD_L;
extern uint16_t DM1DAD_H;
extern uint16_t DM1CNT_L;
extern uint16_t DM1CNT_H;
extern uint16_t DM2SAD_L;
extern uint16_t DM2SAD_H;
extern uint16_t DM2DAD_L;
extern uint16_t DM2DAD_H;
extern uint16_t DM2CNT_L;
extern uint16_t DM2CNT_H;
extern uint16_t DM3SAD_L;
extern uint16_t DM3SAD_H;
extern uint16_t DM3DAD_L;
extern uint16_t DM3DAD_H;
extern uint16_t DM3CNT_L;
extern uint16_t DM3CNT_H;
extern uint16_t TM0D;
extern uint16_t TM0CNT;
extern uint16_t TM1D;
extern uint16_t TM1CNT;
extern uint16_t TM2D;
extern uint16_t TM2CNT;
extern uint16_t TM3D;
extern uint16_t TM3CNT;
extern uint16_t P1;
extern uint16_t IE;
extern uint16_t IF;
extern uint16_t IME;

#endif // VBAM_CORE_GBA_GBAGLOBALS_H_
