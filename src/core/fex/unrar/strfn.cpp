#include "rar.hpp"

#include <ctype.h>

const char *NullToEmpty(const char *Str)
{
  return Str==NULL ? "":Str;
}

static const wchar Empty[] = {0};

const wchar *NullToEmpty(const wchar *Str)
{
  return Str==NULL ? Empty:Str;
}


void IntToExt(const char *Src,char *Dest,size_t DestSize)
{
#ifdef _WIN_ALL
  OemToCharBuffA(Src,Dest,(DWORD)DestSize);
  Dest[DestSize-1]=0;
#elif defined(_ANDROID)
  wchar DestW[NM];
  UnkToWide(Src,DestW,ASIZE(DestW));
  WideToChar(DestW,Dest,DestSize);
#else
  if (Dest!=Src)
    strncpyz(Dest,Src,DestSize);
#endif
}


int stricomp(const char *s1,const char *s2)
{
#ifdef _WIN_ALL
  return CompareStringA(LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,s1,-1,s2,-1)-2;
#else
  while (toupper(*s1)==toupper(*s2))
  {
    if (*s1==0)
      return 0;
    s1++;
    s2++;
  }
  return s1 < s2 ? -1 : 1;
#endif
}


int strnicomp(const char *s1,const char *s2,size_t n)
{
#ifdef _WIN_ALL
  // If we specify 'n' exceeding the actual string length, CompareString goes
  // beyond the trailing zero and compares garbage. So we need to limit 'n'
  // to real string length.
  // It is important to use strnlen (or memchr(...,0)) instead of strlen,
  // because data can be not zero terminated.
  size_t l1=Min(strnlen(s1,n),n);
  size_t l2=Min(strnlen(s2,n),n);
  return CompareStringA(LOCALE_USER_DEFAULT,NORM_IGNORECASE|SORT_STRINGSORT,s1,(int)l1,s2,(int)l2)-2;
#else
  if (n==0)
    return 0;
  while (toupper(*s1)==toupper(*s2))
  {
    if (*s1==0 || --n==0)
      return 0;
    s1++;
    s2++;
  }
  return s1 < s2 ? -1 : 1;
#endif
}


wchar* RemoveEOL(wchar *Str)
{
  for (int I=(int)my_wcslen(Str)-1;I>=0 && (Str[I]=='\r' || Str[I]=='\n' || Str[I]==' ' || Str[I]=='\t');I--)
    Str[I]=0;
  return Str;
}


wchar* RemoveLF(wchar *Str)
{
  for (int I=(int)my_wcslen(Str)-1;I>=0 && (Str[I]=='\r' || Str[I]=='\n');I--)
    Str[I]=0;
  return(Str);
}


unsigned char loctolower(unsigned char ch)
{
#ifdef _WIN_ALL
  // Convert to LPARAM first to avoid a warning in 64 bit mode.
  return((int)(LPARAM)CharLowerA((LPSTR)ch));
#else
  return(tolower(ch));
#endif
}


unsigned char loctoupper(unsigned char ch)
{
#ifdef _WIN_ALL
  // Convert to LPARAM first to avoid a warning in 64 bit mode.
  return((int)(LPARAM)CharUpperA((LPSTR)ch));
#else
  return(toupper(ch));
#endif
}


// toupper with English only results if English input is provided.
// It avoids Turkish (small i) -> (big I with dot) conversion problem.
// We do not define 'ch' as 'int' to avoid necessity to cast all
// signed chars passed to this function to unsigned char.
unsigned char etoupper(unsigned char ch)
{
  if (ch=='i')
    return('I');
  return(toupper(ch));
}


// Unicode version of etoupper.
wchar etoupperw(wchar ch)
{
  if (ch=='i')
    return('I');
#if defined(__APPLE__) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__linux__)
  return(toupper(ch));
#else
  return(toupperw(ch));
#endif
}


// We do not want to cast every signed char to unsigned when passing to
// isdigit, so we implement the replacement. Shall work for Unicode too.
// If chars are signed, conversion from char to int could generate negative
// values, resulting in undefined behavior in standard isdigit.
bool IsDigit(int ch)
{
  return(ch>='0' && ch<='9');
}


