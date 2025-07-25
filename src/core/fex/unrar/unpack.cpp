#include "rar.hpp"

#define _RAR_UNPACK_CPP_

#include "coder.cpp"
#include "suballoc.cpp"
#include "model.cpp"
#include "unpackinline.cpp"
#ifndef SFX_MODULE
#include "unpack15.cpp"
#include "unpack20.cpp"
#endif
#include "unpack30.cpp"
#include "unpack50.cpp"
#include "unpack50frag.cpp"

Unpack::Unpack(ComprDataIO *DataIO)
	: Inp(true), VMCodeInp(true)
{
	LastFilter = 0;
	UnpIO=DataIO;
	Window=NULL;
    Fragmented=false;
    Suspended=false;
	UnpAllBuf=false;
	UnpSomeRead=false;
    MaxWinSize=0;
    MaxWinMask=0;
}

void Unpack::init_tables()
{
    ComprDataIO IOTemp;
    Unpack Temp(&IOTemp);
    // Perform initialization, which should be done only once for all files.
    // It prevents crash if first DoUnpack call is later made with wrong
    // (true) 'Solid' value.
    Temp.UnpInitData(false);
#ifndef SFX_MODULE
    // RAR 1.5 decompression initialization
    Temp.UnpInitData15(false);
    Temp.InitHuff();
#endif
}


Unpack::~Unpack()
{
	if (Window!=NULL)
		rarfree( Window );
    InitFilters30();
}


void Unpack::Init(size_t WinSize,bool Solid)
{
    // If 32-bit RAR unpacks an archive with 4 GB dictionary, the window size
    // will be 0 because of size_t overflow. Let's issue the memory error.
    if (WinSize==0)
        throw std::bad_alloc();

    // Minimum window size must be at least twice more than maximum possible
    // size of filter block, which is 0x10000 in RAR now. If window size is
    // smaller, we can have a block with never cleared flt->NextWindow flag
    // in UnpWriteBuf(). Minimum window size 0x20000 would be enough, but let's
    // use 0x40000 for extra safety and possible filter area size expansion.
    const size_t MinAllocSize=0x40000;
    if (WinSize<MinAllocSize)
        WinSize=MinAllocSize;

    if (WinSize<=MaxWinSize) // Use the already allocated window.
        return;
    if ((WinSize>>16)>0x10000) // Window size must not exceed 4 GB.
        return;

    // Archiving code guarantees that window size does not grow in the same
    // solid stream. So if we are here, we are either creating a new window
    // or increasing the size of non-solid window. So we could safely reject
    // current window data without copying them to a new window, though being
    // extra cautious, we still handle the solid window grow case below.
    bool Grow=Solid && (Window!=NULL || Fragmented);

    // We do not handle growth for existing fragmented window.
    if (Grow && Fragmented)
        throw std::bad_alloc();

    byte *NewWindow=Fragmented ? NULL : (byte *)malloc(WinSize);

    if (NewWindow==NULL) {
        if (Grow || WinSize<0x1000000)
        {
            // We do not support growth for new fragmented window.
            // Also exclude RAR4 and small dictionaries.
            throw std::bad_alloc();
        }
        else
        {
            if (Window!=NULL) // If allocated by preceding files.
            {
                free(Window);
                Window=NULL;
            }
            FragWindow.Init(WinSize);
            Fragmented=true;
        }
    }

    if (!Fragmented)
	{
        // Clean the window to generate the same output when unpacking corrupt
        // RAR files, which may access to unused areas of sliding dictionary.
        memset(NewWindow,0,WinSize);
        
        // If Window is not NULL, it means that window size has grown.
        // In solid streams we need to copy data to a new window in such case.
        // RAR archiving code does not allow it in solid streams now,
        // but let's implement it anyway just in case we'll change it sometimes.
        if (Grow)
            for (size_t I=1;I<MaxWinSize;I++)
                NewWindow[(UnpPtr-I)&(WinSize-1)]=Window[(UnpPtr-I)&(MaxWinSize-1)];

        if (Window!=NULL)
            free(Window);
        Window=NewWindow;
	}

    MaxWinSize=WinSize;
    MaxWinMask=MaxWinSize-1;
}


void Unpack::DoUnpack(int Method,bool Solid)
{
	switch(Method)
	{
#ifndef SFX_MODULE
		case 15: // rar 1.5 compression
			Unpack15(Solid);
			break;
		case 20: // rar 2.x compression
		case 26: // files larger than 2GB
			Unpack20(Solid);
			break;
#endif
		case 29: // rar 3.x compression
			Unpack29(Solid);
			break;
        case 0: // RAR 5.0 compression algorithm 0.
            Unpack5(Solid);
            break;
    }
}


void Unpack::UnpInitData(bool Solid)
{
    if (!Solid)
    {
        memset(OldDist,0,sizeof(OldDist));
        OldDistPtr=0;
        LastDist=LastLength=0;
        //    memset(Window,0,MaxWinSize);
        memset(&BlockTables,0,sizeof(BlockTables));
        UnpPtr=WrPtr=0;
        WriteBorder=Min(MaxWinSize,UNPACK_MAX_WRITE)&MaxWinMask;
        
        InitFilters();
    }
    Inp.InitBitInput();
    WrittenFileSize=0;
    ReadTop=0;
    ReadBorder=0;
    
    memset(&BlockHeader,0,sizeof(BlockHeader));
    BlockHeader.BlockSize=-1;  // '-1' means not defined yet.
#ifndef SFX_MODULE
    UnpInitData20(Solid);
#endif
    UnpInitData30(Solid);
}


