#ifndef VBAM_CORE_GBA_GBACHEATS_H_
#define VBAM_CORE_GBA_GBACHEATS_H_

#include <cstdint>

#include "core/base/system.h"

#if !defined(__LIBRETRO__)
#include <zlib.h>
#endif  // defined(__LIBRETRO__)

#include "core/rust/bindings.hpp"

void cheatsAdd(const char* codeStr, const char* desc, uint32_t rawaddress, uint32_t address, uint32_t value,
    int code, int size);
void cheatsAddCheatCode(const char* code, const char* desc);
void cheatsAddGSACode(const char* code, const char* desc, bool v3);
void cheatsAddCBACode(const char* code, const char* desc);
bool cheatsImportGSACodeFile(const char* name, int game, bool v3);
void cheatsDelete(int number, bool restore);
void cheatsDeleteAll(bool restore);
void cheatsEnable(int number);
void cheatsDisable(int number);
#ifndef __LIBRETRO__
void cheatsSaveGame(gzFile file);
void cheatsReadGame(gzFile file, int version);
void cheatsReadGameSkip(gzFile file, int version);
void cheatsSaveCheatList(const char* file);
bool cheatsLoadCheatList(const char* file);
#endif
#ifdef VBAM_ENABLE_DEBUGGER
void cheatsWriteMemory(uint32_t address, uint32_t value);
void cheatsWriteHalfWord(uint32_t address, uint16_t value);
void cheatsWriteByte(uint32_t address, uint8_t value);
#endif
int cheatsCheckKeys(uint32_t keys, uint32_t extended);

extern int cheatsNumber;
extern core::CheatsData cheatsList[MAX_CHEATS];

#endif // VBAM_CORE_GBA_GBACHEATS_H_