// We do not want to cast every signed char to unsigned when passing to
// isspace, so we implement the replacement. Shall work for Unicode too.
// If chars are signed, conversion from char to int could generate negative
// values, resulting in undefined behavior in standard isspace.
bool IsSpace(int ch)
{
  return(ch==' ' || ch=='\t');
}


// We do not want to cast every signed char to unsigned when passing to
// isalpha, so we implement the replacement. Shall work for Unicode too.
// If chars are signed, conversion from char to int could generate negative
// values, resulting in undefined behavior in standard function.
bool IsAlpha(int ch)
{
  return((ch>='A' && ch<='Z') || (ch>='a' && ch<='z'));
}




void BinToHex(const byte *Bin,size_t BinSize,char *HexA,wchar *HexW,size_t HexSize)
{
  uint A=0,W=0; // ASCII and Unicode hex output positions.
  for (uint I=0;I<BinSize;I++)
  {
    uint High=Bin[I] >> 4;
    uint Low=Bin[I] & 0xf;
    uint HighHex=High>9 ? 'a'+High-10:'0'+High;
    uint LowHex=Low>9 ? 'a'+Low-10:'0'+Low;
    if (HexA!=NULL && A<HexSize-2) // Need space for 2 chars and final zero.
    {
      HexA[A++]=(char)HighHex;
      HexA[A++]=(char)LowHex;
    }
    if (HexW!=NULL && W<HexSize-2) // Need space for 2 chars and final zero.
    {
      HexW[W++]=HighHex;
      HexW[W++]=LowHex;
    }
  }
  if (HexA!=NULL && HexSize>0)
    HexA[A]=0;
  if (HexW!=NULL && HexSize>0)
    HexW[W]=0;
}


#ifndef SFX_MODULE
uint GetDigits(uint Number)
{
  uint Digits=1;
  while (Number>=10)
  {
    Number/=10;
    Digits++;
  }
  return Digits;
}
#endif


bool LowAscii(const char *Str)
{
  for (int I=0;Str[I]!=0;I++)
    if ((byte)Str[I]<32 || (byte)Str[I]>127)
      return false;
  return true;
}


bool LowAscii(const wchar *Str)
{
  for (int I=0;Str[I]!=0;I++)
  {
    // We convert wchar_t to uint just in case if some compiler
    // uses signed wchar_t.
    if ((uint)Str[I]<32 || (uint)Str[I]>127)
      return false;
  }
  return true;
}


int wcsicompc(const wchar *Str1,const wchar *Str2)
{
#if defined(_UNIX) || defined(_MSC_VER) || defined(__APPLE__) || defined(__linux__)
  return my_wcscmp(Str1,Str2);
#elif defined(__MINGW32__)
  return _wcsicmp(Str1,Str2);
#else
  return wcsicomp(Str1,Str2);
#endif
}


// safe strncpy: copies maxlen-1 max and always returns zero terminated dest
char* strncpyz(char *dest, const char *src, size_t maxlen)
{
  if (maxlen>0)
  {
#if (_MSC_VER >= 1300) || __STDC_WANT_SECURE_LIB__
	strcpy_s(dest,maxlen-1,src);
#else
	strncpy(dest,src,maxlen-1);
#endif
    dest[maxlen-1]=0;
  }
  return dest;
}


// Safe wcsncpy: copies maxlen-1 max and always returns zero terminated dest.
wchar* wcsncpyz(wchar *dest, const wchar *src, size_t maxlen)
{
  if (maxlen>0)
  {
    my_wcsncpy(dest,src,maxlen-1);
    dest[maxlen-1]=0;
}
  return dest;
}


// Safe strncat: resulting dest length cannot exceed maxlen and dest 
// is always zero terminated. Note that 'maxlen' parameter defines the entire
// dest buffer size and is not compatible with standard strncat.
char* strncatz(char* dest, const char* src, size_t maxlen)
{
  size_t Length = strlen(dest);
  if (Length + 1 < maxlen)
#if (_MSC_VER >= 1300) || __STDC_WANT_SECURE_LIB__
    strcat_s(dest, maxlen - Length - 1, src);
#else
    strncat(dest, src, maxlen - Length - 1);
#endif
  return dest;
}