// LengthTable contains the length in bits for every element of alphabet.
// Dec is the structure to decode Huffman code/
// Size is size of length table and DecodeNum field in Dec structure,
void Unpack::MakeDecodeTables(byte *LengthTable,DecodeTable *Dec,uint Size)
{
    // Size of alphabet and DecodePos array.
    Dec->MaxNum=Size;
    
    // Calculate how many entries for every bit length in LengthTable we have.
    uint LengthCount[16];
    memset(LengthCount,0,sizeof(LengthCount));
    for (size_t I=0;I<Size;I++)
        LengthCount[LengthTable[I] & 0xf]++;
    
    // We must not calculate the number of zero length codes.
    LengthCount[0]=0;
    
    // Set the entire DecodeNum to zero.
    memset(Dec->DecodeNum,0,Size*sizeof(*Dec->DecodeNum));
    
    // Initialize not really used entry for zero length code.
    Dec->DecodePos[0]=0;
    
    // Start code for bit length 1 is 0.
    Dec->DecodeLen[0]=0;
    
    // Right aligned upper limit code for current bit length.
    uint UpperLimit=0;
    
    for (size_t I=1;I<16;I++)
    {
        // Adjust the upper limit code.
        UpperLimit+=LengthCount[I];
        
        // Left aligned upper limit code.
        uint LeftAligned=UpperLimit<<(16-I);
        
        // Prepare the upper limit code for next bit length.
        UpperLimit*=2;
        
        // Store the left aligned upper limit code.
        Dec->DecodeLen[I]=(uint)LeftAligned;
        
        // Every item of this array contains the sum of all preceding items.
        // So it contains the start position in code list for every bit length.
        Dec->DecodePos[I]=Dec->DecodePos[I-1]+LengthCount[I-1];
    }
    
    // Prepare the copy of DecodePos. We'll modify this copy below,
    // so we cannot use the original DecodePos.
    uint CopyDecodePos[16];
    memcpy(CopyDecodePos,Dec->DecodePos,sizeof(CopyDecodePos));
    
    // For every bit length in the bit length table and so for every item
    // of alphabet.
    for (uint I=0;I<Size;I++)
    {
        // Get the current bit length.
        byte CurBitLength=LengthTable[I] & 0xf;
        
        if (CurBitLength!=0)
        {
            // Last position in code list for current bit length.
            uint LastPos=CopyDecodePos[CurBitLength];
            
            // Prepare the decode table, so this position in code list will be
            // decoded to current alphabet item number.
            Dec->DecodeNum[LastPos]=(ushort)I;
            
            // We'll use next position number for this bit length next time.
            // So we pass through the entire range of positions available
            // for every bit length.
            CopyDecodePos[CurBitLength]++;
        }
    }
    
    // Define the number of bits to process in quick mode. We use more bits
    // for larger alphabets. More bits means that more codes will be processed
    // in quick mode, but also that more time will be spent to preparation
    // of tables for quick decode.
    switch (Size)
    {
        case NC:
        case NC20:
        case NC30:
            Dec->QuickBits=MAX_QUICK_DECODE_BITS;
            break;
        default:
            Dec->QuickBits=MAX_QUICK_DECODE_BITS-3;
            break;
    }
    
    // Size of tables for quick mode.
    uint QuickDataSize=1<<Dec->QuickBits;
    
    // Bit length for current code, start from 1 bit codes. It is important
    // to use 1 bit instead of 0 for minimum code length, so we are moving
    // forward even when processing a corrupt archive.
    uint CurBitLength=1;
    
    // For every right aligned bit string which supports the quick decoding.
    for (uint Code=0;Code<QuickDataSize;Code++)
    {
        // Left align the current code, so it will be in usual bit field format.
        uint BitField=Code<<(16-Dec->QuickBits);
        
        // Prepare the table for quick decoding of bit lengths.
        
        // Find the upper limit for current bit field and adjust the bit length
        // accordingly if necessary.
        while (BitField>=Dec->DecodeLen[CurBitLength] && CurBitLength<ASIZE(Dec->DecodeLen))
            CurBitLength++;
        
        // Translation of right aligned bit string to bit length.
        Dec->QuickLen[Code]=CurBitLength;
        
        // Prepare the table for quick translation of position in code list
        // to position in alphabet.
        
        // Calculate the distance from the start code for current bit length.
        uint Dist=BitField-Dec->DecodeLen[CurBitLength-1];
        
        // Right align the distance.
        Dist>>=(16-CurBitLength);
        
        // Now we can calculate the position in the code list. It is the sum
        // of first position for current bit length and right aligned distance
        // between our bit field and start code for current bit length.
        uint Pos=Dec->DecodePos[CurBitLength]+Dist;
        
        if (Pos<Size) // Safety check for damaged archives.
        {
            // Define the code to alphabet number translation.
            Dec->QuickNum[Code]=Dec->DecodeNum[Pos];
        }
        else
            Dec->QuickNum[Code]=0;
    }
}