// Safe wcsncat: resulting dest length cannot exceed maxlen and dest 
// is always zero terminated. Note that 'maxlen' parameter defines the entire
// dest buffer size and is not compatible with standard wcsncat.
wchar* wcsncatz(wchar* dest, const wchar* src, size_t maxlen)
{
  size_t Length = my_wcslen(dest);
  if (Length + 1 < maxlen)
    my_wcsncat(dest, src, maxlen - Length - 1);
  return dest;
}


void itoa(int64 n,char *Str)
{
  char NumStr[50];
  size_t Pos=0;

  do
  {
    NumStr[Pos++]=char(n%10)+'0';
    n=n/10;
  } while (n!=0);

  for (size_t I=0;I<Pos;I++)
    Str[I]=NumStr[Pos-I-1];
  Str[Pos]=0;
}


void itoa(int64 n,wchar *Str)
{
  wchar NumStr[50];
  size_t Pos=0;

  do
  {
    NumStr[Pos++]=wchar(n%10)+'0';
    n=n/10;
  } while (n!=0);

  for (size_t I=0;I<Pos;I++)
    Str[I]=NumStr[Pos-I-1];
  Str[Pos]=0;
}


const wchar* GetWide(const char *Src)
{
  const size_t MaxLength=NM;
  static wchar StrTable[4][MaxLength];
  static uint StrNum=0;
  if (++StrNum >= ASIZE(StrTable))
    StrNum=0;
  wchar *Str=StrTable[StrNum];
  CharToWide(Src,Str,MaxLength);
  Str[MaxLength-1]=0;
  return Str;
}


// Parse string containing parameters separated with spaces.
// Support quote marks. Param can be NULL to return the pointer to next
// parameter, which can be used to estimate the buffer size for Param.
const wchar* GetCmdParam(const wchar *CmdLine,wchar *Param,size_t MaxSize)
{
  while (IsSpace(*CmdLine))
    CmdLine++;
  if (*CmdLine==0)
    return NULL;

  size_t ParamSize=0;
  bool Quote=false;
  while (*CmdLine!=0 && (Quote || !IsSpace(*CmdLine)))
  {
    if (*CmdLine=='\"')
    {
      if (CmdLine[1]=='\"')
      {
        // Insert the quote character instead of two adjoining quote characters.
        if (Param!=NULL && ParamSize<MaxSize-1)
          Param[ParamSize++]='\"';
        CmdLine++;
      }
      else
        Quote=!Quote;
    }
    else
      if (Param!=NULL && ParamSize<MaxSize-1)
        Param[ParamSize++]=*CmdLine;
    CmdLine++;
  }
  if (Param!=NULL)
    Param[ParamSize]=0;
  return CmdLine;
}


#ifndef SILENT
// For compatibility with existing translations we use %s to print Unicode
// strings in format strings and convert them to %ls here. %s could work
// without such conversion in Windows, but not in Unix wprintf.
void PrintfPrepareFmt(const wchar *Org,wchar *Cvt,size_t MaxSize)
{
  uint Src=0,Dest=0;
  while (Org[Src]!=0 && Dest<MaxSize-1)
  {
    if (Org[Src]=='%' && (Src==0 || Org[Src-1]!='%'))
    {
      uint SPos=Src+1;
      // Skipping a possible width specifier like %-50s.
      while (IsDigit(Org[SPos]) || Org[SPos]=='-')
        SPos++;
      if (Org[SPos]=='s' && Dest<MaxSize-(SPos-Src+1))
      {
        while (Src<SPos)
          Cvt[Dest++]=Org[Src++];
        Cvt[Dest++]='l';
      }
    }
#ifdef _WIN_ALL
    // Convert \n to \r\n in Windows. Important when writing to log,
    // so other tools like Notebook can view resulting log properly.
    if (Org[Src]=='\n' && (Src==0 || Org[Src-1]!='\r'))
      Cvt[Dest++]='\r';
#endif

    Cvt[Dest++]=Org[Src++];
  }
  Cvt[Dest]=0;
}
#endif
