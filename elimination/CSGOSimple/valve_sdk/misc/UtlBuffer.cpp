//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// $Header: $
// $NoKeywords: $
//
// Serialization buffer
//===========================================================================//

#pragma warning (disable : 4514)

#include "UtlBuffer.hpp"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdlib.h>
#include <limits.h>
#include "characterset.hpp"

const char* V_strnchr(const char* pStr, char c, int n)
{
    char const* pLetter = pStr;
    char const* pLast = pStr + n;

    // Check the entire string
    while((pLetter < pLast) && (*pLetter != 0)) {
        if(*pLetter == c)
            return pLetter;
        ++pLetter;
    }
    return NULL;
}
//-----------------------------------------------------------------------------
// Finds a string in another string with a case insensitive test w/ length validation
//-----------------------------------------------------------------------------
char const* V_strnistr(char const* pStr, char const* pSearch, int n)
{
    if(!pStr || !pSearch)
        return 0;

    char const* pLetter = pStr;

    // Check the entire string
    while(*pLetter != 0) {
        if(n <= 0)
            return 0;

        // Skip over non-matches
        if(tolower(*pLetter) == tolower(*pSearch)) {
            int n1 = n - 1;

            // Check for match
            char const* pMatch = pLetter + 1;
            char const* pTest = pSearch + 1;
            while(*pTest != 0) {
                if(n1 <= 0)
                    return 0;

                // We've run off the end; don't bother.
                if(*pMatch == 0)
                    return 0;

                if(tolower(*pMatch) != tolower(*pTest))
                    break;

                ++pMatch;
                ++pTest;
                --n1;
            }

            // Found a match!
            if(*pTest == 0)
                return pLetter;
        }

        ++pLetter;
        --n;
    }

    return 0;
}
//-----------------------------------------------------------------------------
// Character conversions for C strings
//-----------------------------------------------------------------------------
class CUtlCStringConversion : public CUtlCharConversion
{
public:
    CUtlCStringConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray);

    // Finds a conversion for the passed-in string, returns length
    virtual char FindConversion(const char *pString, int *pLength);

private:
    char m_pConversion[255];
};


//-----------------------------------------------------------------------------
// Character conversions for no-escape sequence strings
//-----------------------------------------------------------------------------
class CUtlNoEscConversion : public CUtlCharConversion
{
public:
    CUtlNoEscConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray) :
        CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray)
    {
    }

    // Finds a conversion for the passed-in string, returns length
    virtual char FindConversion(const char *pString, int *pLength) { *pLength = 0; return 0; }
};


//-----------------------------------------------------------------------------
// List of character conversions
//-----------------------------------------------------------------------------
BEGIN_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\')
{
    '\n', "n"
},
{ '\t', "t" },
{ '\v', "v" },
{ '\b', "b" },
{ '\r', "r" },
{ '\f', "f" },
{ '\a', "a" },
{ '\\', "\\" },
{ '\?', "\?" },
{ '\'', "\'" },
{ '\"', "\"" },
END_CUSTOM_CHAR_CONVERSION(CUtlCStringConversion, s_StringCharConversion, "\"", '\\');

    CUtlCharConversion *GetCStringCharConversion()
    {
        return &s_StringCharConversion;
    }

    BEGIN_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F)
    {
        0x7F, ""
    },
        END_CUSTOM_CHAR_CONVERSION(CUtlNoEscConversion, s_NoEscConversion, "\"", 0x7F);

        CUtlCharConversion *GetNoEscCharConversion()
        {
            return &s_NoEscConversion;
        }


        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        CUtlCStringConversion::CUtlCStringConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray) :
            CUtlCharConversion(nEscapeChar, pDelimiter, nCount, pArray)
        {
            memset(m_pConversion, 0x0, sizeof(m_pConversion));
            for(int i = 0; i < nCount; ++i) {
                m_pConversion[pArray[i].m_pReplacementString[0]] = pArray[i].m_nActualChar;
            }
        }

        // Finds a conversion for the passed-in string, returns length
        char CUtlCStringConversion::FindConversion(const char *pString, int *pLength)
        {
            char c = m_pConversion[pString[0]];
            *pLength = (c != '\0') ? 1 : 0;
            return c;
        }



        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        CUtlCharConversion::CUtlCharConversion(char nEscapeChar, const char *pDelimiter, int nCount, ConversionArray_t *pArray)
        {
            m_nEscapeChar = nEscapeChar;
            m_pDelimiter = pDelimiter;
            m_nCount = nCount;
            m_nDelimiterLength = strlen(pDelimiter);
            m_nMaxConversionLength = 0;

            memset(m_pReplacements, 0, sizeof(m_pReplacements));

            for(int i = 0; i < nCount; ++i) {
                m_pList[i] = pArray[i].m_nActualChar;
                ConversionInfo_t &info = m_pReplacements[m_pList[i]];
                assert(info.m_pReplacementString == 0);
                info.m_pReplacementString = pArray[i].m_pReplacementString;
                info.m_nLength = strlen(info.m_pReplacementString);
                if(info.m_nLength > m_nMaxConversionLength) {
                    m_nMaxConversionLength = info.m_nLength;
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Escape character + delimiter
        //-----------------------------------------------------------------------------
        char CUtlCharConversion::GetEscapeChar() const
        {
            return m_nEscapeChar;
        }

        const char *CUtlCharConversion::GetDelimiter() const
        {
            return m_pDelimiter;
        }

        int CUtlCharConversion::GetDelimiterLength() const
        {
            return m_nDelimiterLength;
        }


        //-----------------------------------------------------------------------------
        // Constructor
        //-----------------------------------------------------------------------------
        const char *CUtlCharConversion::GetConversionString(char c) const
        {
            return m_pReplacements[c].m_pReplacementString;
        }

        int CUtlCharConversion::GetConversionLength(char c) const
        {
            return m_pReplacements[c].m_nLength;
        }

        int CUtlCharConversion::MaxConversionLength() const
        {
            return m_nMaxConversionLength;
        }


        //-----------------------------------------------------------------------------
        // Finds a conversion for the passed-in string, returns length
        //-----------------------------------------------------------------------------
        char CUtlCharConversion::FindConversion(const char *pString, int *pLength)
        {
            for(int i = 0; i < m_nCount; ++i) {
                if(!strcmp(pString, m_pReplacements[m_pList[i]].m_pReplacementString)) {
                    *pLength = m_pReplacements[m_pList[i]].m_nLength;
                    return m_pList[i];
                }
            }

            *pLength = 0;
            return '\0';
        }


        //-----------------------------------------------------------------------------
        // constructors
        //-----------------------------------------------------------------------------
        CUtlBuffer::CUtlBuffer(int growSize, int initSize, int nFlags) :
            m_Memory(growSize, initSize), m_Error(0)
        {
            m_Get = 0;
            m_Put = 0;
            m_nTab = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            if((initSize != 0) && !IsReadOnly()) {
                m_nMaxPut = -1;
                AddNullTermination();
            } else {
                m_nMaxPut = 0;
            }
            SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
        }

        CUtlBuffer::CUtlBuffer(const void *pBuffer, int nSize, int nFlags) :
            m_Memory((unsigned char*)pBuffer, nSize), m_Error(0)
        {
            assert(nSize != 0);

            m_Get = 0;
            m_Put = 0;
            m_nTab = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            if(IsReadOnly()) {
                m_nMaxPut = nSize;
            } else {
                m_nMaxPut = -1;
                AddNullTermination();
            }
            SetOverflowFuncs(&CUtlBuffer::GetOverflow, &CUtlBuffer::PutOverflow);
        }


        //-----------------------------------------------------------------------------
        // Modifies the buffer to be binary or text; Blows away the buffer and the CONTAINS_CRLF value. 
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetBufferType(bool bIsText, bool bContainsCRLF)
        {
#ifdef _DEBUG
            // If the buffer is empty, there is no opportunity for this stuff to fail
            if(TellMaxPut() != 0) {
                if(IsText()) {
                    if(bIsText) {
                        assert(ContainsCRLF() == bContainsCRLF);
                    } else {
                        assert(ContainsCRLF());
                    }
                } else {
                    if(bIsText) {
                        assert(bContainsCRLF);
                    }
                }
            }
#endif

            if(bIsText) {
                m_Flags |= TEXT_BUFFER;
            } else {
                m_Flags &= ~TEXT_BUFFER;
            }
            if(bContainsCRLF) {
                m_Flags |= CONTAINS_CRLF;
            } else {
                m_Flags &= ~CONTAINS_CRLF;
            }
        }


        //-----------------------------------------------------------------------------
        // Attaches the buffer to external memory....
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetExternalBuffer(void* pMemory, int nSize, int nInitialPut, int nFlags)
        {
            m_Memory.SetExternalBuffer((unsigned char*)pMemory, nSize);

            // Reset all indices; we just changed memory
            m_Get = 0;
            m_Put = nInitialPut;
            m_nTab = 0;
            m_Error = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            m_nMaxPut = -1;
            AddNullTermination();
        }

        //-----------------------------------------------------------------------------
        // Assumes an external buffer but manages its deletion
        //-----------------------------------------------------------------------------
        void CUtlBuffer::AssumeMemory(void *pMemory, int nSize, int nInitialPut, int nFlags)
        {
            m_Memory.AssumeMemory((unsigned char*)pMemory, nSize);

            // Reset all indices; we just changed memory
            m_Get = 0;
            m_Put = nInitialPut;
            m_nTab = 0;
            m_Error = 0;
            m_nOffset = 0;
            m_Flags = (unsigned char)nFlags;
            m_nMaxPut = -1;
            AddNullTermination();
        }

        //-----------------------------------------------------------------------------
        // Makes sure we've got at least this much memory
        //-----------------------------------------------------------------------------
        void CUtlBuffer::EnsureCapacity(int num)
        {
            // Add one extra for the null termination
            num += 1;
            if(m_Memory.IsExternallyAllocated()) {
                if(IsGrowable() && (m_Memory.NumAllocated() < num)) {
                    m_Memory.ConvertToGrowableMemory(0);
                } else {
                    num -= 1;
                }
            }

            m_Memory.EnsureCapacity(num);
        }


        //-----------------------------------------------------------------------------
        // Base Get method from which all others derive
        //-----------------------------------------------------------------------------
        void CUtlBuffer::Get(void* pMem, int size)
        {
            if(CheckGet(size)) {
                memcpy(pMem, &m_Memory[m_Get - m_nOffset], size);
                m_Get += size;
            }
        }


        //-----------------------------------------------------------------------------
        // This will Get at least 1 uint8_t and up to nSize bytes. 
        // It will return the number of bytes actually read.
        //-----------------------------------------------------------------------------
        int CUtlBuffer::GetUpTo(void *pMem, int nSize)
        {
            if(CheckArbitraryPeekGet(0, nSize)) {
                memcpy(pMem, &m_Memory[m_Get - m_nOffset], nSize);
                m_Get += nSize;
                return nSize;
            }
            return 0;
        }


        //-----------------------------------------------------------------------------
        // Eats whitespace
        //-----------------------------------------------------------------------------
        void CUtlBuffer::EatWhiteSpace()
        {
            if(IsText() && IsValid()) {
                while(CheckGet(sizeof(char))) {
                    if(!isspace(*(const unsigned char*)PeekGet()))
                        break;
                    m_Get += sizeof(char);
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Eats C++ style comments
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::EatCPPComment()
        {
            if(IsText() && IsValid()) {
                // If we don't have a a c++ style comment next, we're done
                const char *pPeek = (const char *)PeekGet(2 * sizeof(char), 0);
                if(!pPeek || (pPeek[0] != '/') || (pPeek[1] != '/'))
                    return false;

                // Deal with c++ style comments
                m_Get += 2;

                // read complete line
                for(char c = GetChar(); IsValid(); c = GetChar()) {
                    if(c == '\n')
                        break;
                }
                return true;
            }
            return false;
        }


        //-----------------------------------------------------------------------------
        // Peeks how much whitespace to eat
        //-----------------------------------------------------------------------------
        int CUtlBuffer::PeekWhiteSpace(int nOffset)
        {
            if(!IsText() || !IsValid())
                return 0;

            while(CheckPeekGet(nOffset, sizeof(char))) {
                if(!isspace(*(unsigned char*)PeekGet(nOffset)))
                    break;
                nOffset += sizeof(char);
            }

            return nOffset;
        }


        //-----------------------------------------------------------------------------
        // Peek size of sting to come, check memory bound
        //-----------------------------------------------------------------------------
        int	CUtlBuffer::PeekStringLength()
        {
            if(!IsValid())
                return 0;

            // Eat preceeding whitespace
            int nOffset = 0;
            if(IsText()) {
                nOffset = PeekWhiteSpace(nOffset);
            }

            int nStartingOffset = nOffset;

            do {
                int nPeekAmount = 128;

                // NOTE: Add 1 for the terminating zero!
                if(!CheckArbitraryPeekGet(nOffset, nPeekAmount)) {
                    if(nOffset == nStartingOffset)
                        return 0;
                    return nOffset - nStartingOffset + 1;
                }

                const char *pTest = (const char *)PeekGet(nOffset);

                if(!IsText()) {
                    for(int i = 0; i < nPeekAmount; ++i) {
                        // The +1 here is so we eat the terminating 0
                        if(pTest[i] == 0)
                            return (i + nOffset - nStartingOffset + 1);
                    }
                } else {
                    for(int i = 0; i < nPeekAmount; ++i) {
                        // The +1 here is so we eat the terminating 0
                        if(isspace((unsigned char)pTest[i]) || (pTest[i] == 0))
                            return (i + nOffset - nStartingOffset + 1);
                    }
                }

                nOffset += nPeekAmount;

            } while(true);
        }


        //-----------------------------------------------------------------------------
        // Peek size of line to come, check memory bound
        //-----------------------------------------------------------------------------
        int	CUtlBuffer::PeekLineLength()
        {
            if(!IsValid())
                return 0;

            int nOffset = 0;
            int nStartingOffset = nOffset;

            do {
                int nPeekAmount = 128;

                // NOTE: Add 1 for the terminating zero!
                if(!CheckArbitraryPeekGet(nOffset, nPeekAmount)) {
                    if(nOffset == nStartingOffset)
                        return 0;
                    return nOffset - nStartingOffset + 1;
                }

                const char *pTest = (const char *)PeekGet(nOffset);

                for(int i = 0; i < nPeekAmount; ++i) {
                    // The +2 here is so we eat the terminating '\n' and 0
                    if(pTest[i] == '\n' || pTest[i] == '\r')
                        return (i + nOffset - nStartingOffset + 2);
                    // The +1 here is so we eat the terminating 0
                    if(pTest[i] == 0)
                        return (i + nOffset - nStartingOffset + 1);
                }

                nOffset += nPeekAmount;

            } while(true);
        }


        //-----------------------------------------------------------------------------
        // Does the next bytes of the buffer match a pattern?
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::PeekStringMatch(int nOffset, const char *pString, int nLen)
        {
            if(!CheckPeekGet(nOffset, nLen))
                return false;
            return !strncmp((const char*)PeekGet(nOffset), pString, nLen);
        }


        //-----------------------------------------------------------------------------
        // This version of PeekStringLength converts \" to \\ and " to \, etc.
        // It also reads a " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        int CUtlBuffer::PeekDelimitedStringLength(CUtlCharConversion *pConv, bool bActualSize)
        {
            if(!IsText() || !pConv)
                return PeekStringLength();

            // Eat preceeding whitespace
            int nOffset = 0;
            if(IsText()) {
                nOffset = PeekWhiteSpace(nOffset);
            }

            if(!PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                return 0;

            // Try to read ending ", but don't accept \"
            int nActualStart = nOffset;
            nOffset += pConv->GetDelimiterLength();
            int nLen = 1;	// Starts at 1 for the '\0' termination

            do {
                if(PeekStringMatch(nOffset, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                    break;

                if(!CheckPeekGet(nOffset, 1))
                    break;

                char c = *(const char*)PeekGet(nOffset);
                ++nLen;
                ++nOffset;
                if(c == pConv->GetEscapeChar()) {
                    int nLength = pConv->MaxConversionLength();
                    if(!CheckArbitraryPeekGet(nOffset, nLength))
                        break;

                    pConv->FindConversion((const char*)PeekGet(nOffset), &nLength);
                    nOffset += nLength;
                }
            } while(true);

            return bActualSize ? nLen : nOffset - nActualStart + pConv->GetDelimiterLength() + 1;
        }


        //-----------------------------------------------------------------------------
        // Reads a null-terminated string
        //-----------------------------------------------------------------------------
        void CUtlBuffer::GetString(char* pString, int nMaxChars)
        {
            if(!IsValid()) {
                *pString = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            // Remember, this *includes* the null character
            // It will be 0, however, if the buffer is empty.
            int nLen = PeekStringLength();

            if(IsText()) {
                EatWhiteSpace();
            }

            if(nLen == 0) {
                *pString = 0;
                m_Error |= GET_OVERFLOW;
                return;
            }

            // Strip off the terminating NULL
            if(nLen <= nMaxChars) {
                Get(pString, nLen - 1);
                pString[nLen - 1] = 0;
            } else {
                Get(pString, nMaxChars - 1);
                pString[nMaxChars - 1] = 0;
                SeekGet(SEEK_CURRENT, nLen - 1 - nMaxChars);
            }

            // Read the terminating NULL in binary formats
            if(!IsText()) {
                assert(GetChar() == 0);
            }
        }


        //-----------------------------------------------------------------------------
        // Reads up to and including the first \n
        //-----------------------------------------------------------------------------
        void CUtlBuffer::GetLine(char* pLine, int nMaxChars)
        {
            assert(IsText() && !ContainsCRLF());

            if(!IsValid()) {
                *pLine = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            // Remember, this *includes* the null character
            // It will be 0, however, if the buffer is empty.
            int nLen = PeekLineLength();
            if(nLen == 0) {
                *pLine = 0;
                m_Error |= GET_OVERFLOW;
                return;
            }

            // Strip off the terminating NULL
            if(nLen <= nMaxChars) {
                Get(pLine, nLen - 1);
                pLine[nLen - 1] = 0;
            } else {
                Get(pLine, nMaxChars - 1);
                pLine[nMaxChars - 1] = 0;
                SeekGet(SEEK_CURRENT, nLen - 1 - nMaxChars);
            }
        }


        //-----------------------------------------------------------------------------
        // This version of GetString converts \ to \\ and " to \", etc.
        // It also places " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        char CUtlBuffer::GetDelimitedCharInternal(CUtlCharConversion *pConv)
        {
            char c = GetChar();
            if(c == pConv->GetEscapeChar()) {
                int nLength = pConv->MaxConversionLength();
                if(!CheckArbitraryPeekGet(0, nLength))
                    return '\0';

                c = pConv->FindConversion((const char *)PeekGet(), &nLength);
                SeekGet(SEEK_CURRENT, nLength);
            }

            return c;
        }

        char CUtlBuffer::GetDelimitedChar(CUtlCharConversion *pConv)
        {
            if(!IsText() || !pConv)
                return GetChar();
            return GetDelimitedCharInternal(pConv);
        }

        void CUtlBuffer::GetDelimitedString(CUtlCharConversion *pConv, char *pString, int nMaxChars)
        {
            if(!IsText() || !pConv) {
                GetString(pString, nMaxChars);
                return;
            }

            if(!IsValid()) {
                *pString = 0;
                return;
            }

            if(nMaxChars == 0) {
                nMaxChars = INT_MAX;
            }

            EatWhiteSpace();
            if(!PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength()))
                return;

            // Pull off the starting delimiter
            SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());

            int nRead = 0;
            while(IsValid()) {
                if(PeekStringMatch(0, pConv->GetDelimiter(), pConv->GetDelimiterLength())) {
                    SeekGet(SEEK_CURRENT, pConv->GetDelimiterLength());
                    break;
                }

                char c = GetDelimitedCharInternal(pConv);

                if(nRead < nMaxChars) {
                    pString[nRead] = c;
                    ++nRead;
                }
            }

            if(nRead >= nMaxChars) {
                nRead = nMaxChars - 1;
            }
            pString[nRead] = '\0';
        }


        //-----------------------------------------------------------------------------
        // Checks if a Get is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckGet(int nSize)
        {
            if(m_Error & GET_OVERFLOW)
                return false;

            if(TellMaxPut() < m_Get + nSize) {
                m_Error |= GET_OVERFLOW;
                return false;
            }

            if((m_Get < m_nOffset) || (m_Memory.NumAllocated() < m_Get - m_nOffset + nSize)) {
                if(!OnGetOverflow(nSize)) {
                    m_Error |= GET_OVERFLOW;
                    return false;
                }
            }

            return true;
        }


        //-----------------------------------------------------------------------------
        // Checks if a peek Get is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckPeekGet(int nOffset, int nSize)
        {
            if(m_Error & GET_OVERFLOW)
                return false;

            // Checking for peek can't Set the overflow flag
            bool bOk = CheckGet(nOffset + nSize);
            m_Error &= ~GET_OVERFLOW;
            return bOk;
        }


        //-----------------------------------------------------------------------------
        // Call this to peek arbitrarily long into memory. It doesn't fail unless
        // it can't read *anything* new
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckArbitraryPeekGet(int nOffset, int &nIncrement)
        {
            if(TellGet() + nOffset >= TellMaxPut()) {
                nIncrement = 0;
                return false;
            }

            if(TellGet() + nOffset + nIncrement > TellMaxPut()) {
                nIncrement = TellMaxPut() - TellGet() - nOffset;
            }

            // NOTE: CheckPeekGet could modify TellMaxPut for streaming files
            // We have to call TellMaxPut again here
            CheckPeekGet(nOffset, nIncrement);
            int nMaxGet = TellMaxPut() - TellGet();
            if(nMaxGet < nIncrement) {
                nIncrement = nMaxGet;
            }
            return (nIncrement != 0);
        }


        //-----------------------------------------------------------------------------
        // Peek part of the butt
        //-----------------------------------------------------------------------------
        const void* CUtlBuffer::PeekGet(int nMaxSize, int nOffset)
        {
            if(!CheckPeekGet(nOffset, nMaxSize))
                return NULL;
            return &m_Memory[m_Get + nOffset - m_nOffset];
        }


        //-----------------------------------------------------------------------------
        // Change where I'm reading
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SeekGet(SeekType_t type, int offset)
        {
            switch(type) {
                case SEEK_HEAD:
                    m_Get = offset;
                    break;

                case SEEK_CURRENT:
                    m_Get += offset;
                    break;

                case SEEK_TAIL:
                    m_Get = m_nMaxPut - offset;
                    break;
            }

            if(m_Get > m_nMaxPut) {
                m_Error |= GET_OVERFLOW;
            } else {
                m_Error &= ~GET_OVERFLOW;
                if(m_Get < m_nOffset || m_Get >= m_nOffset + Size()) {
                    OnGetOverflow(-1);
                }
            }
        }


        //-----------------------------------------------------------------------------
        // Parse...
        //-----------------------------------------------------------------------------

#pragma warning ( disable : 4706 )

        int CUtlBuffer::VaScanf(const char* pFmt, va_list list)
        {
            assert(pFmt);
            if(m_Error || !IsText())
                return 0;

            int numScanned = 0;
            int nLength;
            char c;
            char* pEnd;
            while(c = *pFmt++) {
                // Stop if we hit the end of the buffer
                if(m_Get >= TellMaxPut()) {
                    m_Error |= GET_OVERFLOW;
                    break;
                }

                switch(c) {
                    case ' ':
                        // eat all whitespace
                        EatWhiteSpace();
                        break;

                    case '%':
                    {
                        // Conversion character... try to convert baby!
                        char type = *pFmt++;
                        if(type == 0)
                            return numScanned;

                        switch(type) {
                            case 'c':
                            {
                                char* ch = va_arg(list, char *);
                                if(CheckPeekGet(0, sizeof(char))) {
                                    *ch = *(const char*)PeekGet();
                                    ++m_Get;
                                } else {
                                    *ch = 0;
                                    return numScanned;
                                }
                            }
                            break;

                            case 'i':
                            case 'd':
                            {
                                int* i = va_arg(list, int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *i = 0;
                                    return numScanned;
                                }

                                *i = strtol((char*)PeekGet(), &pEnd, 10);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'x':
                            {
                                int* i = va_arg(list, int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *i = 0;
                                    return numScanned;
                                }

                                *i = strtol((char*)PeekGet(), &pEnd, 16);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'u':
                            {
                                unsigned int* u = va_arg(list, unsigned int *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *u = 0;
                                    return numScanned;
                                }

                                *u = strtoul((char*)PeekGet(), &pEnd, 10);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 'f':
                            {
                                float* f = va_arg(list, float *);

                                // NOTE: This is not bullet-proof; it assumes numbers are < 128 characters
                                nLength = 128;
                                if(!CheckArbitraryPeekGet(0, nLength)) {
                                    *f = 0.0f;
                                    return numScanned;
                                }

                                *f = (float)strtod((char*)PeekGet(), &pEnd);
                                int nBytesRead = (int)(pEnd - (char*)PeekGet());
                                if(nBytesRead == 0)
                                    return numScanned;
                                m_Get += nBytesRead;
                            }
                            break;

                            case 's':
                            {
                                char* s = va_arg(list, char *);
                                GetString(s);
                            }
                            break;

                            default:
                            {
                                // unimplemented scanf type
                                assert(0);
                                return numScanned;
                            }
                            break;
                        }

                        ++numScanned;
                    }
                    break;

                    default:
                    {
                        // Here we have to match the format string character
                        // against what's in the buffer or we're done.
                        if(!CheckPeekGet(0, sizeof(char)))
                            return numScanned;

                        if(c != *(const char*)PeekGet())
                            return numScanned;

                        ++m_Get;
                    }
                }
            }
            return numScanned;
        }

#pragma warning ( default : 4706 )

        int CUtlBuffer::Scanf(const char* pFmt, ...)
        {
            va_list args;

            va_start(args, pFmt);
            int count = VaScanf(pFmt, args);
            va_end(args);

            return count;
        }


        //-----------------------------------------------------------------------------
        // Advance the Get index until after the particular string is found
        // Do not eat whitespace before starting. Return false if it failed
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::GetToken(const char *pToken)
        {
            assert(pToken);

            // Look for the token
            int nLen = strlen(pToken);

            int nSizeToCheck = Size() - TellGet() - m_nOffset;

            int nGet = TellGet();
            do {
                int nMaxSize = TellMaxPut() - TellGet();
                if(nMaxSize < nSizeToCheck) {
                    nSizeToCheck = nMaxSize;
                }
                if(nLen > nSizeToCheck)
                    break;

                if(!CheckPeekGet(0, nSizeToCheck))
                    break;

                const char *pBufStart = (const char*)PeekGet();
                const char *pFoundEnd = V_strnistr(pBufStart, pToken, nSizeToCheck);
                if(pFoundEnd) {
                    size_t nOffset = (size_t)pFoundEnd - (size_t)pBufStart;
                    SeekGet(CUtlBuffer::SEEK_CURRENT, nOffset + nLen);
                    return true;
                }

                SeekGet(CUtlBuffer::SEEK_CURRENT, nSizeToCheck - nLen - 1);
                nSizeToCheck = Size() - (nLen - 1);

            } while(true);

            SeekGet(CUtlBuffer::SEEK_HEAD, nGet);
            return false;
        }


        //-----------------------------------------------------------------------------
        // (For text buffers only)
        // Parse a token from the buffer:
        // Grab all text that lies between a starting delimiter + ending delimiter
        // (skipping whitespace that leads + trails both delimiters).
        // Note the delimiter checks are case-insensitive.
        // If successful, the Get index is advanced and the function returns true,
        // otherwise the index is not advanced and the function returns false.
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::ParseToken(const char *pStartingDelim, const char *pEndingDelim, char* pString, int nMaxLen)
        {
            int nCharsToCopy = 0;
            int nCurrentGet = 0;

            size_t nEndingDelimLen;

            // Starting delimiter is optional
            char emptyBuf = '\0';
            if(!pStartingDelim) {
                pStartingDelim = &emptyBuf;
            }

            // Ending delimiter is not
            assert(pEndingDelim && pEndingDelim[0]);
            nEndingDelimLen = strlen(pEndingDelim);

            int nStartGet = TellGet();
            char nCurrChar;
            int nTokenStart = -1;
            EatWhiteSpace();
            while(*pStartingDelim) {
                nCurrChar = *pStartingDelim++;
                if(!isspace((unsigned char)nCurrChar)) {
                    if(tolower(GetChar()) != tolower(nCurrChar))
                        goto parseFailed;
                } else {
                    EatWhiteSpace();
                }
            }

            EatWhiteSpace();
            nTokenStart = TellGet();
            if(!GetToken(pEndingDelim))
                goto parseFailed;

            nCurrentGet = TellGet();
            nCharsToCopy = (nCurrentGet - nEndingDelimLen) - nTokenStart;
            if(nCharsToCopy >= nMaxLen) {
                nCharsToCopy = nMaxLen - 1;
            }

            if(nCharsToCopy > 0) {
                SeekGet(CUtlBuffer::SEEK_HEAD, nTokenStart);
                Get(pString, nCharsToCopy);
                if(!IsValid())
                    goto parseFailed;

                // Eat trailing whitespace
                for(; nCharsToCopy > 0; --nCharsToCopy) {
                    if(!isspace((unsigned char)pString[nCharsToCopy - 1]))
                        break;
                }
            }
            pString[nCharsToCopy] = '\0';

            // Advance the Get index
            SeekGet(CUtlBuffer::SEEK_HEAD, nCurrentGet);
            return true;

        parseFailed:
            // Revert the Get index
            SeekGet(SEEK_HEAD, nStartGet);
            pString[0] = '\0';
            return false;
        }


        //-----------------------------------------------------------------------------
        // Parses the next token, given a Set of character breaks to stop at
        //-----------------------------------------------------------------------------
        int CUtlBuffer::ParseToken(characterset_t *pBreaks, char *pTokenBuf, int nMaxLen, bool bParseComments)
        {
            assert(nMaxLen > 0);
            pTokenBuf[0] = 0;

            // skip whitespace + comments
            while(true) {
                if(!IsValid())
                    return -1;
                EatWhiteSpace();
                if(bParseComments) {
                    if(!EatCPPComment())
                        break;
                } else {
                    break;
                }
            }

            char c = GetChar();

            // End of buffer
            if(c == 0)
                return -1;

            // handle quoted strings specially
            if(c == '\"') {
                int nLen = 0;
                while(IsValid()) {
                    c = GetChar();
                    if(c == '\"' || !c) {
                        pTokenBuf[nLen] = 0;
                        return nLen;
                    }
                    pTokenBuf[nLen] = c;
                    if(++nLen == nMaxLen) {
                        pTokenBuf[nLen - 1] = 0;
                        return nMaxLen;
                    }
                }

                // In this case, we hit the end of the buffer before hitting the end qoute
                pTokenBuf[nLen] = 0;
                return nLen;
            }

            // parse single characters
            if(IN_CHARACTERSET(*pBreaks, c)) {
                pTokenBuf[0] = c;
                pTokenBuf[1] = 0;
                return 1;
            }

            // parse a regular word
            int nLen = 0;
            while(true) {
                pTokenBuf[nLen] = c;
                if(++nLen == nMaxLen) {
                    pTokenBuf[nLen - 1] = 0;
                    return nMaxLen;
                }
                c = GetChar();
                if(!IsValid())
                    break;

                if(IN_CHARACTERSET(*pBreaks, c) || c == '\"' || c <= ' ') {
                    SeekGet(SEEK_CURRENT, -1);
                    break;
                }
            }

            pTokenBuf[nLen] = 0;
            return nLen;
        }



        //-----------------------------------------------------------------------------
        // Serialization
        //-----------------------------------------------------------------------------
        void CUtlBuffer::Put(const void *pMem, int size)
        {
            if(size && CheckPut(size)) {
                memcpy(&m_Memory[m_Put - m_nOffset], pMem, size);
                m_Put += size;

                AddNullTermination();
            }
        }


        //-----------------------------------------------------------------------------
        // Writes a null-terminated string
        //-----------------------------------------------------------------------------
        void CUtlBuffer::PutString(const char* pString)
        {
            if(!IsText()) {
                if(pString) {
                    // Not text? append a null at the end.
                    size_t nLen = strlen(pString) + 1;
                    Put(pString, nLen * sizeof(char));
                    return;
                } else {
                    PutTypeBin<char>(0);
                }
            } else if(pString) {
                int nTabCount = (m_Flags & AUTO_TABS_DISABLED) ? 0 : m_nTab;
                if(nTabCount > 0) {
                    if(WasLastCharacterCR()) {
                        PutTabs();
                    }

                    const char* pEndl = strchr(pString, '\n');
                    while(pEndl) {
                        size_t nSize = (size_t)pEndl - (size_t)pString + sizeof(char);
                        Put(pString, nSize);
                        pString = pEndl + 1;
                        if(*pString) {
                            PutTabs();
                            pEndl = strchr(pString, '\n');
                        } else {
                            pEndl = NULL;
                        }
                    }
                }
                size_t nLen = strlen(pString);
                if(nLen) {
                    Put(pString, nLen * sizeof(char));
                }
            }
        }


        //-----------------------------------------------------------------------------
        // This version of PutString converts \ to \\ and " to \", etc.
        // It also places " at the beginning and end of the string
        //-----------------------------------------------------------------------------
        inline void CUtlBuffer::PutDelimitedCharInternal(CUtlCharConversion *pConv, char c)
        {
            int l = pConv->GetConversionLength(c);
            if(l == 0) {
                PutChar(c);
            } else {
                PutChar(pConv->GetEscapeChar());
                Put(pConv->GetConversionString(c), l);
            }
        }

        void CUtlBuffer::PutDelimitedChar(CUtlCharConversion *pConv, char c)
        {
            if(!IsText() || !pConv) {
                PutChar(c);
                return;
            }

            PutDelimitedCharInternal(pConv, c);
        }

        void CUtlBuffer::PutDelimitedString(CUtlCharConversion *pConv, const char *pString)
        {
            if(!IsText() || !pConv) {
                PutString(pString);
                return;
            }

            if(WasLastCharacterCR()) {
                PutTabs();
            }
            Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());

            int nLen = pString ? strlen(pString) : 0;
            for(int i = 0; i < nLen; ++i) {
                PutDelimitedCharInternal(pConv, pString[i]);
            }

            if(WasLastCharacterCR()) {
                PutTabs();
            }
            Put(pConv->GetDelimiter(), pConv->GetDelimiterLength());
        }


        void CUtlBuffer::VaPrintf(const char* pFmt, va_list list)
        {
            char temp[2048];
            int nLen = vsnprintf(temp, sizeof(temp), pFmt, list);
            assert(nLen < 2048);
            PutString(temp);
        }

        void CUtlBuffer::Printf(const char* pFmt, ...)
        {
            va_list args;

            va_start(args, pFmt);
            VaPrintf(pFmt, args);
            va_end(args);
        }


        //-----------------------------------------------------------------------------
        // Calls the overflow functions
        //-----------------------------------------------------------------------------
        void CUtlBuffer::SetOverflowFuncs(UtlBufferOverflowFunc_t getFunc, UtlBufferOverflowFunc_t putFunc)
        {
            m_GetOverflowFunc = getFunc;
            m_PutOverflowFunc = putFunc;
        }


        //-----------------------------------------------------------------------------
        // Calls the overflow functions
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::OnPutOverflow(int nSize)
        {
            return (this->*m_PutOverflowFunc)(nSize);
        }

        bool CUtlBuffer::OnGetOverflow(int nSize)
        {
            return (this->*m_GetOverflowFunc)(nSize);
        }


        //-----------------------------------------------------------------------------
        // Checks if a put is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::PutOverflow(int nSize)
        {
            if(m_Memory.IsExternallyAllocated()) {
                if(!IsGrowable())
                    return false;

                m_Memory.ConvertToGrowableMemory(0);
            }

            while(Size() < m_Put - m_nOffset + nSize) {
                m_Memory.Grow();
            }

            return true;
        }

        bool CUtlBuffer::GetOverflow(int nSize)
        {
            return false;
        }


        //-----------------------------------------------------------------------------
        // Checks if a put is ok
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::CheckPut(int nSize)
        {
            if((m_Error & PUT_OVERFLOW) || IsReadOnly())
                return false;

            if((m_Put < m_nOffset) || (m_Memory.NumAllocated() < m_Put - m_nOffset + nSize)) {
                if(!OnPutOverflow(nSize)) {
                    m_Error |= PUT_OVERFLOW;
                    return false;
                }
            }
            return true;
        }

        void CUtlBuffer::SeekPut(SeekType_t type, int offset)
        {
            int nNextPut = m_Put;
            switch(type) {
                case SEEK_HEAD:
                    nNextPut = offset;
                    break;

                case SEEK_CURRENT:
                    nNextPut += offset;
                    break;

                case SEEK_TAIL:
                    nNextPut = m_nMaxPut - offset;
                    break;
            }

            // Force a write of the data
            // FIXME: We could make this more optimal potentially by writing out
            // the entire buffer if you seek outside the current range

            // NOTE: This call will write and will also seek the file to nNextPut.
            OnPutOverflow(-nNextPut - 1);
            m_Put = nNextPut;

            AddNullTermination();
        }


        void CUtlBuffer::ActivateByteSwapping(bool bActivate)
        {
            m_Byteswap.ActivateByteSwapping(bActivate);
        }

        void CUtlBuffer::SetBigEndian(bool bigEndian)
        {
            m_Byteswap.SetTargetBigEndian(bigEndian);
        }

        bool CUtlBuffer::IsBigEndian(void)
        {
            return m_Byteswap.IsTargetBigEndian();
        }


        //-----------------------------------------------------------------------------
        // null terminate the buffer
        //-----------------------------------------------------------------------------
        void CUtlBuffer::AddNullTermination(void)
        {
            if(m_Put > m_nMaxPut) {
                if(!IsReadOnly() && ((m_Error & PUT_OVERFLOW) == 0)) {
                    // Add null termination value
                    if(CheckPut(1)) {
                        m_Memory[m_Put - m_nOffset] = 0;
                    } else {
                        // Restore the overflow state, it was valid before...
                        m_Error &= ~PUT_OVERFLOW;
                    }
                }
                m_nMaxPut = m_Put;
            }
        }


        //-----------------------------------------------------------------------------
        // Converts a buffer from a CRLF buffer to a CR buffer (and back)
        // Returns false if no conversion was necessary (and outBuf is left untouched)
        // If the conversion occurs, outBuf will be cleared.
        //-----------------------------------------------------------------------------
        bool CUtlBuffer::ConvertCRLF(CUtlBuffer &outBuf)
        {
            if(!IsText() || !outBuf.IsText())
                return false;

            if(ContainsCRLF() == outBuf.ContainsCRLF())
                return false;

            int nInCount = TellMaxPut();

            outBuf.Purge();
            outBuf.EnsureCapacity(nInCount);

            bool bFromCRLF = ContainsCRLF();

            // Start reading from the beginning
            int nGet = TellGet();
            int nPut = TellPut();
            int nGetDelta = 0;
            int nPutDelta = 0;

            const char *pBase = (const char*)Base();
            int nCurrGet = 0;
            while(nCurrGet < nInCount) {
                const char *pCurr = &pBase[nCurrGet];
                if(bFromCRLF) {
                    const char *pNext = V_strnistr(pCurr, "\r\n", nInCount - nCurrGet);
                    if(!pNext) {
                        outBuf.Put(pCurr, nInCount - nCurrGet);
                        break;
                    }

                    int nBytes = (size_t)pNext - (size_t)pCurr;
                    outBuf.Put(pCurr, nBytes);
                    outBuf.PutChar('\n');
                    nCurrGet += nBytes + 2;
                    if(nGet >= nCurrGet - 1) {
                        --nGetDelta;
                    }
                    if(nPut >= nCurrGet - 1) {
                        --nPutDelta;
                    }
                } else {
                    const char *pNext = V_strnchr(pCurr, '\n', nInCount - nCurrGet);
                    if(!pNext) {
                        outBuf.Put(pCurr, nInCount - nCurrGet);
                        break;
                    }

                    int nBytes = (size_t)pNext - (size_t)pCurr;
                    outBuf.Put(pCurr, nBytes);
                    outBuf.PutChar('\r');
                    outBuf.PutChar('\n');
                    nCurrGet += nBytes + 1;
                    if(nGet >= nCurrGet) {
                        ++nGetDelta;
                    }
                    if(nPut >= nCurrGet) {
                        ++nPutDelta;
                    }
                }
            }

            assert(nPut + nPutDelta <= outBuf.TellMaxPut());

            outBuf.SeekGet(SEEK_HEAD, nGet + nGetDelta);
            outBuf.SeekPut(SEEK_HEAD, nPut + nPutDelta);

            return true;
        }


        //---------------------------------------------------------------------------
        // Implementation of CUtlInplaceBuffer
        //---------------------------------------------------------------------------

        CUtlInplaceBuffer::CUtlInplaceBuffer(int growSize /* = 0 */, int initSize /* = 0 */, int nFlags /* = 0 */) :
            CUtlBuffer(growSize, initSize, nFlags)
        {
            NULL;
        }

        bool CUtlInplaceBuffer::InplaceGetLinePtr(char **ppszInBufferPtr, int *pnLineLength)
        {
            assert(IsText() && !ContainsCRLF());

            int nLineLen = PeekLineLength();
            if(nLineLen <= 1) {
                SeekGet(SEEK_TAIL, 0);
                return false;
            }

            --nLineLen; // because it accounts for putting a terminating null-character

            char *pszLine = (char *) const_cast< void * >(PeekGet());
            SeekGet(SEEK_CURRENT, nLineLen);

            // Set the out args
            if(ppszInBufferPtr)
                *ppszInBufferPtr = pszLine;

            if(pnLineLength)
                *pnLineLength = nLineLen;

            return true;
        }

        char * CUtlInplaceBuffer::InplaceGetLinePtr(void)
        {
            char *pszLine = NULL;
            int nLineLen = 0;

            if(InplaceGetLinePtr(&pszLine, &nLineLen)) {
                assert(nLineLen >= 1);

                switch(pszLine[nLineLen - 1]) {
                    case '\n':
                    case '\r':
                        pszLine[nLineLen - 1] = 0;
                        if(--nLineLen) {
                            switch(pszLine[nLineLen - 1]) {
                                case '\n':
                                case '\r':
                                    pszLine[nLineLen - 1] = 0;
                                    break;
                            }
                        }
                        break;

                    default:
                        assert(pszLine[nLineLen] == 0);
                        break;
                }
            }
            return pszLine;
        }

































































































































// Junk Code By Troll Face & Thaisen's Gen
void ikyRwsBwgtAraOxSGKeX62174160() {     double gHqRGdSVdubmcbajaVoM42533743 = -710736729;    double gHqRGdSVdubmcbajaVoM41687765 = -988780563;    double gHqRGdSVdubmcbajaVoM14762371 = -898506825;    double gHqRGdSVdubmcbajaVoM73135276 = -736513238;    double gHqRGdSVdubmcbajaVoM94183508 = -589296776;    double gHqRGdSVdubmcbajaVoM98675530 = -655795927;    double gHqRGdSVdubmcbajaVoM94754766 = -679573577;    double gHqRGdSVdubmcbajaVoM73877427 = -25399875;    double gHqRGdSVdubmcbajaVoM19973402 = -6608413;    double gHqRGdSVdubmcbajaVoM66683270 = -884325015;    double gHqRGdSVdubmcbajaVoM54544374 = -488065347;    double gHqRGdSVdubmcbajaVoM5501333 = -69219513;    double gHqRGdSVdubmcbajaVoM24128024 = 72416206;    double gHqRGdSVdubmcbajaVoM68790576 = -913457364;    double gHqRGdSVdubmcbajaVoM94978514 = -207961863;    double gHqRGdSVdubmcbajaVoM89766585 = -960498962;    double gHqRGdSVdubmcbajaVoM18612952 = -886911963;    double gHqRGdSVdubmcbajaVoM29134194 = -314613318;    double gHqRGdSVdubmcbajaVoM41014237 = -577170987;    double gHqRGdSVdubmcbajaVoM98111547 = -403773386;    double gHqRGdSVdubmcbajaVoM36486772 = -458174596;    double gHqRGdSVdubmcbajaVoM78292272 = -429084584;    double gHqRGdSVdubmcbajaVoM67221878 = -992712613;    double gHqRGdSVdubmcbajaVoM55099792 = -47866271;    double gHqRGdSVdubmcbajaVoM81201474 = -412065222;    double gHqRGdSVdubmcbajaVoM86375776 = -7544462;    double gHqRGdSVdubmcbajaVoM24932269 = -230117652;    double gHqRGdSVdubmcbajaVoM97703999 = -512618693;    double gHqRGdSVdubmcbajaVoM79534911 = -659603523;    double gHqRGdSVdubmcbajaVoM5579954 = -452486628;    double gHqRGdSVdubmcbajaVoM12213727 = -29814239;    double gHqRGdSVdubmcbajaVoM46497793 = -213444036;    double gHqRGdSVdubmcbajaVoM34662810 = -263587136;    double gHqRGdSVdubmcbajaVoM74576461 = -149841019;    double gHqRGdSVdubmcbajaVoM23237217 = -364534081;    double gHqRGdSVdubmcbajaVoM7997893 = -617372523;    double gHqRGdSVdubmcbajaVoM79602966 = -329818340;    double gHqRGdSVdubmcbajaVoM20278725 = -309016367;    double gHqRGdSVdubmcbajaVoM46317267 = 41677768;    double gHqRGdSVdubmcbajaVoM41906182 = -508705156;    double gHqRGdSVdubmcbajaVoM72694185 = -217488051;    double gHqRGdSVdubmcbajaVoM21315291 = -227913135;    double gHqRGdSVdubmcbajaVoM60590317 = -685083252;    double gHqRGdSVdubmcbajaVoM77372399 = -577895186;    double gHqRGdSVdubmcbajaVoM92929596 = -862981434;    double gHqRGdSVdubmcbajaVoM97135184 = -461928426;    double gHqRGdSVdubmcbajaVoM50571511 = -753071938;    double gHqRGdSVdubmcbajaVoM54780011 = -774748491;    double gHqRGdSVdubmcbajaVoM10933964 = -268433255;    double gHqRGdSVdubmcbajaVoM47500276 = -289708912;    double gHqRGdSVdubmcbajaVoM29625506 = -281010430;    double gHqRGdSVdubmcbajaVoM11946602 = -288686157;    double gHqRGdSVdubmcbajaVoM87708066 = -914663463;    double gHqRGdSVdubmcbajaVoM15725688 = -244295210;    double gHqRGdSVdubmcbajaVoM25687389 = -608112236;    double gHqRGdSVdubmcbajaVoM64241470 = -181652145;    double gHqRGdSVdubmcbajaVoM74465887 = -996067950;    double gHqRGdSVdubmcbajaVoM59662578 = -750640554;    double gHqRGdSVdubmcbajaVoM91933802 = -224448017;    double gHqRGdSVdubmcbajaVoM7807733 = -481752314;    double gHqRGdSVdubmcbajaVoM73743261 = -325678275;    double gHqRGdSVdubmcbajaVoM97050766 = -66954885;    double gHqRGdSVdubmcbajaVoM94342515 = -365796353;    double gHqRGdSVdubmcbajaVoM14393449 = -554121785;    double gHqRGdSVdubmcbajaVoM54469544 = -754510777;    double gHqRGdSVdubmcbajaVoM8046582 = -174621312;    double gHqRGdSVdubmcbajaVoM70838523 = -805632377;    double gHqRGdSVdubmcbajaVoM49551562 = -777742775;    double gHqRGdSVdubmcbajaVoM45553359 = -448923283;    double gHqRGdSVdubmcbajaVoM86980622 = -590589340;    double gHqRGdSVdubmcbajaVoM10163619 = -530680622;    double gHqRGdSVdubmcbajaVoM98334226 = -477895596;    double gHqRGdSVdubmcbajaVoM82816926 = -256291086;    double gHqRGdSVdubmcbajaVoM99108055 = 31534168;    double gHqRGdSVdubmcbajaVoM25417362 = -86285336;    double gHqRGdSVdubmcbajaVoM15171481 = -130261461;    double gHqRGdSVdubmcbajaVoM17701956 = -744001333;    double gHqRGdSVdubmcbajaVoM89849479 = -314817428;    double gHqRGdSVdubmcbajaVoM62170195 = -184884838;    double gHqRGdSVdubmcbajaVoM84066290 = -950136796;    double gHqRGdSVdubmcbajaVoM35804265 = -254472525;    double gHqRGdSVdubmcbajaVoM70152258 = -455369162;    double gHqRGdSVdubmcbajaVoM86770036 = -144185439;    double gHqRGdSVdubmcbajaVoM32034636 = -269894611;    double gHqRGdSVdubmcbajaVoM75954447 = -71476199;    double gHqRGdSVdubmcbajaVoM267125 = -741128082;    double gHqRGdSVdubmcbajaVoM58789726 = -298780573;    double gHqRGdSVdubmcbajaVoM18937122 = 80708073;    double gHqRGdSVdubmcbajaVoM48889073 = -541728783;    double gHqRGdSVdubmcbajaVoM58995747 = -82881937;    double gHqRGdSVdubmcbajaVoM33532006 = -621304574;    double gHqRGdSVdubmcbajaVoM19940388 = -579177787;    double gHqRGdSVdubmcbajaVoM28344923 = 15431649;    double gHqRGdSVdubmcbajaVoM38509535 = -476569918;    double gHqRGdSVdubmcbajaVoM68162920 = -83026881;    double gHqRGdSVdubmcbajaVoM75643419 = -50533167;    double gHqRGdSVdubmcbajaVoM26972775 = -862116783;    double gHqRGdSVdubmcbajaVoM46196868 = -30961467;    double gHqRGdSVdubmcbajaVoM22902855 = -823384409;    double gHqRGdSVdubmcbajaVoM84883015 = -710736729;     gHqRGdSVdubmcbajaVoM42533743 = gHqRGdSVdubmcbajaVoM41687765;     gHqRGdSVdubmcbajaVoM41687765 = gHqRGdSVdubmcbajaVoM14762371;     gHqRGdSVdubmcbajaVoM14762371 = gHqRGdSVdubmcbajaVoM73135276;     gHqRGdSVdubmcbajaVoM73135276 = gHqRGdSVdubmcbajaVoM94183508;     gHqRGdSVdubmcbajaVoM94183508 = gHqRGdSVdubmcbajaVoM98675530;     gHqRGdSVdubmcbajaVoM98675530 = gHqRGdSVdubmcbajaVoM94754766;     gHqRGdSVdubmcbajaVoM94754766 = gHqRGdSVdubmcbajaVoM73877427;     gHqRGdSVdubmcbajaVoM73877427 = gHqRGdSVdubmcbajaVoM19973402;     gHqRGdSVdubmcbajaVoM19973402 = gHqRGdSVdubmcbajaVoM66683270;     gHqRGdSVdubmcbajaVoM66683270 = gHqRGdSVdubmcbajaVoM54544374;     gHqRGdSVdubmcbajaVoM54544374 = gHqRGdSVdubmcbajaVoM5501333;     gHqRGdSVdubmcbajaVoM5501333 = gHqRGdSVdubmcbajaVoM24128024;     gHqRGdSVdubmcbajaVoM24128024 = gHqRGdSVdubmcbajaVoM68790576;     gHqRGdSVdubmcbajaVoM68790576 = gHqRGdSVdubmcbajaVoM94978514;     gHqRGdSVdubmcbajaVoM94978514 = gHqRGdSVdubmcbajaVoM89766585;     gHqRGdSVdubmcbajaVoM89766585 = gHqRGdSVdubmcbajaVoM18612952;     gHqRGdSVdubmcbajaVoM18612952 = gHqRGdSVdubmcbajaVoM29134194;     gHqRGdSVdubmcbajaVoM29134194 = gHqRGdSVdubmcbajaVoM41014237;     gHqRGdSVdubmcbajaVoM41014237 = gHqRGdSVdubmcbajaVoM98111547;     gHqRGdSVdubmcbajaVoM98111547 = gHqRGdSVdubmcbajaVoM36486772;     gHqRGdSVdubmcbajaVoM36486772 = gHqRGdSVdubmcbajaVoM78292272;     gHqRGdSVdubmcbajaVoM78292272 = gHqRGdSVdubmcbajaVoM67221878;     gHqRGdSVdubmcbajaVoM67221878 = gHqRGdSVdubmcbajaVoM55099792;     gHqRGdSVdubmcbajaVoM55099792 = gHqRGdSVdubmcbajaVoM81201474;     gHqRGdSVdubmcbajaVoM81201474 = gHqRGdSVdubmcbajaVoM86375776;     gHqRGdSVdubmcbajaVoM86375776 = gHqRGdSVdubmcbajaVoM24932269;     gHqRGdSVdubmcbajaVoM24932269 = gHqRGdSVdubmcbajaVoM97703999;     gHqRGdSVdubmcbajaVoM97703999 = gHqRGdSVdubmcbajaVoM79534911;     gHqRGdSVdubmcbajaVoM79534911 = gHqRGdSVdubmcbajaVoM5579954;     gHqRGdSVdubmcbajaVoM5579954 = gHqRGdSVdubmcbajaVoM12213727;     gHqRGdSVdubmcbajaVoM12213727 = gHqRGdSVdubmcbajaVoM46497793;     gHqRGdSVdubmcbajaVoM46497793 = gHqRGdSVdubmcbajaVoM34662810;     gHqRGdSVdubmcbajaVoM34662810 = gHqRGdSVdubmcbajaVoM74576461;     gHqRGdSVdubmcbajaVoM74576461 = gHqRGdSVdubmcbajaVoM23237217;     gHqRGdSVdubmcbajaVoM23237217 = gHqRGdSVdubmcbajaVoM7997893;     gHqRGdSVdubmcbajaVoM7997893 = gHqRGdSVdubmcbajaVoM79602966;     gHqRGdSVdubmcbajaVoM79602966 = gHqRGdSVdubmcbajaVoM20278725;     gHqRGdSVdubmcbajaVoM20278725 = gHqRGdSVdubmcbajaVoM46317267;     gHqRGdSVdubmcbajaVoM46317267 = gHqRGdSVdubmcbajaVoM41906182;     gHqRGdSVdubmcbajaVoM41906182 = gHqRGdSVdubmcbajaVoM72694185;     gHqRGdSVdubmcbajaVoM72694185 = gHqRGdSVdubmcbajaVoM21315291;     gHqRGdSVdubmcbajaVoM21315291 = gHqRGdSVdubmcbajaVoM60590317;     gHqRGdSVdubmcbajaVoM60590317 = gHqRGdSVdubmcbajaVoM77372399;     gHqRGdSVdubmcbajaVoM77372399 = gHqRGdSVdubmcbajaVoM92929596;     gHqRGdSVdubmcbajaVoM92929596 = gHqRGdSVdubmcbajaVoM97135184;     gHqRGdSVdubmcbajaVoM97135184 = gHqRGdSVdubmcbajaVoM50571511;     gHqRGdSVdubmcbajaVoM50571511 = gHqRGdSVdubmcbajaVoM54780011;     gHqRGdSVdubmcbajaVoM54780011 = gHqRGdSVdubmcbajaVoM10933964;     gHqRGdSVdubmcbajaVoM10933964 = gHqRGdSVdubmcbajaVoM47500276;     gHqRGdSVdubmcbajaVoM47500276 = gHqRGdSVdubmcbajaVoM29625506;     gHqRGdSVdubmcbajaVoM29625506 = gHqRGdSVdubmcbajaVoM11946602;     gHqRGdSVdubmcbajaVoM11946602 = gHqRGdSVdubmcbajaVoM87708066;     gHqRGdSVdubmcbajaVoM87708066 = gHqRGdSVdubmcbajaVoM15725688;     gHqRGdSVdubmcbajaVoM15725688 = gHqRGdSVdubmcbajaVoM25687389;     gHqRGdSVdubmcbajaVoM25687389 = gHqRGdSVdubmcbajaVoM64241470;     gHqRGdSVdubmcbajaVoM64241470 = gHqRGdSVdubmcbajaVoM74465887;     gHqRGdSVdubmcbajaVoM74465887 = gHqRGdSVdubmcbajaVoM59662578;     gHqRGdSVdubmcbajaVoM59662578 = gHqRGdSVdubmcbajaVoM91933802;     gHqRGdSVdubmcbajaVoM91933802 = gHqRGdSVdubmcbajaVoM7807733;     gHqRGdSVdubmcbajaVoM7807733 = gHqRGdSVdubmcbajaVoM73743261;     gHqRGdSVdubmcbajaVoM73743261 = gHqRGdSVdubmcbajaVoM97050766;     gHqRGdSVdubmcbajaVoM97050766 = gHqRGdSVdubmcbajaVoM94342515;     gHqRGdSVdubmcbajaVoM94342515 = gHqRGdSVdubmcbajaVoM14393449;     gHqRGdSVdubmcbajaVoM14393449 = gHqRGdSVdubmcbajaVoM54469544;     gHqRGdSVdubmcbajaVoM54469544 = gHqRGdSVdubmcbajaVoM8046582;     gHqRGdSVdubmcbajaVoM8046582 = gHqRGdSVdubmcbajaVoM70838523;     gHqRGdSVdubmcbajaVoM70838523 = gHqRGdSVdubmcbajaVoM49551562;     gHqRGdSVdubmcbajaVoM49551562 = gHqRGdSVdubmcbajaVoM45553359;     gHqRGdSVdubmcbajaVoM45553359 = gHqRGdSVdubmcbajaVoM86980622;     gHqRGdSVdubmcbajaVoM86980622 = gHqRGdSVdubmcbajaVoM10163619;     gHqRGdSVdubmcbajaVoM10163619 = gHqRGdSVdubmcbajaVoM98334226;     gHqRGdSVdubmcbajaVoM98334226 = gHqRGdSVdubmcbajaVoM82816926;     gHqRGdSVdubmcbajaVoM82816926 = gHqRGdSVdubmcbajaVoM99108055;     gHqRGdSVdubmcbajaVoM99108055 = gHqRGdSVdubmcbajaVoM25417362;     gHqRGdSVdubmcbajaVoM25417362 = gHqRGdSVdubmcbajaVoM15171481;     gHqRGdSVdubmcbajaVoM15171481 = gHqRGdSVdubmcbajaVoM17701956;     gHqRGdSVdubmcbajaVoM17701956 = gHqRGdSVdubmcbajaVoM89849479;     gHqRGdSVdubmcbajaVoM89849479 = gHqRGdSVdubmcbajaVoM62170195;     gHqRGdSVdubmcbajaVoM62170195 = gHqRGdSVdubmcbajaVoM84066290;     gHqRGdSVdubmcbajaVoM84066290 = gHqRGdSVdubmcbajaVoM35804265;     gHqRGdSVdubmcbajaVoM35804265 = gHqRGdSVdubmcbajaVoM70152258;     gHqRGdSVdubmcbajaVoM70152258 = gHqRGdSVdubmcbajaVoM86770036;     gHqRGdSVdubmcbajaVoM86770036 = gHqRGdSVdubmcbajaVoM32034636;     gHqRGdSVdubmcbajaVoM32034636 = gHqRGdSVdubmcbajaVoM75954447;     gHqRGdSVdubmcbajaVoM75954447 = gHqRGdSVdubmcbajaVoM267125;     gHqRGdSVdubmcbajaVoM267125 = gHqRGdSVdubmcbajaVoM58789726;     gHqRGdSVdubmcbajaVoM58789726 = gHqRGdSVdubmcbajaVoM18937122;     gHqRGdSVdubmcbajaVoM18937122 = gHqRGdSVdubmcbajaVoM48889073;     gHqRGdSVdubmcbajaVoM48889073 = gHqRGdSVdubmcbajaVoM58995747;     gHqRGdSVdubmcbajaVoM58995747 = gHqRGdSVdubmcbajaVoM33532006;     gHqRGdSVdubmcbajaVoM33532006 = gHqRGdSVdubmcbajaVoM19940388;     gHqRGdSVdubmcbajaVoM19940388 = gHqRGdSVdubmcbajaVoM28344923;     gHqRGdSVdubmcbajaVoM28344923 = gHqRGdSVdubmcbajaVoM38509535;     gHqRGdSVdubmcbajaVoM38509535 = gHqRGdSVdubmcbajaVoM68162920;     gHqRGdSVdubmcbajaVoM68162920 = gHqRGdSVdubmcbajaVoM75643419;     gHqRGdSVdubmcbajaVoM75643419 = gHqRGdSVdubmcbajaVoM26972775;     gHqRGdSVdubmcbajaVoM26972775 = gHqRGdSVdubmcbajaVoM46196868;     gHqRGdSVdubmcbajaVoM46196868 = gHqRGdSVdubmcbajaVoM22902855;     gHqRGdSVdubmcbajaVoM22902855 = gHqRGdSVdubmcbajaVoM84883015;     gHqRGdSVdubmcbajaVoM84883015 = gHqRGdSVdubmcbajaVoM42533743;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void lTiAYXCuTHMnBaQqAJZg89768416() {     double aWFKbaxaNVTvDHpamaeG68119019 = -135769829;    double aWFKbaxaNVTvDHpamaeG38084909 = -144258339;    double aWFKbaxaNVTvDHpamaeG47641546 = -700968361;    double aWFKbaxaNVTvDHpamaeG18857206 = -743836131;    double aWFKbaxaNVTvDHpamaeG63525513 = -636219063;    double aWFKbaxaNVTvDHpamaeG15684807 = -810776930;    double aWFKbaxaNVTvDHpamaeG75612 = -161844784;    double aWFKbaxaNVTvDHpamaeG88887551 = -406081754;    double aWFKbaxaNVTvDHpamaeG45302694 = -502832458;    double aWFKbaxaNVTvDHpamaeG85841653 = -118886539;    double aWFKbaxaNVTvDHpamaeG68877928 = -798493234;    double aWFKbaxaNVTvDHpamaeG40818989 = -512928419;    double aWFKbaxaNVTvDHpamaeG708802 = -791832159;    double aWFKbaxaNVTvDHpamaeG60350293 = -887981658;    double aWFKbaxaNVTvDHpamaeG51247403 = -96507820;    double aWFKbaxaNVTvDHpamaeG97487627 = -296690440;    double aWFKbaxaNVTvDHpamaeG18411068 = -575231757;    double aWFKbaxaNVTvDHpamaeG50250098 = -830890679;    double aWFKbaxaNVTvDHpamaeG2478958 = -260144844;    double aWFKbaxaNVTvDHpamaeG59273014 = -378212762;    double aWFKbaxaNVTvDHpamaeG37496037 = -919874377;    double aWFKbaxaNVTvDHpamaeG79795379 = -848817514;    double aWFKbaxaNVTvDHpamaeG70985676 = 63195282;    double aWFKbaxaNVTvDHpamaeG56813835 = 93284749;    double aWFKbaxaNVTvDHpamaeG36104613 = 20438636;    double aWFKbaxaNVTvDHpamaeG15864913 = -671995004;    double aWFKbaxaNVTvDHpamaeG7320179 = -895930787;    double aWFKbaxaNVTvDHpamaeG61602800 = -742280926;    double aWFKbaxaNVTvDHpamaeG87412123 = -895897373;    double aWFKbaxaNVTvDHpamaeG58902444 = -361396190;    double aWFKbaxaNVTvDHpamaeG34260361 = -908461270;    double aWFKbaxaNVTvDHpamaeG83687725 = -487239870;    double aWFKbaxaNVTvDHpamaeG55849861 = -711408273;    double aWFKbaxaNVTvDHpamaeG28163533 = -205985990;    double aWFKbaxaNVTvDHpamaeG24642206 = 34030007;    double aWFKbaxaNVTvDHpamaeG12789743 = -34967239;    double aWFKbaxaNVTvDHpamaeG29248352 = -810330285;    double aWFKbaxaNVTvDHpamaeG95851411 = -157731192;    double aWFKbaxaNVTvDHpamaeG87240845 = -416660554;    double aWFKbaxaNVTvDHpamaeG50689856 = -978675475;    double aWFKbaxaNVTvDHpamaeG19845467 = -792548241;    double aWFKbaxaNVTvDHpamaeG65324881 = -736951639;    double aWFKbaxaNVTvDHpamaeG9790537 = -713689633;    double aWFKbaxaNVTvDHpamaeG48165679 = 53689216;    double aWFKbaxaNVTvDHpamaeG84910473 = -802002551;    double aWFKbaxaNVTvDHpamaeG63535506 = -984475964;    double aWFKbaxaNVTvDHpamaeG50339966 = -785540798;    double aWFKbaxaNVTvDHpamaeG62892782 = -967931337;    double aWFKbaxaNVTvDHpamaeG61745284 = -368109481;    double aWFKbaxaNVTvDHpamaeG1310271 = -137732065;    double aWFKbaxaNVTvDHpamaeG39761754 = -529669032;    double aWFKbaxaNVTvDHpamaeG39205181 = -466765337;    double aWFKbaxaNVTvDHpamaeG5781671 = -382562012;    double aWFKbaxaNVTvDHpamaeG84624597 = -559358991;    double aWFKbaxaNVTvDHpamaeG52272380 = 16797347;    double aWFKbaxaNVTvDHpamaeG88323640 = -286952316;    double aWFKbaxaNVTvDHpamaeG67099232 = -107453621;    double aWFKbaxaNVTvDHpamaeG90827711 = -694253111;    double aWFKbaxaNVTvDHpamaeG82752593 = -664274768;    double aWFKbaxaNVTvDHpamaeG47660600 = -964224060;    double aWFKbaxaNVTvDHpamaeG8364629 = -914846144;    double aWFKbaxaNVTvDHpamaeG38472811 = -419563859;    double aWFKbaxaNVTvDHpamaeG1475428 = -510184381;    double aWFKbaxaNVTvDHpamaeG86400250 = -41436268;    double aWFKbaxaNVTvDHpamaeG51581292 = -210425270;    double aWFKbaxaNVTvDHpamaeG85190203 = -211253365;    double aWFKbaxaNVTvDHpamaeG84969127 = -801520147;    double aWFKbaxaNVTvDHpamaeG72545269 = -485846169;    double aWFKbaxaNVTvDHpamaeG35708088 = -822011666;    double aWFKbaxaNVTvDHpamaeG38457660 = 38459418;    double aWFKbaxaNVTvDHpamaeG68239276 = -486360155;    double aWFKbaxaNVTvDHpamaeG22559657 = -317500565;    double aWFKbaxaNVTvDHpamaeG63009252 = -314230125;    double aWFKbaxaNVTvDHpamaeG51789102 = -281469370;    double aWFKbaxaNVTvDHpamaeG39427548 = -585664521;    double aWFKbaxaNVTvDHpamaeG72171156 = -82922738;    double aWFKbaxaNVTvDHpamaeG70004843 = -35127881;    double aWFKbaxaNVTvDHpamaeG22819998 = -990493935;    double aWFKbaxaNVTvDHpamaeG71903362 = -104712700;    double aWFKbaxaNVTvDHpamaeG72569107 = 4914599;    double aWFKbaxaNVTvDHpamaeG65524947 = -886454206;    double aWFKbaxaNVTvDHpamaeG44427396 = -927999450;    double aWFKbaxaNVTvDHpamaeG99857516 = -274171445;    double aWFKbaxaNVTvDHpamaeG86101853 = -658165309;    double aWFKbaxaNVTvDHpamaeG19140691 = -831727158;    double aWFKbaxaNVTvDHpamaeG95055179 = -341695933;    double aWFKbaxaNVTvDHpamaeG77906055 = -4677858;    double aWFKbaxaNVTvDHpamaeG71225264 = -52049282;    double aWFKbaxaNVTvDHpamaeG75891152 = -122783338;    double aWFKbaxaNVTvDHpamaeG36318565 = -679017677;    double aWFKbaxaNVTvDHpamaeG45690511 = -927513618;    double aWFKbaxaNVTvDHpamaeG38420640 = -16077175;    double aWFKbaxaNVTvDHpamaeG13098819 = -493456425;    double aWFKbaxaNVTvDHpamaeG39580245 = -452436494;    double aWFKbaxaNVTvDHpamaeG42325227 = 36170669;    double aWFKbaxaNVTvDHpamaeG81372655 = -272984382;    double aWFKbaxaNVTvDHpamaeG63849453 = -126767258;    double aWFKbaxaNVTvDHpamaeG23390286 = -572253366;    double aWFKbaxaNVTvDHpamaeG96584386 = -735885515;    double aWFKbaxaNVTvDHpamaeG99720270 = -135769829;     aWFKbaxaNVTvDHpamaeG68119019 = aWFKbaxaNVTvDHpamaeG38084909;     aWFKbaxaNVTvDHpamaeG38084909 = aWFKbaxaNVTvDHpamaeG47641546;     aWFKbaxaNVTvDHpamaeG47641546 = aWFKbaxaNVTvDHpamaeG18857206;     aWFKbaxaNVTvDHpamaeG18857206 = aWFKbaxaNVTvDHpamaeG63525513;     aWFKbaxaNVTvDHpamaeG63525513 = aWFKbaxaNVTvDHpamaeG15684807;     aWFKbaxaNVTvDHpamaeG15684807 = aWFKbaxaNVTvDHpamaeG75612;     aWFKbaxaNVTvDHpamaeG75612 = aWFKbaxaNVTvDHpamaeG88887551;     aWFKbaxaNVTvDHpamaeG88887551 = aWFKbaxaNVTvDHpamaeG45302694;     aWFKbaxaNVTvDHpamaeG45302694 = aWFKbaxaNVTvDHpamaeG85841653;     aWFKbaxaNVTvDHpamaeG85841653 = aWFKbaxaNVTvDHpamaeG68877928;     aWFKbaxaNVTvDHpamaeG68877928 = aWFKbaxaNVTvDHpamaeG40818989;     aWFKbaxaNVTvDHpamaeG40818989 = aWFKbaxaNVTvDHpamaeG708802;     aWFKbaxaNVTvDHpamaeG708802 = aWFKbaxaNVTvDHpamaeG60350293;     aWFKbaxaNVTvDHpamaeG60350293 = aWFKbaxaNVTvDHpamaeG51247403;     aWFKbaxaNVTvDHpamaeG51247403 = aWFKbaxaNVTvDHpamaeG97487627;     aWFKbaxaNVTvDHpamaeG97487627 = aWFKbaxaNVTvDHpamaeG18411068;     aWFKbaxaNVTvDHpamaeG18411068 = aWFKbaxaNVTvDHpamaeG50250098;     aWFKbaxaNVTvDHpamaeG50250098 = aWFKbaxaNVTvDHpamaeG2478958;     aWFKbaxaNVTvDHpamaeG2478958 = aWFKbaxaNVTvDHpamaeG59273014;     aWFKbaxaNVTvDHpamaeG59273014 = aWFKbaxaNVTvDHpamaeG37496037;     aWFKbaxaNVTvDHpamaeG37496037 = aWFKbaxaNVTvDHpamaeG79795379;     aWFKbaxaNVTvDHpamaeG79795379 = aWFKbaxaNVTvDHpamaeG70985676;     aWFKbaxaNVTvDHpamaeG70985676 = aWFKbaxaNVTvDHpamaeG56813835;     aWFKbaxaNVTvDHpamaeG56813835 = aWFKbaxaNVTvDHpamaeG36104613;     aWFKbaxaNVTvDHpamaeG36104613 = aWFKbaxaNVTvDHpamaeG15864913;     aWFKbaxaNVTvDHpamaeG15864913 = aWFKbaxaNVTvDHpamaeG7320179;     aWFKbaxaNVTvDHpamaeG7320179 = aWFKbaxaNVTvDHpamaeG61602800;     aWFKbaxaNVTvDHpamaeG61602800 = aWFKbaxaNVTvDHpamaeG87412123;     aWFKbaxaNVTvDHpamaeG87412123 = aWFKbaxaNVTvDHpamaeG58902444;     aWFKbaxaNVTvDHpamaeG58902444 = aWFKbaxaNVTvDHpamaeG34260361;     aWFKbaxaNVTvDHpamaeG34260361 = aWFKbaxaNVTvDHpamaeG83687725;     aWFKbaxaNVTvDHpamaeG83687725 = aWFKbaxaNVTvDHpamaeG55849861;     aWFKbaxaNVTvDHpamaeG55849861 = aWFKbaxaNVTvDHpamaeG28163533;     aWFKbaxaNVTvDHpamaeG28163533 = aWFKbaxaNVTvDHpamaeG24642206;     aWFKbaxaNVTvDHpamaeG24642206 = aWFKbaxaNVTvDHpamaeG12789743;     aWFKbaxaNVTvDHpamaeG12789743 = aWFKbaxaNVTvDHpamaeG29248352;     aWFKbaxaNVTvDHpamaeG29248352 = aWFKbaxaNVTvDHpamaeG95851411;     aWFKbaxaNVTvDHpamaeG95851411 = aWFKbaxaNVTvDHpamaeG87240845;     aWFKbaxaNVTvDHpamaeG87240845 = aWFKbaxaNVTvDHpamaeG50689856;     aWFKbaxaNVTvDHpamaeG50689856 = aWFKbaxaNVTvDHpamaeG19845467;     aWFKbaxaNVTvDHpamaeG19845467 = aWFKbaxaNVTvDHpamaeG65324881;     aWFKbaxaNVTvDHpamaeG65324881 = aWFKbaxaNVTvDHpamaeG9790537;     aWFKbaxaNVTvDHpamaeG9790537 = aWFKbaxaNVTvDHpamaeG48165679;     aWFKbaxaNVTvDHpamaeG48165679 = aWFKbaxaNVTvDHpamaeG84910473;     aWFKbaxaNVTvDHpamaeG84910473 = aWFKbaxaNVTvDHpamaeG63535506;     aWFKbaxaNVTvDHpamaeG63535506 = aWFKbaxaNVTvDHpamaeG50339966;     aWFKbaxaNVTvDHpamaeG50339966 = aWFKbaxaNVTvDHpamaeG62892782;     aWFKbaxaNVTvDHpamaeG62892782 = aWFKbaxaNVTvDHpamaeG61745284;     aWFKbaxaNVTvDHpamaeG61745284 = aWFKbaxaNVTvDHpamaeG1310271;     aWFKbaxaNVTvDHpamaeG1310271 = aWFKbaxaNVTvDHpamaeG39761754;     aWFKbaxaNVTvDHpamaeG39761754 = aWFKbaxaNVTvDHpamaeG39205181;     aWFKbaxaNVTvDHpamaeG39205181 = aWFKbaxaNVTvDHpamaeG5781671;     aWFKbaxaNVTvDHpamaeG5781671 = aWFKbaxaNVTvDHpamaeG84624597;     aWFKbaxaNVTvDHpamaeG84624597 = aWFKbaxaNVTvDHpamaeG52272380;     aWFKbaxaNVTvDHpamaeG52272380 = aWFKbaxaNVTvDHpamaeG88323640;     aWFKbaxaNVTvDHpamaeG88323640 = aWFKbaxaNVTvDHpamaeG67099232;     aWFKbaxaNVTvDHpamaeG67099232 = aWFKbaxaNVTvDHpamaeG90827711;     aWFKbaxaNVTvDHpamaeG90827711 = aWFKbaxaNVTvDHpamaeG82752593;     aWFKbaxaNVTvDHpamaeG82752593 = aWFKbaxaNVTvDHpamaeG47660600;     aWFKbaxaNVTvDHpamaeG47660600 = aWFKbaxaNVTvDHpamaeG8364629;     aWFKbaxaNVTvDHpamaeG8364629 = aWFKbaxaNVTvDHpamaeG38472811;     aWFKbaxaNVTvDHpamaeG38472811 = aWFKbaxaNVTvDHpamaeG1475428;     aWFKbaxaNVTvDHpamaeG1475428 = aWFKbaxaNVTvDHpamaeG86400250;     aWFKbaxaNVTvDHpamaeG86400250 = aWFKbaxaNVTvDHpamaeG51581292;     aWFKbaxaNVTvDHpamaeG51581292 = aWFKbaxaNVTvDHpamaeG85190203;     aWFKbaxaNVTvDHpamaeG85190203 = aWFKbaxaNVTvDHpamaeG84969127;     aWFKbaxaNVTvDHpamaeG84969127 = aWFKbaxaNVTvDHpamaeG72545269;     aWFKbaxaNVTvDHpamaeG72545269 = aWFKbaxaNVTvDHpamaeG35708088;     aWFKbaxaNVTvDHpamaeG35708088 = aWFKbaxaNVTvDHpamaeG38457660;     aWFKbaxaNVTvDHpamaeG38457660 = aWFKbaxaNVTvDHpamaeG68239276;     aWFKbaxaNVTvDHpamaeG68239276 = aWFKbaxaNVTvDHpamaeG22559657;     aWFKbaxaNVTvDHpamaeG22559657 = aWFKbaxaNVTvDHpamaeG63009252;     aWFKbaxaNVTvDHpamaeG63009252 = aWFKbaxaNVTvDHpamaeG51789102;     aWFKbaxaNVTvDHpamaeG51789102 = aWFKbaxaNVTvDHpamaeG39427548;     aWFKbaxaNVTvDHpamaeG39427548 = aWFKbaxaNVTvDHpamaeG72171156;     aWFKbaxaNVTvDHpamaeG72171156 = aWFKbaxaNVTvDHpamaeG70004843;     aWFKbaxaNVTvDHpamaeG70004843 = aWFKbaxaNVTvDHpamaeG22819998;     aWFKbaxaNVTvDHpamaeG22819998 = aWFKbaxaNVTvDHpamaeG71903362;     aWFKbaxaNVTvDHpamaeG71903362 = aWFKbaxaNVTvDHpamaeG72569107;     aWFKbaxaNVTvDHpamaeG72569107 = aWFKbaxaNVTvDHpamaeG65524947;     aWFKbaxaNVTvDHpamaeG65524947 = aWFKbaxaNVTvDHpamaeG44427396;     aWFKbaxaNVTvDHpamaeG44427396 = aWFKbaxaNVTvDHpamaeG99857516;     aWFKbaxaNVTvDHpamaeG99857516 = aWFKbaxaNVTvDHpamaeG86101853;     aWFKbaxaNVTvDHpamaeG86101853 = aWFKbaxaNVTvDHpamaeG19140691;     aWFKbaxaNVTvDHpamaeG19140691 = aWFKbaxaNVTvDHpamaeG95055179;     aWFKbaxaNVTvDHpamaeG95055179 = aWFKbaxaNVTvDHpamaeG77906055;     aWFKbaxaNVTvDHpamaeG77906055 = aWFKbaxaNVTvDHpamaeG71225264;     aWFKbaxaNVTvDHpamaeG71225264 = aWFKbaxaNVTvDHpamaeG75891152;     aWFKbaxaNVTvDHpamaeG75891152 = aWFKbaxaNVTvDHpamaeG36318565;     aWFKbaxaNVTvDHpamaeG36318565 = aWFKbaxaNVTvDHpamaeG45690511;     aWFKbaxaNVTvDHpamaeG45690511 = aWFKbaxaNVTvDHpamaeG38420640;     aWFKbaxaNVTvDHpamaeG38420640 = aWFKbaxaNVTvDHpamaeG13098819;     aWFKbaxaNVTvDHpamaeG13098819 = aWFKbaxaNVTvDHpamaeG39580245;     aWFKbaxaNVTvDHpamaeG39580245 = aWFKbaxaNVTvDHpamaeG42325227;     aWFKbaxaNVTvDHpamaeG42325227 = aWFKbaxaNVTvDHpamaeG81372655;     aWFKbaxaNVTvDHpamaeG81372655 = aWFKbaxaNVTvDHpamaeG63849453;     aWFKbaxaNVTvDHpamaeG63849453 = aWFKbaxaNVTvDHpamaeG23390286;     aWFKbaxaNVTvDHpamaeG23390286 = aWFKbaxaNVTvDHpamaeG96584386;     aWFKbaxaNVTvDHpamaeG96584386 = aWFKbaxaNVTvDHpamaeG99720270;     aWFKbaxaNVTvDHpamaeG99720270 = aWFKbaxaNVTvDHpamaeG68119019;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ecbDfQvPmlWYfhtMcvZg17362674() {     double GOXyDYgHsYPiPALoCkpV93704296 = -660802930;    double GOXyDYgHsYPiPALoCkpV34482052 = -399736116;    double GOXyDYgHsYPiPALoCkpV80520721 = -503429897;    double GOXyDYgHsYPiPALoCkpV64579136 = -751159025;    double GOXyDYgHsYPiPALoCkpV32867518 = -683141351;    double GOXyDYgHsYPiPALoCkpV32694084 = -965757934;    double GOXyDYgHsYPiPALoCkpV5396456 = -744115991;    double GOXyDYgHsYPiPALoCkpV3897676 = -786763633;    double GOXyDYgHsYPiPALoCkpV70631987 = -999056502;    double GOXyDYgHsYPiPALoCkpV5000036 = -453448062;    double GOXyDYgHsYPiPALoCkpV83211482 = -8921121;    double GOXyDYgHsYPiPALoCkpV76136644 = -956637326;    double GOXyDYgHsYPiPALoCkpV77289579 = -556080524;    double GOXyDYgHsYPiPALoCkpV51910010 = -862505951;    double GOXyDYgHsYPiPALoCkpV7516291 = 14946222;    double GOXyDYgHsYPiPALoCkpV5208671 = -732881917;    double GOXyDYgHsYPiPALoCkpV18209184 = -263551551;    double GOXyDYgHsYPiPALoCkpV71366002 = -247168039;    double GOXyDYgHsYPiPALoCkpV63943679 = 56881298;    double GOXyDYgHsYPiPALoCkpV20434481 = -352652137;    double GOXyDYgHsYPiPALoCkpV38505303 = -281574158;    double GOXyDYgHsYPiPALoCkpV81298486 = -168550443;    double GOXyDYgHsYPiPALoCkpV74749474 = 19103176;    double GOXyDYgHsYPiPALoCkpV58527877 = -865564230;    double GOXyDYgHsYPiPALoCkpV91007752 = -647057507;    double GOXyDYgHsYPiPALoCkpV45354050 = -236445546;    double GOXyDYgHsYPiPALoCkpV89708087 = -461743922;    double GOXyDYgHsYPiPALoCkpV25501600 = -971943158;    double GOXyDYgHsYPiPALoCkpV95289336 = -32191223;    double GOXyDYgHsYPiPALoCkpV12224935 = -270305752;    double GOXyDYgHsYPiPALoCkpV56306995 = -687108300;    double GOXyDYgHsYPiPALoCkpV20877658 = -761035704;    double GOXyDYgHsYPiPALoCkpV77036913 = -59229410;    double GOXyDYgHsYPiPALoCkpV81750603 = -262130962;    double GOXyDYgHsYPiPALoCkpV26047194 = -667405904;    double GOXyDYgHsYPiPALoCkpV17581594 = -552561955;    double GOXyDYgHsYPiPALoCkpV78893737 = -190842230;    double GOXyDYgHsYPiPALoCkpV71424098 = -6446017;    double GOXyDYgHsYPiPALoCkpV28164425 = -874998876;    double GOXyDYgHsYPiPALoCkpV59473529 = -348645794;    double GOXyDYgHsYPiPALoCkpV66996747 = -267608430;    double GOXyDYgHsYPiPALoCkpV9334471 = -145990143;    double GOXyDYgHsYPiPALoCkpV58990755 = -742296015;    double GOXyDYgHsYPiPALoCkpV18958958 = -414726383;    double GOXyDYgHsYPiPALoCkpV76891349 = -741023668;    double GOXyDYgHsYPiPALoCkpV29935829 = -407023501;    double GOXyDYgHsYPiPALoCkpV50108420 = -818009659;    double GOXyDYgHsYPiPALoCkpV71005553 = -61114184;    double GOXyDYgHsYPiPALoCkpV12556604 = -467785708;    double GOXyDYgHsYPiPALoCkpV55120266 = 14244782;    double GOXyDYgHsYPiPALoCkpV49898001 = -778327635;    double GOXyDYgHsYPiPALoCkpV66463760 = -644844516;    double GOXyDYgHsYPiPALoCkpV23855274 = -950460561;    double GOXyDYgHsYPiPALoCkpV53523506 = -874422773;    double GOXyDYgHsYPiPALoCkpV78857371 = -458293071;    double GOXyDYgHsYPiPALoCkpV12405811 = -392252487;    double GOXyDYgHsYPiPALoCkpV59732578 = -318839293;    double GOXyDYgHsYPiPALoCkpV21992845 = -637865667;    double GOXyDYgHsYPiPALoCkpV73571383 = -4101519;    double GOXyDYgHsYPiPALoCkpV87513468 = -346695806;    double GOXyDYgHsYPiPALoCkpV42985996 = -404014013;    double GOXyDYgHsYPiPALoCkpV79894855 = -772172833;    double GOXyDYgHsYPiPALoCkpV8608340 = -654572410;    double GOXyDYgHsYPiPALoCkpV58407052 = -628750751;    double GOXyDYgHsYPiPALoCkpV48693040 = -766339762;    double GOXyDYgHsYPiPALoCkpV62333824 = -247885417;    double GOXyDYgHsYPiPALoCkpV99099731 = -797407916;    double GOXyDYgHsYPiPALoCkpV95538975 = -193949563;    double GOXyDYgHsYPiPALoCkpV25862817 = -95100048;    double GOXyDYgHsYPiPALoCkpV89934697 = -432491824;    double GOXyDYgHsYPiPALoCkpV26314934 = -442039688;    double GOXyDYgHsYPiPALoCkpV46785086 = -157105534;    double GOXyDYgHsYPiPALoCkpV43201578 = -372169164;    double GOXyDYgHsYPiPALoCkpV4470150 = -594472908;    double GOXyDYgHsYPiPALoCkpV53437733 = 14956293;    double GOXyDYgHsYPiPALoCkpV29170832 = -35584015;    double GOXyDYgHsYPiPALoCkpV22307731 = -426254429;    double GOXyDYgHsYPiPALoCkpV55790516 = -566170441;    double GOXyDYgHsYPiPALoCkpV81636528 = -24540562;    double GOXyDYgHsYPiPALoCkpV61071924 = -140034006;    double GOXyDYgHsYPiPALoCkpV95245630 = -418435887;    double GOXyDYgHsYPiPALoCkpV18702534 = -300629738;    double GOXyDYgHsYPiPALoCkpV12944997 = -404157451;    double GOXyDYgHsYPiPALoCkpV40169070 = 53563994;    double GOXyDYgHsYPiPALoCkpV62326934 = -491978118;    double GOXyDYgHsYPiPALoCkpV89843234 = 57736215;    double GOXyDYgHsYPiPALoCkpV97022384 = -810575144;    double GOXyDYgHsYPiPALoCkpV23513408 = -184806638;    double GOXyDYgHsYPiPALoCkpV2893233 = -803837892;    double GOXyDYgHsYPiPALoCkpV13641383 = -175153417;    double GOXyDYgHsYPiPALoCkpV57849016 = -133722662;    double GOXyDYgHsYPiPALoCkpV56900892 = -552976563;    double GOXyDYgHsYPiPALoCkpV97852715 = 97655501;    double GOXyDYgHsYPiPALoCkpV40650956 = -428303071;    double GOXyDYgHsYPiPALoCkpV16487534 = -944631782;    double GOXyDYgHsYPiPALoCkpV87101892 = -495435598;    double GOXyDYgHsYPiPALoCkpV726132 = -491417734;    double GOXyDYgHsYPiPALoCkpV583704 = -13545264;    double GOXyDYgHsYPiPALoCkpV70265917 = -648386622;    double GOXyDYgHsYPiPALoCkpV14557525 = -660802930;     GOXyDYgHsYPiPALoCkpV93704296 = GOXyDYgHsYPiPALoCkpV34482052;     GOXyDYgHsYPiPALoCkpV34482052 = GOXyDYgHsYPiPALoCkpV80520721;     GOXyDYgHsYPiPALoCkpV80520721 = GOXyDYgHsYPiPALoCkpV64579136;     GOXyDYgHsYPiPALoCkpV64579136 = GOXyDYgHsYPiPALoCkpV32867518;     GOXyDYgHsYPiPALoCkpV32867518 = GOXyDYgHsYPiPALoCkpV32694084;     GOXyDYgHsYPiPALoCkpV32694084 = GOXyDYgHsYPiPALoCkpV5396456;     GOXyDYgHsYPiPALoCkpV5396456 = GOXyDYgHsYPiPALoCkpV3897676;     GOXyDYgHsYPiPALoCkpV3897676 = GOXyDYgHsYPiPALoCkpV70631987;     GOXyDYgHsYPiPALoCkpV70631987 = GOXyDYgHsYPiPALoCkpV5000036;     GOXyDYgHsYPiPALoCkpV5000036 = GOXyDYgHsYPiPALoCkpV83211482;     GOXyDYgHsYPiPALoCkpV83211482 = GOXyDYgHsYPiPALoCkpV76136644;     GOXyDYgHsYPiPALoCkpV76136644 = GOXyDYgHsYPiPALoCkpV77289579;     GOXyDYgHsYPiPALoCkpV77289579 = GOXyDYgHsYPiPALoCkpV51910010;     GOXyDYgHsYPiPALoCkpV51910010 = GOXyDYgHsYPiPALoCkpV7516291;     GOXyDYgHsYPiPALoCkpV7516291 = GOXyDYgHsYPiPALoCkpV5208671;     GOXyDYgHsYPiPALoCkpV5208671 = GOXyDYgHsYPiPALoCkpV18209184;     GOXyDYgHsYPiPALoCkpV18209184 = GOXyDYgHsYPiPALoCkpV71366002;     GOXyDYgHsYPiPALoCkpV71366002 = GOXyDYgHsYPiPALoCkpV63943679;     GOXyDYgHsYPiPALoCkpV63943679 = GOXyDYgHsYPiPALoCkpV20434481;     GOXyDYgHsYPiPALoCkpV20434481 = GOXyDYgHsYPiPALoCkpV38505303;     GOXyDYgHsYPiPALoCkpV38505303 = GOXyDYgHsYPiPALoCkpV81298486;     GOXyDYgHsYPiPALoCkpV81298486 = GOXyDYgHsYPiPALoCkpV74749474;     GOXyDYgHsYPiPALoCkpV74749474 = GOXyDYgHsYPiPALoCkpV58527877;     GOXyDYgHsYPiPALoCkpV58527877 = GOXyDYgHsYPiPALoCkpV91007752;     GOXyDYgHsYPiPALoCkpV91007752 = GOXyDYgHsYPiPALoCkpV45354050;     GOXyDYgHsYPiPALoCkpV45354050 = GOXyDYgHsYPiPALoCkpV89708087;     GOXyDYgHsYPiPALoCkpV89708087 = GOXyDYgHsYPiPALoCkpV25501600;     GOXyDYgHsYPiPALoCkpV25501600 = GOXyDYgHsYPiPALoCkpV95289336;     GOXyDYgHsYPiPALoCkpV95289336 = GOXyDYgHsYPiPALoCkpV12224935;     GOXyDYgHsYPiPALoCkpV12224935 = GOXyDYgHsYPiPALoCkpV56306995;     GOXyDYgHsYPiPALoCkpV56306995 = GOXyDYgHsYPiPALoCkpV20877658;     GOXyDYgHsYPiPALoCkpV20877658 = GOXyDYgHsYPiPALoCkpV77036913;     GOXyDYgHsYPiPALoCkpV77036913 = GOXyDYgHsYPiPALoCkpV81750603;     GOXyDYgHsYPiPALoCkpV81750603 = GOXyDYgHsYPiPALoCkpV26047194;     GOXyDYgHsYPiPALoCkpV26047194 = GOXyDYgHsYPiPALoCkpV17581594;     GOXyDYgHsYPiPALoCkpV17581594 = GOXyDYgHsYPiPALoCkpV78893737;     GOXyDYgHsYPiPALoCkpV78893737 = GOXyDYgHsYPiPALoCkpV71424098;     GOXyDYgHsYPiPALoCkpV71424098 = GOXyDYgHsYPiPALoCkpV28164425;     GOXyDYgHsYPiPALoCkpV28164425 = GOXyDYgHsYPiPALoCkpV59473529;     GOXyDYgHsYPiPALoCkpV59473529 = GOXyDYgHsYPiPALoCkpV66996747;     GOXyDYgHsYPiPALoCkpV66996747 = GOXyDYgHsYPiPALoCkpV9334471;     GOXyDYgHsYPiPALoCkpV9334471 = GOXyDYgHsYPiPALoCkpV58990755;     GOXyDYgHsYPiPALoCkpV58990755 = GOXyDYgHsYPiPALoCkpV18958958;     GOXyDYgHsYPiPALoCkpV18958958 = GOXyDYgHsYPiPALoCkpV76891349;     GOXyDYgHsYPiPALoCkpV76891349 = GOXyDYgHsYPiPALoCkpV29935829;     GOXyDYgHsYPiPALoCkpV29935829 = GOXyDYgHsYPiPALoCkpV50108420;     GOXyDYgHsYPiPALoCkpV50108420 = GOXyDYgHsYPiPALoCkpV71005553;     GOXyDYgHsYPiPALoCkpV71005553 = GOXyDYgHsYPiPALoCkpV12556604;     GOXyDYgHsYPiPALoCkpV12556604 = GOXyDYgHsYPiPALoCkpV55120266;     GOXyDYgHsYPiPALoCkpV55120266 = GOXyDYgHsYPiPALoCkpV49898001;     GOXyDYgHsYPiPALoCkpV49898001 = GOXyDYgHsYPiPALoCkpV66463760;     GOXyDYgHsYPiPALoCkpV66463760 = GOXyDYgHsYPiPALoCkpV23855274;     GOXyDYgHsYPiPALoCkpV23855274 = GOXyDYgHsYPiPALoCkpV53523506;     GOXyDYgHsYPiPALoCkpV53523506 = GOXyDYgHsYPiPALoCkpV78857371;     GOXyDYgHsYPiPALoCkpV78857371 = GOXyDYgHsYPiPALoCkpV12405811;     GOXyDYgHsYPiPALoCkpV12405811 = GOXyDYgHsYPiPALoCkpV59732578;     GOXyDYgHsYPiPALoCkpV59732578 = GOXyDYgHsYPiPALoCkpV21992845;     GOXyDYgHsYPiPALoCkpV21992845 = GOXyDYgHsYPiPALoCkpV73571383;     GOXyDYgHsYPiPALoCkpV73571383 = GOXyDYgHsYPiPALoCkpV87513468;     GOXyDYgHsYPiPALoCkpV87513468 = GOXyDYgHsYPiPALoCkpV42985996;     GOXyDYgHsYPiPALoCkpV42985996 = GOXyDYgHsYPiPALoCkpV79894855;     GOXyDYgHsYPiPALoCkpV79894855 = GOXyDYgHsYPiPALoCkpV8608340;     GOXyDYgHsYPiPALoCkpV8608340 = GOXyDYgHsYPiPALoCkpV58407052;     GOXyDYgHsYPiPALoCkpV58407052 = GOXyDYgHsYPiPALoCkpV48693040;     GOXyDYgHsYPiPALoCkpV48693040 = GOXyDYgHsYPiPALoCkpV62333824;     GOXyDYgHsYPiPALoCkpV62333824 = GOXyDYgHsYPiPALoCkpV99099731;     GOXyDYgHsYPiPALoCkpV99099731 = GOXyDYgHsYPiPALoCkpV95538975;     GOXyDYgHsYPiPALoCkpV95538975 = GOXyDYgHsYPiPALoCkpV25862817;     GOXyDYgHsYPiPALoCkpV25862817 = GOXyDYgHsYPiPALoCkpV89934697;     GOXyDYgHsYPiPALoCkpV89934697 = GOXyDYgHsYPiPALoCkpV26314934;     GOXyDYgHsYPiPALoCkpV26314934 = GOXyDYgHsYPiPALoCkpV46785086;     GOXyDYgHsYPiPALoCkpV46785086 = GOXyDYgHsYPiPALoCkpV43201578;     GOXyDYgHsYPiPALoCkpV43201578 = GOXyDYgHsYPiPALoCkpV4470150;     GOXyDYgHsYPiPALoCkpV4470150 = GOXyDYgHsYPiPALoCkpV53437733;     GOXyDYgHsYPiPALoCkpV53437733 = GOXyDYgHsYPiPALoCkpV29170832;     GOXyDYgHsYPiPALoCkpV29170832 = GOXyDYgHsYPiPALoCkpV22307731;     GOXyDYgHsYPiPALoCkpV22307731 = GOXyDYgHsYPiPALoCkpV55790516;     GOXyDYgHsYPiPALoCkpV55790516 = GOXyDYgHsYPiPALoCkpV81636528;     GOXyDYgHsYPiPALoCkpV81636528 = GOXyDYgHsYPiPALoCkpV61071924;     GOXyDYgHsYPiPALoCkpV61071924 = GOXyDYgHsYPiPALoCkpV95245630;     GOXyDYgHsYPiPALoCkpV95245630 = GOXyDYgHsYPiPALoCkpV18702534;     GOXyDYgHsYPiPALoCkpV18702534 = GOXyDYgHsYPiPALoCkpV12944997;     GOXyDYgHsYPiPALoCkpV12944997 = GOXyDYgHsYPiPALoCkpV40169070;     GOXyDYgHsYPiPALoCkpV40169070 = GOXyDYgHsYPiPALoCkpV62326934;     GOXyDYgHsYPiPALoCkpV62326934 = GOXyDYgHsYPiPALoCkpV89843234;     GOXyDYgHsYPiPALoCkpV89843234 = GOXyDYgHsYPiPALoCkpV97022384;     GOXyDYgHsYPiPALoCkpV97022384 = GOXyDYgHsYPiPALoCkpV23513408;     GOXyDYgHsYPiPALoCkpV23513408 = GOXyDYgHsYPiPALoCkpV2893233;     GOXyDYgHsYPiPALoCkpV2893233 = GOXyDYgHsYPiPALoCkpV13641383;     GOXyDYgHsYPiPALoCkpV13641383 = GOXyDYgHsYPiPALoCkpV57849016;     GOXyDYgHsYPiPALoCkpV57849016 = GOXyDYgHsYPiPALoCkpV56900892;     GOXyDYgHsYPiPALoCkpV56900892 = GOXyDYgHsYPiPALoCkpV97852715;     GOXyDYgHsYPiPALoCkpV97852715 = GOXyDYgHsYPiPALoCkpV40650956;     GOXyDYgHsYPiPALoCkpV40650956 = GOXyDYgHsYPiPALoCkpV16487534;     GOXyDYgHsYPiPALoCkpV16487534 = GOXyDYgHsYPiPALoCkpV87101892;     GOXyDYgHsYPiPALoCkpV87101892 = GOXyDYgHsYPiPALoCkpV726132;     GOXyDYgHsYPiPALoCkpV726132 = GOXyDYgHsYPiPALoCkpV583704;     GOXyDYgHsYPiPALoCkpV583704 = GOXyDYgHsYPiPALoCkpV70265917;     GOXyDYgHsYPiPALoCkpV70265917 = GOXyDYgHsYPiPALoCkpV14557525;     GOXyDYgHsYPiPALoCkpV14557525 = GOXyDYgHsYPiPALoCkpV93704296;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HWbgmxshckurDHNTtjiA44956930() {     double ZmxizyVhfpSJgWkXcdAB19289574 = -85836031;    double ZmxizyVhfpSJgWkXcdAB30879195 = -655213893;    double ZmxizyVhfpSJgWkXcdAB13399898 = -305891433;    double ZmxizyVhfpSJgWkXcdAB10301066 = -758481918;    double ZmxizyVhfpSJgWkXcdAB2209523 = -730063638;    double ZmxizyVhfpSJgWkXcdAB49703360 = -20738937;    double ZmxizyVhfpSJgWkXcdAB10717300 = -226387198;    double ZmxizyVhfpSJgWkXcdAB18907800 = -67445512;    double ZmxizyVhfpSJgWkXcdAB95961279 = -395280547;    double ZmxizyVhfpSJgWkXcdAB24158418 = -788009586;    double ZmxizyVhfpSJgWkXcdAB97545036 = -319349007;    double ZmxizyVhfpSJgWkXcdAB11454301 = -300346233;    double ZmxizyVhfpSJgWkXcdAB53870357 = -320328890;    double ZmxizyVhfpSJgWkXcdAB43469727 = -837030245;    double ZmxizyVhfpSJgWkXcdAB63785179 = -973599736;    double ZmxizyVhfpSJgWkXcdAB12929714 = -69073394;    double ZmxizyVhfpSJgWkXcdAB18007301 = 48128655;    double ZmxizyVhfpSJgWkXcdAB92481906 = -763445400;    double ZmxizyVhfpSJgWkXcdAB25408400 = -726092559;    double ZmxizyVhfpSJgWkXcdAB81595947 = -327091513;    double ZmxizyVhfpSJgWkXcdAB39514568 = -743273939;    double ZmxizyVhfpSJgWkXcdAB82801592 = -588283373;    double ZmxizyVhfpSJgWkXcdAB78513272 = -24988929;    double ZmxizyVhfpSJgWkXcdAB60241920 = -724413210;    double ZmxizyVhfpSJgWkXcdAB45910892 = -214553649;    double ZmxizyVhfpSJgWkXcdAB74843186 = -900896087;    double ZmxizyVhfpSJgWkXcdAB72095996 = -27557056;    double ZmxizyVhfpSJgWkXcdAB89400400 = -101605391;    double ZmxizyVhfpSJgWkXcdAB3166549 = -268485074;    double ZmxizyVhfpSJgWkXcdAB65547426 = -179215314;    double ZmxizyVhfpSJgWkXcdAB78353629 = -465755331;    double ZmxizyVhfpSJgWkXcdAB58067590 = 65168462;    double ZmxizyVhfpSJgWkXcdAB98223965 = -507050547;    double ZmxizyVhfpSJgWkXcdAB35337675 = -318275933;    double ZmxizyVhfpSJgWkXcdAB27452182 = -268841816;    double ZmxizyVhfpSJgWkXcdAB22373444 = 29843329;    double ZmxizyVhfpSJgWkXcdAB28539122 = -671354174;    double ZmxizyVhfpSJgWkXcdAB46996785 = -955160842;    double ZmxizyVhfpSJgWkXcdAB69088003 = -233337198;    double ZmxizyVhfpSJgWkXcdAB68257203 = -818616113;    double ZmxizyVhfpSJgWkXcdAB14148029 = -842668620;    double ZmxizyVhfpSJgWkXcdAB53344061 = -655028647;    double ZmxizyVhfpSJgWkXcdAB8190975 = -770902396;    double ZmxizyVhfpSJgWkXcdAB89752236 = -883141982;    double ZmxizyVhfpSJgWkXcdAB68872225 = -680044786;    double ZmxizyVhfpSJgWkXcdAB96336150 = -929571039;    double ZmxizyVhfpSJgWkXcdAB49876874 = -850478519;    double ZmxizyVhfpSJgWkXcdAB79118324 = -254297031;    double ZmxizyVhfpSJgWkXcdAB63367924 = -567461935;    double ZmxizyVhfpSJgWkXcdAB8930261 = -933778371;    double ZmxizyVhfpSJgWkXcdAB60034248 = 73013763;    double ZmxizyVhfpSJgWkXcdAB93722340 = -822923696;    double ZmxizyVhfpSJgWkXcdAB41928878 = -418359109;    double ZmxizyVhfpSJgWkXcdAB22422415 = -89486554;    double ZmxizyVhfpSJgWkXcdAB5442363 = -933383488;    double ZmxizyVhfpSJgWkXcdAB36487981 = -497552659;    double ZmxizyVhfpSJgWkXcdAB52365923 = -530224964;    double ZmxizyVhfpSJgWkXcdAB53157977 = -581478224;    double ZmxizyVhfpSJgWkXcdAB64390173 = -443928270;    double ZmxizyVhfpSJgWkXcdAB27366337 = -829167551;    double ZmxizyVhfpSJgWkXcdAB77607363 = -993181881;    double ZmxizyVhfpSJgWkXcdAB21316900 = -24781808;    double ZmxizyVhfpSJgWkXcdAB15741252 = -798960439;    double ZmxizyVhfpSJgWkXcdAB30413854 = -116065234;    double ZmxizyVhfpSJgWkXcdAB45804788 = -222254255;    double ZmxizyVhfpSJgWkXcdAB39477446 = -284517470;    double ZmxizyVhfpSJgWkXcdAB13230335 = -793295686;    double ZmxizyVhfpSJgWkXcdAB18532683 = 97947043;    double ZmxizyVhfpSJgWkXcdAB16017546 = -468188430;    double ZmxizyVhfpSJgWkXcdAB41411735 = -903443066;    double ZmxizyVhfpSJgWkXcdAB84390591 = -397719220;    double ZmxizyVhfpSJgWkXcdAB71010515 = 3289497;    double ZmxizyVhfpSJgWkXcdAB23393904 = -430108203;    double ZmxizyVhfpSJgWkXcdAB57151197 = -907476446;    double ZmxizyVhfpSJgWkXcdAB67447919 = -484422893;    double ZmxizyVhfpSJgWkXcdAB86170506 = 11754708;    double ZmxizyVhfpSJgWkXcdAB74610618 = -817380977;    double ZmxizyVhfpSJgWkXcdAB88761035 = -141846947;    double ZmxizyVhfpSJgWkXcdAB91369694 = 55631575;    double ZmxizyVhfpSJgWkXcdAB49574741 = -284982611;    double ZmxizyVhfpSJgWkXcdAB24966313 = 49582432;    double ZmxizyVhfpSJgWkXcdAB92977672 = -773260026;    double ZmxizyVhfpSJgWkXcdAB26032477 = -534143457;    double ZmxizyVhfpSJgWkXcdAB94236287 = -334706703;    double ZmxizyVhfpSJgWkXcdAB5513178 = -152229077;    double ZmxizyVhfpSJgWkXcdAB84631289 = -642831636;    double ZmxizyVhfpSJgWkXcdAB16138713 = -516472429;    double ZmxizyVhfpSJgWkXcdAB75801550 = -317563994;    double ZmxizyVhfpSJgWkXcdAB29895312 = -384892446;    double ZmxizyVhfpSJgWkXcdAB90964200 = -771289158;    double ZmxizyVhfpSJgWkXcdAB70007521 = -439931707;    double ZmxizyVhfpSJgWkXcdAB75381144 = 10124049;    double ZmxizyVhfpSJgWkXcdAB82606611 = -411232573;    double ZmxizyVhfpSJgWkXcdAB41721666 = -404169647;    double ZmxizyVhfpSJgWkXcdAB90649839 = -825434232;    double ZmxizyVhfpSJgWkXcdAB92831129 = -717886813;    double ZmxizyVhfpSJgWkXcdAB37602810 = -856068209;    double ZmxizyVhfpSJgWkXcdAB77777121 = -554837163;    double ZmxizyVhfpSJgWkXcdAB43947449 = -560887728;    double ZmxizyVhfpSJgWkXcdAB29394780 = -85836031;     ZmxizyVhfpSJgWkXcdAB19289574 = ZmxizyVhfpSJgWkXcdAB30879195;     ZmxizyVhfpSJgWkXcdAB30879195 = ZmxizyVhfpSJgWkXcdAB13399898;     ZmxizyVhfpSJgWkXcdAB13399898 = ZmxizyVhfpSJgWkXcdAB10301066;     ZmxizyVhfpSJgWkXcdAB10301066 = ZmxizyVhfpSJgWkXcdAB2209523;     ZmxizyVhfpSJgWkXcdAB2209523 = ZmxizyVhfpSJgWkXcdAB49703360;     ZmxizyVhfpSJgWkXcdAB49703360 = ZmxizyVhfpSJgWkXcdAB10717300;     ZmxizyVhfpSJgWkXcdAB10717300 = ZmxizyVhfpSJgWkXcdAB18907800;     ZmxizyVhfpSJgWkXcdAB18907800 = ZmxizyVhfpSJgWkXcdAB95961279;     ZmxizyVhfpSJgWkXcdAB95961279 = ZmxizyVhfpSJgWkXcdAB24158418;     ZmxizyVhfpSJgWkXcdAB24158418 = ZmxizyVhfpSJgWkXcdAB97545036;     ZmxizyVhfpSJgWkXcdAB97545036 = ZmxizyVhfpSJgWkXcdAB11454301;     ZmxizyVhfpSJgWkXcdAB11454301 = ZmxizyVhfpSJgWkXcdAB53870357;     ZmxizyVhfpSJgWkXcdAB53870357 = ZmxizyVhfpSJgWkXcdAB43469727;     ZmxizyVhfpSJgWkXcdAB43469727 = ZmxizyVhfpSJgWkXcdAB63785179;     ZmxizyVhfpSJgWkXcdAB63785179 = ZmxizyVhfpSJgWkXcdAB12929714;     ZmxizyVhfpSJgWkXcdAB12929714 = ZmxizyVhfpSJgWkXcdAB18007301;     ZmxizyVhfpSJgWkXcdAB18007301 = ZmxizyVhfpSJgWkXcdAB92481906;     ZmxizyVhfpSJgWkXcdAB92481906 = ZmxizyVhfpSJgWkXcdAB25408400;     ZmxizyVhfpSJgWkXcdAB25408400 = ZmxizyVhfpSJgWkXcdAB81595947;     ZmxizyVhfpSJgWkXcdAB81595947 = ZmxizyVhfpSJgWkXcdAB39514568;     ZmxizyVhfpSJgWkXcdAB39514568 = ZmxizyVhfpSJgWkXcdAB82801592;     ZmxizyVhfpSJgWkXcdAB82801592 = ZmxizyVhfpSJgWkXcdAB78513272;     ZmxizyVhfpSJgWkXcdAB78513272 = ZmxizyVhfpSJgWkXcdAB60241920;     ZmxizyVhfpSJgWkXcdAB60241920 = ZmxizyVhfpSJgWkXcdAB45910892;     ZmxizyVhfpSJgWkXcdAB45910892 = ZmxizyVhfpSJgWkXcdAB74843186;     ZmxizyVhfpSJgWkXcdAB74843186 = ZmxizyVhfpSJgWkXcdAB72095996;     ZmxizyVhfpSJgWkXcdAB72095996 = ZmxizyVhfpSJgWkXcdAB89400400;     ZmxizyVhfpSJgWkXcdAB89400400 = ZmxizyVhfpSJgWkXcdAB3166549;     ZmxizyVhfpSJgWkXcdAB3166549 = ZmxizyVhfpSJgWkXcdAB65547426;     ZmxizyVhfpSJgWkXcdAB65547426 = ZmxizyVhfpSJgWkXcdAB78353629;     ZmxizyVhfpSJgWkXcdAB78353629 = ZmxizyVhfpSJgWkXcdAB58067590;     ZmxizyVhfpSJgWkXcdAB58067590 = ZmxizyVhfpSJgWkXcdAB98223965;     ZmxizyVhfpSJgWkXcdAB98223965 = ZmxizyVhfpSJgWkXcdAB35337675;     ZmxizyVhfpSJgWkXcdAB35337675 = ZmxizyVhfpSJgWkXcdAB27452182;     ZmxizyVhfpSJgWkXcdAB27452182 = ZmxizyVhfpSJgWkXcdAB22373444;     ZmxizyVhfpSJgWkXcdAB22373444 = ZmxizyVhfpSJgWkXcdAB28539122;     ZmxizyVhfpSJgWkXcdAB28539122 = ZmxizyVhfpSJgWkXcdAB46996785;     ZmxizyVhfpSJgWkXcdAB46996785 = ZmxizyVhfpSJgWkXcdAB69088003;     ZmxizyVhfpSJgWkXcdAB69088003 = ZmxizyVhfpSJgWkXcdAB68257203;     ZmxizyVhfpSJgWkXcdAB68257203 = ZmxizyVhfpSJgWkXcdAB14148029;     ZmxizyVhfpSJgWkXcdAB14148029 = ZmxizyVhfpSJgWkXcdAB53344061;     ZmxizyVhfpSJgWkXcdAB53344061 = ZmxizyVhfpSJgWkXcdAB8190975;     ZmxizyVhfpSJgWkXcdAB8190975 = ZmxizyVhfpSJgWkXcdAB89752236;     ZmxizyVhfpSJgWkXcdAB89752236 = ZmxizyVhfpSJgWkXcdAB68872225;     ZmxizyVhfpSJgWkXcdAB68872225 = ZmxizyVhfpSJgWkXcdAB96336150;     ZmxizyVhfpSJgWkXcdAB96336150 = ZmxizyVhfpSJgWkXcdAB49876874;     ZmxizyVhfpSJgWkXcdAB49876874 = ZmxizyVhfpSJgWkXcdAB79118324;     ZmxizyVhfpSJgWkXcdAB79118324 = ZmxizyVhfpSJgWkXcdAB63367924;     ZmxizyVhfpSJgWkXcdAB63367924 = ZmxizyVhfpSJgWkXcdAB8930261;     ZmxizyVhfpSJgWkXcdAB8930261 = ZmxizyVhfpSJgWkXcdAB60034248;     ZmxizyVhfpSJgWkXcdAB60034248 = ZmxizyVhfpSJgWkXcdAB93722340;     ZmxizyVhfpSJgWkXcdAB93722340 = ZmxizyVhfpSJgWkXcdAB41928878;     ZmxizyVhfpSJgWkXcdAB41928878 = ZmxizyVhfpSJgWkXcdAB22422415;     ZmxizyVhfpSJgWkXcdAB22422415 = ZmxizyVhfpSJgWkXcdAB5442363;     ZmxizyVhfpSJgWkXcdAB5442363 = ZmxizyVhfpSJgWkXcdAB36487981;     ZmxizyVhfpSJgWkXcdAB36487981 = ZmxizyVhfpSJgWkXcdAB52365923;     ZmxizyVhfpSJgWkXcdAB52365923 = ZmxizyVhfpSJgWkXcdAB53157977;     ZmxizyVhfpSJgWkXcdAB53157977 = ZmxizyVhfpSJgWkXcdAB64390173;     ZmxizyVhfpSJgWkXcdAB64390173 = ZmxizyVhfpSJgWkXcdAB27366337;     ZmxizyVhfpSJgWkXcdAB27366337 = ZmxizyVhfpSJgWkXcdAB77607363;     ZmxizyVhfpSJgWkXcdAB77607363 = ZmxizyVhfpSJgWkXcdAB21316900;     ZmxizyVhfpSJgWkXcdAB21316900 = ZmxizyVhfpSJgWkXcdAB15741252;     ZmxizyVhfpSJgWkXcdAB15741252 = ZmxizyVhfpSJgWkXcdAB30413854;     ZmxizyVhfpSJgWkXcdAB30413854 = ZmxizyVhfpSJgWkXcdAB45804788;     ZmxizyVhfpSJgWkXcdAB45804788 = ZmxizyVhfpSJgWkXcdAB39477446;     ZmxizyVhfpSJgWkXcdAB39477446 = ZmxizyVhfpSJgWkXcdAB13230335;     ZmxizyVhfpSJgWkXcdAB13230335 = ZmxizyVhfpSJgWkXcdAB18532683;     ZmxizyVhfpSJgWkXcdAB18532683 = ZmxizyVhfpSJgWkXcdAB16017546;     ZmxizyVhfpSJgWkXcdAB16017546 = ZmxizyVhfpSJgWkXcdAB41411735;     ZmxizyVhfpSJgWkXcdAB41411735 = ZmxizyVhfpSJgWkXcdAB84390591;     ZmxizyVhfpSJgWkXcdAB84390591 = ZmxizyVhfpSJgWkXcdAB71010515;     ZmxizyVhfpSJgWkXcdAB71010515 = ZmxizyVhfpSJgWkXcdAB23393904;     ZmxizyVhfpSJgWkXcdAB23393904 = ZmxizyVhfpSJgWkXcdAB57151197;     ZmxizyVhfpSJgWkXcdAB57151197 = ZmxizyVhfpSJgWkXcdAB67447919;     ZmxizyVhfpSJgWkXcdAB67447919 = ZmxizyVhfpSJgWkXcdAB86170506;     ZmxizyVhfpSJgWkXcdAB86170506 = ZmxizyVhfpSJgWkXcdAB74610618;     ZmxizyVhfpSJgWkXcdAB74610618 = ZmxizyVhfpSJgWkXcdAB88761035;     ZmxizyVhfpSJgWkXcdAB88761035 = ZmxizyVhfpSJgWkXcdAB91369694;     ZmxizyVhfpSJgWkXcdAB91369694 = ZmxizyVhfpSJgWkXcdAB49574741;     ZmxizyVhfpSJgWkXcdAB49574741 = ZmxizyVhfpSJgWkXcdAB24966313;     ZmxizyVhfpSJgWkXcdAB24966313 = ZmxizyVhfpSJgWkXcdAB92977672;     ZmxizyVhfpSJgWkXcdAB92977672 = ZmxizyVhfpSJgWkXcdAB26032477;     ZmxizyVhfpSJgWkXcdAB26032477 = ZmxizyVhfpSJgWkXcdAB94236287;     ZmxizyVhfpSJgWkXcdAB94236287 = ZmxizyVhfpSJgWkXcdAB5513178;     ZmxizyVhfpSJgWkXcdAB5513178 = ZmxizyVhfpSJgWkXcdAB84631289;     ZmxizyVhfpSJgWkXcdAB84631289 = ZmxizyVhfpSJgWkXcdAB16138713;     ZmxizyVhfpSJgWkXcdAB16138713 = ZmxizyVhfpSJgWkXcdAB75801550;     ZmxizyVhfpSJgWkXcdAB75801550 = ZmxizyVhfpSJgWkXcdAB29895312;     ZmxizyVhfpSJgWkXcdAB29895312 = ZmxizyVhfpSJgWkXcdAB90964200;     ZmxizyVhfpSJgWkXcdAB90964200 = ZmxizyVhfpSJgWkXcdAB70007521;     ZmxizyVhfpSJgWkXcdAB70007521 = ZmxizyVhfpSJgWkXcdAB75381144;     ZmxizyVhfpSJgWkXcdAB75381144 = ZmxizyVhfpSJgWkXcdAB82606611;     ZmxizyVhfpSJgWkXcdAB82606611 = ZmxizyVhfpSJgWkXcdAB41721666;     ZmxizyVhfpSJgWkXcdAB41721666 = ZmxizyVhfpSJgWkXcdAB90649839;     ZmxizyVhfpSJgWkXcdAB90649839 = ZmxizyVhfpSJgWkXcdAB92831129;     ZmxizyVhfpSJgWkXcdAB92831129 = ZmxizyVhfpSJgWkXcdAB37602810;     ZmxizyVhfpSJgWkXcdAB37602810 = ZmxizyVhfpSJgWkXcdAB77777121;     ZmxizyVhfpSJgWkXcdAB77777121 = ZmxizyVhfpSJgWkXcdAB43947449;     ZmxizyVhfpSJgWkXcdAB43947449 = ZmxizyVhfpSJgWkXcdAB29394780;     ZmxizyVhfpSJgWkXcdAB29394780 = ZmxizyVhfpSJgWkXcdAB19289574;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void ilRPJpCyrJvUwVHFOKxu2867615() {     double rXxiYTvxFjxmMKPOJjFk12607571 = -907745511;    double rXxiYTvxFjxmMKPOJjFk64626219 = -279669910;    double rXxiYTvxFjxmMKPOJjFk14315706 = 37377800;    double rXxiYTvxFjxmMKPOJjFk99594742 = -766109932;    double rXxiYTvxFjxmMKPOJjFk95274111 = -595607687;    double rXxiYTvxFjxmMKPOJjFk67421356 = 92822518;    double rXxiYTvxFjxmMKPOJjFk87093179 = -328753039;    double rXxiYTvxFjxmMKPOJjFk80376679 = -418155803;    double rXxiYTvxFjxmMKPOJjFk30679293 = -728847260;    double rXxiYTvxFjxmMKPOJjFk52448400 = -586511172;    double rXxiYTvxFjxmMKPOJjFk70809155 = -1044723;    double rXxiYTvxFjxmMKPOJjFk39910192 = -808376344;    double rXxiYTvxFjxmMKPOJjFk33642001 = -899754270;    double rXxiYTvxFjxmMKPOJjFk84677765 = -627159718;    double rXxiYTvxFjxmMKPOJjFk26565271 = -582501775;    double rXxiYTvxFjxmMKPOJjFk75139133 = -156772849;    double rXxiYTvxFjxmMKPOJjFk67797005 = -543871131;    double rXxiYTvxFjxmMKPOJjFk51977640 = -567900984;    double rXxiYTvxFjxmMKPOJjFk26934151 = -258356994;    double rXxiYTvxFjxmMKPOJjFk3639142 = -208799196;    double rXxiYTvxFjxmMKPOJjFk7232553 = -628377877;    double rXxiYTvxFjxmMKPOJjFk96867329 = -246338508;    double rXxiYTvxFjxmMKPOJjFk49100561 = -758418205;    double rXxiYTvxFjxmMKPOJjFk87027380 = -348214231;    double rXxiYTvxFjxmMKPOJjFk40601662 = -130695464;    double rXxiYTvxFjxmMKPOJjFk51394370 = -905532068;    double rXxiYTvxFjxmMKPOJjFk99583401 = -996112405;    double rXxiYTvxFjxmMKPOJjFk5961651 = -111670216;    double rXxiYTvxFjxmMKPOJjFk78038644 = -468791168;    double rXxiYTvxFjxmMKPOJjFk33591687 = -817662774;    double rXxiYTvxFjxmMKPOJjFk92985540 = -876845989;    double rXxiYTvxFjxmMKPOJjFk21807104 = -449202198;    double rXxiYTvxFjxmMKPOJjFk32793811 = -927697565;    double rXxiYTvxFjxmMKPOJjFk53657541 = 81573055;    double rXxiYTvxFjxmMKPOJjFk70582377 = -816170890;    double rXxiYTvxFjxmMKPOJjFk81531621 = -555151167;    double rXxiYTvxFjxmMKPOJjFk96919731 = 65612550;    double rXxiYTvxFjxmMKPOJjFk63218334 = -797572118;    double rXxiYTvxFjxmMKPOJjFk7550064 = -710772950;    double rXxiYTvxFjxmMKPOJjFk73240197 = -666501862;    double rXxiYTvxFjxmMKPOJjFk34097280 = -708356318;    double rXxiYTvxFjxmMKPOJjFk49187384 = -314443756;    double rXxiYTvxFjxmMKPOJjFk71941203 = -846534043;    double rXxiYTvxFjxmMKPOJjFk50995236 = -775241564;    double rXxiYTvxFjxmMKPOJjFk68852304 = -295691783;    double rXxiYTvxFjxmMKPOJjFk15503153 = -465558057;    double rXxiYTvxFjxmMKPOJjFk45469014 = -334300249;    double rXxiYTvxFjxmMKPOJjFk8402461 = -226362496;    double rXxiYTvxFjxmMKPOJjFk66296382 = -717124671;    double rXxiYTvxFjxmMKPOJjFk14982339 = -683802489;    double rXxiYTvxFjxmMKPOJjFk3926173 = -873505614;    double rXxiYTvxFjxmMKPOJjFk5450028 = -916756174;    double rXxiYTvxFjxmMKPOJjFk6588882 = -918253431;    double rXxiYTvxFjxmMKPOJjFk94192111 = -784344660;    double rXxiYTvxFjxmMKPOJjFk95635062 = -53269340;    double rXxiYTvxFjxmMKPOJjFk15740242 = -561407004;    double rXxiYTvxFjxmMKPOJjFk15525658 = -521251706;    double rXxiYTvxFjxmMKPOJjFk27288325 = -614407970;    double rXxiYTvxFjxmMKPOJjFk58993080 = -535414468;    double rXxiYTvxFjxmMKPOJjFk43879741 = -690075620;    double rXxiYTvxFjxmMKPOJjFk67837955 = 88934922;    double rXxiYTvxFjxmMKPOJjFk81131529 = -117082823;    double rXxiYTvxFjxmMKPOJjFk2338035 = -949364635;    double rXxiYTvxFjxmMKPOJjFk97087605 = -911184487;    double rXxiYTvxFjxmMKPOJjFk59462859 = -709665184;    double rXxiYTvxFjxmMKPOJjFk49002051 = -551842525;    double rXxiYTvxFjxmMKPOJjFk7116381 = -880678779;    double rXxiYTvxFjxmMKPOJjFk79984460 = -881327325;    double rXxiYTvxFjxmMKPOJjFk14095388 = -810988828;    double rXxiYTvxFjxmMKPOJjFk45033649 = 72649391;    double rXxiYTvxFjxmMKPOJjFk78219401 = -122385400;    double rXxiYTvxFjxmMKPOJjFk4578672 = -746299013;    double rXxiYTvxFjxmMKPOJjFk44427576 = -857128035;    double rXxiYTvxFjxmMKPOJjFk53693954 = -591855132;    double rXxiYTvxFjxmMKPOJjFk69541862 = -500442878;    double rXxiYTvxFjxmMKPOJjFk58045168 = -213934122;    double rXxiYTvxFjxmMKPOJjFk24926126 = -399804465;    double rXxiYTvxFjxmMKPOJjFk98105325 = -983176641;    double rXxiYTvxFjxmMKPOJjFk18175077 = 47477552;    double rXxiYTvxFjxmMKPOJjFk25098509 = -665137407;    double rXxiYTvxFjxmMKPOJjFk5925357 = -471231819;    double rXxiYTvxFjxmMKPOJjFk91180941 = -669749910;    double rXxiYTvxFjxmMKPOJjFk39665268 = -394545546;    double rXxiYTvxFjxmMKPOJjFk63056306 = -784988680;    double rXxiYTvxFjxmMKPOJjFk29665514 = -944157160;    double rXxiYTvxFjxmMKPOJjFk87535513 = -960089815;    double rXxiYTvxFjxmMKPOJjFk15218222 = -530948767;    double rXxiYTvxFjxmMKPOJjFk38601700 = -43352906;    double rXxiYTvxFjxmMKPOJjFk58022478 = -865157606;    double rXxiYTvxFjxmMKPOJjFk54842136 = -154763887;    double rXxiYTvxFjxmMKPOJjFk66005963 = 66100539;    double rXxiYTvxFjxmMKPOJjFk69631407 = -319979480;    double rXxiYTvxFjxmMKPOJjFk4225254 = -162157650;    double rXxiYTvxFjxmMKPOJjFk63670323 = 79302669;    double rXxiYTvxFjxmMKPOJjFk5402243 = -655436785;    double rXxiYTvxFjxmMKPOJjFk52965751 = -491273496;    double rXxiYTvxFjxmMKPOJjFk46849349 = -365079121;    double rXxiYTvxFjxmMKPOJjFk74853598 = -935349557;    double rXxiYTvxFjxmMKPOJjFk91532376 = 34423620;    double rXxiYTvxFjxmMKPOJjFk19850253 = -907745511;     rXxiYTvxFjxmMKPOJjFk12607571 = rXxiYTvxFjxmMKPOJjFk64626219;     rXxiYTvxFjxmMKPOJjFk64626219 = rXxiYTvxFjxmMKPOJjFk14315706;     rXxiYTvxFjxmMKPOJjFk14315706 = rXxiYTvxFjxmMKPOJjFk99594742;     rXxiYTvxFjxmMKPOJjFk99594742 = rXxiYTvxFjxmMKPOJjFk95274111;     rXxiYTvxFjxmMKPOJjFk95274111 = rXxiYTvxFjxmMKPOJjFk67421356;     rXxiYTvxFjxmMKPOJjFk67421356 = rXxiYTvxFjxmMKPOJjFk87093179;     rXxiYTvxFjxmMKPOJjFk87093179 = rXxiYTvxFjxmMKPOJjFk80376679;     rXxiYTvxFjxmMKPOJjFk80376679 = rXxiYTvxFjxmMKPOJjFk30679293;     rXxiYTvxFjxmMKPOJjFk30679293 = rXxiYTvxFjxmMKPOJjFk52448400;     rXxiYTvxFjxmMKPOJjFk52448400 = rXxiYTvxFjxmMKPOJjFk70809155;     rXxiYTvxFjxmMKPOJjFk70809155 = rXxiYTvxFjxmMKPOJjFk39910192;     rXxiYTvxFjxmMKPOJjFk39910192 = rXxiYTvxFjxmMKPOJjFk33642001;     rXxiYTvxFjxmMKPOJjFk33642001 = rXxiYTvxFjxmMKPOJjFk84677765;     rXxiYTvxFjxmMKPOJjFk84677765 = rXxiYTvxFjxmMKPOJjFk26565271;     rXxiYTvxFjxmMKPOJjFk26565271 = rXxiYTvxFjxmMKPOJjFk75139133;     rXxiYTvxFjxmMKPOJjFk75139133 = rXxiYTvxFjxmMKPOJjFk67797005;     rXxiYTvxFjxmMKPOJjFk67797005 = rXxiYTvxFjxmMKPOJjFk51977640;     rXxiYTvxFjxmMKPOJjFk51977640 = rXxiYTvxFjxmMKPOJjFk26934151;     rXxiYTvxFjxmMKPOJjFk26934151 = rXxiYTvxFjxmMKPOJjFk3639142;     rXxiYTvxFjxmMKPOJjFk3639142 = rXxiYTvxFjxmMKPOJjFk7232553;     rXxiYTvxFjxmMKPOJjFk7232553 = rXxiYTvxFjxmMKPOJjFk96867329;     rXxiYTvxFjxmMKPOJjFk96867329 = rXxiYTvxFjxmMKPOJjFk49100561;     rXxiYTvxFjxmMKPOJjFk49100561 = rXxiYTvxFjxmMKPOJjFk87027380;     rXxiYTvxFjxmMKPOJjFk87027380 = rXxiYTvxFjxmMKPOJjFk40601662;     rXxiYTvxFjxmMKPOJjFk40601662 = rXxiYTvxFjxmMKPOJjFk51394370;     rXxiYTvxFjxmMKPOJjFk51394370 = rXxiYTvxFjxmMKPOJjFk99583401;     rXxiYTvxFjxmMKPOJjFk99583401 = rXxiYTvxFjxmMKPOJjFk5961651;     rXxiYTvxFjxmMKPOJjFk5961651 = rXxiYTvxFjxmMKPOJjFk78038644;     rXxiYTvxFjxmMKPOJjFk78038644 = rXxiYTvxFjxmMKPOJjFk33591687;     rXxiYTvxFjxmMKPOJjFk33591687 = rXxiYTvxFjxmMKPOJjFk92985540;     rXxiYTvxFjxmMKPOJjFk92985540 = rXxiYTvxFjxmMKPOJjFk21807104;     rXxiYTvxFjxmMKPOJjFk21807104 = rXxiYTvxFjxmMKPOJjFk32793811;     rXxiYTvxFjxmMKPOJjFk32793811 = rXxiYTvxFjxmMKPOJjFk53657541;     rXxiYTvxFjxmMKPOJjFk53657541 = rXxiYTvxFjxmMKPOJjFk70582377;     rXxiYTvxFjxmMKPOJjFk70582377 = rXxiYTvxFjxmMKPOJjFk81531621;     rXxiYTvxFjxmMKPOJjFk81531621 = rXxiYTvxFjxmMKPOJjFk96919731;     rXxiYTvxFjxmMKPOJjFk96919731 = rXxiYTvxFjxmMKPOJjFk63218334;     rXxiYTvxFjxmMKPOJjFk63218334 = rXxiYTvxFjxmMKPOJjFk7550064;     rXxiYTvxFjxmMKPOJjFk7550064 = rXxiYTvxFjxmMKPOJjFk73240197;     rXxiYTvxFjxmMKPOJjFk73240197 = rXxiYTvxFjxmMKPOJjFk34097280;     rXxiYTvxFjxmMKPOJjFk34097280 = rXxiYTvxFjxmMKPOJjFk49187384;     rXxiYTvxFjxmMKPOJjFk49187384 = rXxiYTvxFjxmMKPOJjFk71941203;     rXxiYTvxFjxmMKPOJjFk71941203 = rXxiYTvxFjxmMKPOJjFk50995236;     rXxiYTvxFjxmMKPOJjFk50995236 = rXxiYTvxFjxmMKPOJjFk68852304;     rXxiYTvxFjxmMKPOJjFk68852304 = rXxiYTvxFjxmMKPOJjFk15503153;     rXxiYTvxFjxmMKPOJjFk15503153 = rXxiYTvxFjxmMKPOJjFk45469014;     rXxiYTvxFjxmMKPOJjFk45469014 = rXxiYTvxFjxmMKPOJjFk8402461;     rXxiYTvxFjxmMKPOJjFk8402461 = rXxiYTvxFjxmMKPOJjFk66296382;     rXxiYTvxFjxmMKPOJjFk66296382 = rXxiYTvxFjxmMKPOJjFk14982339;     rXxiYTvxFjxmMKPOJjFk14982339 = rXxiYTvxFjxmMKPOJjFk3926173;     rXxiYTvxFjxmMKPOJjFk3926173 = rXxiYTvxFjxmMKPOJjFk5450028;     rXxiYTvxFjxmMKPOJjFk5450028 = rXxiYTvxFjxmMKPOJjFk6588882;     rXxiYTvxFjxmMKPOJjFk6588882 = rXxiYTvxFjxmMKPOJjFk94192111;     rXxiYTvxFjxmMKPOJjFk94192111 = rXxiYTvxFjxmMKPOJjFk95635062;     rXxiYTvxFjxmMKPOJjFk95635062 = rXxiYTvxFjxmMKPOJjFk15740242;     rXxiYTvxFjxmMKPOJjFk15740242 = rXxiYTvxFjxmMKPOJjFk15525658;     rXxiYTvxFjxmMKPOJjFk15525658 = rXxiYTvxFjxmMKPOJjFk27288325;     rXxiYTvxFjxmMKPOJjFk27288325 = rXxiYTvxFjxmMKPOJjFk58993080;     rXxiYTvxFjxmMKPOJjFk58993080 = rXxiYTvxFjxmMKPOJjFk43879741;     rXxiYTvxFjxmMKPOJjFk43879741 = rXxiYTvxFjxmMKPOJjFk67837955;     rXxiYTvxFjxmMKPOJjFk67837955 = rXxiYTvxFjxmMKPOJjFk81131529;     rXxiYTvxFjxmMKPOJjFk81131529 = rXxiYTvxFjxmMKPOJjFk2338035;     rXxiYTvxFjxmMKPOJjFk2338035 = rXxiYTvxFjxmMKPOJjFk97087605;     rXxiYTvxFjxmMKPOJjFk97087605 = rXxiYTvxFjxmMKPOJjFk59462859;     rXxiYTvxFjxmMKPOJjFk59462859 = rXxiYTvxFjxmMKPOJjFk49002051;     rXxiYTvxFjxmMKPOJjFk49002051 = rXxiYTvxFjxmMKPOJjFk7116381;     rXxiYTvxFjxmMKPOJjFk7116381 = rXxiYTvxFjxmMKPOJjFk79984460;     rXxiYTvxFjxmMKPOJjFk79984460 = rXxiYTvxFjxmMKPOJjFk14095388;     rXxiYTvxFjxmMKPOJjFk14095388 = rXxiYTvxFjxmMKPOJjFk45033649;     rXxiYTvxFjxmMKPOJjFk45033649 = rXxiYTvxFjxmMKPOJjFk78219401;     rXxiYTvxFjxmMKPOJjFk78219401 = rXxiYTvxFjxmMKPOJjFk4578672;     rXxiYTvxFjxmMKPOJjFk4578672 = rXxiYTvxFjxmMKPOJjFk44427576;     rXxiYTvxFjxmMKPOJjFk44427576 = rXxiYTvxFjxmMKPOJjFk53693954;     rXxiYTvxFjxmMKPOJjFk53693954 = rXxiYTvxFjxmMKPOJjFk69541862;     rXxiYTvxFjxmMKPOJjFk69541862 = rXxiYTvxFjxmMKPOJjFk58045168;     rXxiYTvxFjxmMKPOJjFk58045168 = rXxiYTvxFjxmMKPOJjFk24926126;     rXxiYTvxFjxmMKPOJjFk24926126 = rXxiYTvxFjxmMKPOJjFk98105325;     rXxiYTvxFjxmMKPOJjFk98105325 = rXxiYTvxFjxmMKPOJjFk18175077;     rXxiYTvxFjxmMKPOJjFk18175077 = rXxiYTvxFjxmMKPOJjFk25098509;     rXxiYTvxFjxmMKPOJjFk25098509 = rXxiYTvxFjxmMKPOJjFk5925357;     rXxiYTvxFjxmMKPOJjFk5925357 = rXxiYTvxFjxmMKPOJjFk91180941;     rXxiYTvxFjxmMKPOJjFk91180941 = rXxiYTvxFjxmMKPOJjFk39665268;     rXxiYTvxFjxmMKPOJjFk39665268 = rXxiYTvxFjxmMKPOJjFk63056306;     rXxiYTvxFjxmMKPOJjFk63056306 = rXxiYTvxFjxmMKPOJjFk29665514;     rXxiYTvxFjxmMKPOJjFk29665514 = rXxiYTvxFjxmMKPOJjFk87535513;     rXxiYTvxFjxmMKPOJjFk87535513 = rXxiYTvxFjxmMKPOJjFk15218222;     rXxiYTvxFjxmMKPOJjFk15218222 = rXxiYTvxFjxmMKPOJjFk38601700;     rXxiYTvxFjxmMKPOJjFk38601700 = rXxiYTvxFjxmMKPOJjFk58022478;     rXxiYTvxFjxmMKPOJjFk58022478 = rXxiYTvxFjxmMKPOJjFk54842136;     rXxiYTvxFjxmMKPOJjFk54842136 = rXxiYTvxFjxmMKPOJjFk66005963;     rXxiYTvxFjxmMKPOJjFk66005963 = rXxiYTvxFjxmMKPOJjFk69631407;     rXxiYTvxFjxmMKPOJjFk69631407 = rXxiYTvxFjxmMKPOJjFk4225254;     rXxiYTvxFjxmMKPOJjFk4225254 = rXxiYTvxFjxmMKPOJjFk63670323;     rXxiYTvxFjxmMKPOJjFk63670323 = rXxiYTvxFjxmMKPOJjFk5402243;     rXxiYTvxFjxmMKPOJjFk5402243 = rXxiYTvxFjxmMKPOJjFk52965751;     rXxiYTvxFjxmMKPOJjFk52965751 = rXxiYTvxFjxmMKPOJjFk46849349;     rXxiYTvxFjxmMKPOJjFk46849349 = rXxiYTvxFjxmMKPOJjFk74853598;     rXxiYTvxFjxmMKPOJjFk74853598 = rXxiYTvxFjxmMKPOJjFk91532376;     rXxiYTvxFjxmMKPOJjFk91532376 = rXxiYTvxFjxmMKPOJjFk19850253;     rXxiYTvxFjxmMKPOJjFk19850253 = rXxiYTvxFjxmMKPOJjFk12607571;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void tvFjDHutzECgeLrMEabT30461871() {     double aTWKdwbeLOUvMxWAyunW38192848 = -332778611;    double aTWKdwbeLOUvMxWAyunW61023363 = -535147687;    double aTWKdwbeLOUvMxWAyunW47194881 = -865083736;    double aTWKdwbeLOUvMxWAyunW45316672 = -773432825;    double aTWKdwbeLOUvMxWAyunW64616116 = -642529974;    double aTWKdwbeLOUvMxWAyunW84430633 = -62158485;    double aTWKdwbeLOUvMxWAyunW92414024 = -911024246;    double aTWKdwbeLOUvMxWAyunW95386803 = -798837682;    double aTWKdwbeLOUvMxWAyunW56008585 = -125071305;    double aTWKdwbeLOUvMxWAyunW71606782 = -921072696;    double aTWKdwbeLOUvMxWAyunW85142708 = -311472609;    double aTWKdwbeLOUvMxWAyunW75227848 = -152085251;    double aTWKdwbeLOUvMxWAyunW10222779 = -664002636;    double aTWKdwbeLOUvMxWAyunW76237482 = -601684011;    double aTWKdwbeLOUvMxWAyunW82834158 = -471047733;    double aTWKdwbeLOUvMxWAyunW82860176 = -592964326;    double aTWKdwbeLOUvMxWAyunW67595121 = -232190925;    double aTWKdwbeLOUvMxWAyunW73093544 = 15821655;    double aTWKdwbeLOUvMxWAyunW88398872 = 58669149;    double aTWKdwbeLOUvMxWAyunW64800608 = -183238571;    double aTWKdwbeLOUvMxWAyunW8241818 = 9922341;    double aTWKdwbeLOUvMxWAyunW98370435 = -666071437;    double aTWKdwbeLOUvMxWAyunW52864359 = -802510310;    double aTWKdwbeLOUvMxWAyunW88741423 = -207063210;    double aTWKdwbeLOUvMxWAyunW95504801 = -798191606;    double aTWKdwbeLOUvMxWAyunW80883507 = -469982609;    double aTWKdwbeLOUvMxWAyunW81971310 = -561925540;    double aTWKdwbeLOUvMxWAyunW69860450 = -341332449;    double aTWKdwbeLOUvMxWAyunW85915857 = -705085018;    double aTWKdwbeLOUvMxWAyunW86914178 = -726572336;    double aTWKdwbeLOUvMxWAyunW15032175 = -655493019;    double aTWKdwbeLOUvMxWAyunW58997036 = -722998032;    double aTWKdwbeLOUvMxWAyunW53980863 = -275518702;    double aTWKdwbeLOUvMxWAyunW7244612 = 25428083;    double aTWKdwbeLOUvMxWAyunW71987365 = -417606801;    double aTWKdwbeLOUvMxWAyunW86323472 = 27254117;    double aTWKdwbeLOUvMxWAyunW46565117 = -414899394;    double aTWKdwbeLOUvMxWAyunW38791021 = -646286944;    double aTWKdwbeLOUvMxWAyunW48473643 = -69111272;    double aTWKdwbeLOUvMxWAyunW82023870 = -36472181;    double aTWKdwbeLOUvMxWAyunW81248560 = -183416508;    double aTWKdwbeLOUvMxWAyunW93196973 = -823482260;    double aTWKdwbeLOUvMxWAyunW21141423 = -875140425;    double aTWKdwbeLOUvMxWAyunW21788515 = -143657163;    double aTWKdwbeLOUvMxWAyunW60833181 = -234712900;    double aTWKdwbeLOUvMxWAyunW81903475 = -988105595;    double aTWKdwbeLOUvMxWAyunW45237468 = -366769109;    double aTWKdwbeLOUvMxWAyunW16515232 = -419545342;    double aTWKdwbeLOUvMxWAyunW17107703 = -816800897;    double aTWKdwbeLOUvMxWAyunW68792334 = -531825642;    double aTWKdwbeLOUvMxWAyunW14062421 = -22164216;    double aTWKdwbeLOUvMxWAyunW32708607 = 5164646;    double aTWKdwbeLOUvMxWAyunW24662485 = -386151980;    double aTWKdwbeLOUvMxWAyunW63091020 = 591559;    double aTWKdwbeLOUvMxWAyunW22220054 = -528359757;    double aTWKdwbeLOUvMxWAyunW39822412 = -666707175;    double aTWKdwbeLOUvMxWAyunW8159004 = -732637377;    double aTWKdwbeLOUvMxWAyunW58453458 = -558020526;    double aTWKdwbeLOUvMxWAyunW49811871 = -975241219;    double aTWKdwbeLOUvMxWAyunW83732609 = -72547365;    double aTWKdwbeLOUvMxWAyunW2459323 = -500232946;    double aTWKdwbeLOUvMxWAyunW22553574 = -469691797;    double aTWKdwbeLOUvMxWAyunW9470947 = 6247336;    double aTWKdwbeLOUvMxWAyunW69094407 = -398498970;    double aTWKdwbeLOUvMxWAyunW56574607 = -165579677;    double aTWKdwbeLOUvMxWAyunW26145673 = -588474578;    double aTWKdwbeLOUvMxWAyunW21246985 = -876566549;    double aTWKdwbeLOUvMxWAyunW2978167 = -589430719;    double aTWKdwbeLOUvMxWAyunW4250117 = -84077210;    double aTWKdwbeLOUvMxWAyunW96510686 = -398301851;    double aTWKdwbeLOUvMxWAyunW36295059 = -78064932;    double aTWKdwbeLOUvMxWAyunW28804101 = -585903982;    double aTWKdwbeLOUvMxWAyunW24619902 = -915067074;    double aTWKdwbeLOUvMxWAyunW6375002 = -904858670;    double aTWKdwbeLOUvMxWAyunW83552047 = -999822064;    double aTWKdwbeLOUvMxWAyunW15044844 = -166595399;    double aTWKdwbeLOUvMxWAyunW77229012 = -790931013;    double aTWKdwbeLOUvMxWAyunW31075844 = -558853148;    double aTWKdwbeLOUvMxWAyunW27908243 = -972350310;    double aTWKdwbeLOUvMxWAyunW13601327 = -810086012;    double aTWKdwbeLOUvMxWAyunW35646039 = -3213501;    double aTWKdwbeLOUvMxWAyunW65456079 = -42380198;    double aTWKdwbeLOUvMxWAyunW52752748 = -524531552;    double aTWKdwbeLOUvMxWAyunW17123524 = -73259377;    double aTWKdwbeLOUvMxWAyunW72851757 = -604408120;    double aTWKdwbeLOUvMxWAyunW82323568 = -560657666;    double aTWKdwbeLOUvMxWAyunW34334551 = -236846053;    double aTWKdwbeLOUvMxWAyunW90889842 = -176110261;    double aTWKdwbeLOUvMxWAyunW85024558 = -446212160;    double aTWKdwbeLOUvMxWAyunW32164954 = -750899627;    double aTWKdwbeLOUvMxWAyunW78164468 = -240108506;    double aTWKdwbeLOUvMxWAyunW88111659 = -856878869;    double aTWKdwbeLOUvMxWAyunW88979149 = -671045725;    double aTWKdwbeLOUvMxWAyunW64741034 = -996563907;    double aTWKdwbeLOUvMxWAyunW79564548 = -536239236;    double aTWKdwbeLOUvMxWAyunW58694987 = -713724711;    double aTWKdwbeLOUvMxWAyunW83726027 = -729729596;    double aTWKdwbeLOUvMxWAyunW52047015 = -376641456;    double aTWKdwbeLOUvMxWAyunW65213908 = -978077487;    double aTWKdwbeLOUvMxWAyunW34687508 = -332778611;     aTWKdwbeLOUvMxWAyunW38192848 = aTWKdwbeLOUvMxWAyunW61023363;     aTWKdwbeLOUvMxWAyunW61023363 = aTWKdwbeLOUvMxWAyunW47194881;     aTWKdwbeLOUvMxWAyunW47194881 = aTWKdwbeLOUvMxWAyunW45316672;     aTWKdwbeLOUvMxWAyunW45316672 = aTWKdwbeLOUvMxWAyunW64616116;     aTWKdwbeLOUvMxWAyunW64616116 = aTWKdwbeLOUvMxWAyunW84430633;     aTWKdwbeLOUvMxWAyunW84430633 = aTWKdwbeLOUvMxWAyunW92414024;     aTWKdwbeLOUvMxWAyunW92414024 = aTWKdwbeLOUvMxWAyunW95386803;     aTWKdwbeLOUvMxWAyunW95386803 = aTWKdwbeLOUvMxWAyunW56008585;     aTWKdwbeLOUvMxWAyunW56008585 = aTWKdwbeLOUvMxWAyunW71606782;     aTWKdwbeLOUvMxWAyunW71606782 = aTWKdwbeLOUvMxWAyunW85142708;     aTWKdwbeLOUvMxWAyunW85142708 = aTWKdwbeLOUvMxWAyunW75227848;     aTWKdwbeLOUvMxWAyunW75227848 = aTWKdwbeLOUvMxWAyunW10222779;     aTWKdwbeLOUvMxWAyunW10222779 = aTWKdwbeLOUvMxWAyunW76237482;     aTWKdwbeLOUvMxWAyunW76237482 = aTWKdwbeLOUvMxWAyunW82834158;     aTWKdwbeLOUvMxWAyunW82834158 = aTWKdwbeLOUvMxWAyunW82860176;     aTWKdwbeLOUvMxWAyunW82860176 = aTWKdwbeLOUvMxWAyunW67595121;     aTWKdwbeLOUvMxWAyunW67595121 = aTWKdwbeLOUvMxWAyunW73093544;     aTWKdwbeLOUvMxWAyunW73093544 = aTWKdwbeLOUvMxWAyunW88398872;     aTWKdwbeLOUvMxWAyunW88398872 = aTWKdwbeLOUvMxWAyunW64800608;     aTWKdwbeLOUvMxWAyunW64800608 = aTWKdwbeLOUvMxWAyunW8241818;     aTWKdwbeLOUvMxWAyunW8241818 = aTWKdwbeLOUvMxWAyunW98370435;     aTWKdwbeLOUvMxWAyunW98370435 = aTWKdwbeLOUvMxWAyunW52864359;     aTWKdwbeLOUvMxWAyunW52864359 = aTWKdwbeLOUvMxWAyunW88741423;     aTWKdwbeLOUvMxWAyunW88741423 = aTWKdwbeLOUvMxWAyunW95504801;     aTWKdwbeLOUvMxWAyunW95504801 = aTWKdwbeLOUvMxWAyunW80883507;     aTWKdwbeLOUvMxWAyunW80883507 = aTWKdwbeLOUvMxWAyunW81971310;     aTWKdwbeLOUvMxWAyunW81971310 = aTWKdwbeLOUvMxWAyunW69860450;     aTWKdwbeLOUvMxWAyunW69860450 = aTWKdwbeLOUvMxWAyunW85915857;     aTWKdwbeLOUvMxWAyunW85915857 = aTWKdwbeLOUvMxWAyunW86914178;     aTWKdwbeLOUvMxWAyunW86914178 = aTWKdwbeLOUvMxWAyunW15032175;     aTWKdwbeLOUvMxWAyunW15032175 = aTWKdwbeLOUvMxWAyunW58997036;     aTWKdwbeLOUvMxWAyunW58997036 = aTWKdwbeLOUvMxWAyunW53980863;     aTWKdwbeLOUvMxWAyunW53980863 = aTWKdwbeLOUvMxWAyunW7244612;     aTWKdwbeLOUvMxWAyunW7244612 = aTWKdwbeLOUvMxWAyunW71987365;     aTWKdwbeLOUvMxWAyunW71987365 = aTWKdwbeLOUvMxWAyunW86323472;     aTWKdwbeLOUvMxWAyunW86323472 = aTWKdwbeLOUvMxWAyunW46565117;     aTWKdwbeLOUvMxWAyunW46565117 = aTWKdwbeLOUvMxWAyunW38791021;     aTWKdwbeLOUvMxWAyunW38791021 = aTWKdwbeLOUvMxWAyunW48473643;     aTWKdwbeLOUvMxWAyunW48473643 = aTWKdwbeLOUvMxWAyunW82023870;     aTWKdwbeLOUvMxWAyunW82023870 = aTWKdwbeLOUvMxWAyunW81248560;     aTWKdwbeLOUvMxWAyunW81248560 = aTWKdwbeLOUvMxWAyunW93196973;     aTWKdwbeLOUvMxWAyunW93196973 = aTWKdwbeLOUvMxWAyunW21141423;     aTWKdwbeLOUvMxWAyunW21141423 = aTWKdwbeLOUvMxWAyunW21788515;     aTWKdwbeLOUvMxWAyunW21788515 = aTWKdwbeLOUvMxWAyunW60833181;     aTWKdwbeLOUvMxWAyunW60833181 = aTWKdwbeLOUvMxWAyunW81903475;     aTWKdwbeLOUvMxWAyunW81903475 = aTWKdwbeLOUvMxWAyunW45237468;     aTWKdwbeLOUvMxWAyunW45237468 = aTWKdwbeLOUvMxWAyunW16515232;     aTWKdwbeLOUvMxWAyunW16515232 = aTWKdwbeLOUvMxWAyunW17107703;     aTWKdwbeLOUvMxWAyunW17107703 = aTWKdwbeLOUvMxWAyunW68792334;     aTWKdwbeLOUvMxWAyunW68792334 = aTWKdwbeLOUvMxWAyunW14062421;     aTWKdwbeLOUvMxWAyunW14062421 = aTWKdwbeLOUvMxWAyunW32708607;     aTWKdwbeLOUvMxWAyunW32708607 = aTWKdwbeLOUvMxWAyunW24662485;     aTWKdwbeLOUvMxWAyunW24662485 = aTWKdwbeLOUvMxWAyunW63091020;     aTWKdwbeLOUvMxWAyunW63091020 = aTWKdwbeLOUvMxWAyunW22220054;     aTWKdwbeLOUvMxWAyunW22220054 = aTWKdwbeLOUvMxWAyunW39822412;     aTWKdwbeLOUvMxWAyunW39822412 = aTWKdwbeLOUvMxWAyunW8159004;     aTWKdwbeLOUvMxWAyunW8159004 = aTWKdwbeLOUvMxWAyunW58453458;     aTWKdwbeLOUvMxWAyunW58453458 = aTWKdwbeLOUvMxWAyunW49811871;     aTWKdwbeLOUvMxWAyunW49811871 = aTWKdwbeLOUvMxWAyunW83732609;     aTWKdwbeLOUvMxWAyunW83732609 = aTWKdwbeLOUvMxWAyunW2459323;     aTWKdwbeLOUvMxWAyunW2459323 = aTWKdwbeLOUvMxWAyunW22553574;     aTWKdwbeLOUvMxWAyunW22553574 = aTWKdwbeLOUvMxWAyunW9470947;     aTWKdwbeLOUvMxWAyunW9470947 = aTWKdwbeLOUvMxWAyunW69094407;     aTWKdwbeLOUvMxWAyunW69094407 = aTWKdwbeLOUvMxWAyunW56574607;     aTWKdwbeLOUvMxWAyunW56574607 = aTWKdwbeLOUvMxWAyunW26145673;     aTWKdwbeLOUvMxWAyunW26145673 = aTWKdwbeLOUvMxWAyunW21246985;     aTWKdwbeLOUvMxWAyunW21246985 = aTWKdwbeLOUvMxWAyunW2978167;     aTWKdwbeLOUvMxWAyunW2978167 = aTWKdwbeLOUvMxWAyunW4250117;     aTWKdwbeLOUvMxWAyunW4250117 = aTWKdwbeLOUvMxWAyunW96510686;     aTWKdwbeLOUvMxWAyunW96510686 = aTWKdwbeLOUvMxWAyunW36295059;     aTWKdwbeLOUvMxWAyunW36295059 = aTWKdwbeLOUvMxWAyunW28804101;     aTWKdwbeLOUvMxWAyunW28804101 = aTWKdwbeLOUvMxWAyunW24619902;     aTWKdwbeLOUvMxWAyunW24619902 = aTWKdwbeLOUvMxWAyunW6375002;     aTWKdwbeLOUvMxWAyunW6375002 = aTWKdwbeLOUvMxWAyunW83552047;     aTWKdwbeLOUvMxWAyunW83552047 = aTWKdwbeLOUvMxWAyunW15044844;     aTWKdwbeLOUvMxWAyunW15044844 = aTWKdwbeLOUvMxWAyunW77229012;     aTWKdwbeLOUvMxWAyunW77229012 = aTWKdwbeLOUvMxWAyunW31075844;     aTWKdwbeLOUvMxWAyunW31075844 = aTWKdwbeLOUvMxWAyunW27908243;     aTWKdwbeLOUvMxWAyunW27908243 = aTWKdwbeLOUvMxWAyunW13601327;     aTWKdwbeLOUvMxWAyunW13601327 = aTWKdwbeLOUvMxWAyunW35646039;     aTWKdwbeLOUvMxWAyunW35646039 = aTWKdwbeLOUvMxWAyunW65456079;     aTWKdwbeLOUvMxWAyunW65456079 = aTWKdwbeLOUvMxWAyunW52752748;     aTWKdwbeLOUvMxWAyunW52752748 = aTWKdwbeLOUvMxWAyunW17123524;     aTWKdwbeLOUvMxWAyunW17123524 = aTWKdwbeLOUvMxWAyunW72851757;     aTWKdwbeLOUvMxWAyunW72851757 = aTWKdwbeLOUvMxWAyunW82323568;     aTWKdwbeLOUvMxWAyunW82323568 = aTWKdwbeLOUvMxWAyunW34334551;     aTWKdwbeLOUvMxWAyunW34334551 = aTWKdwbeLOUvMxWAyunW90889842;     aTWKdwbeLOUvMxWAyunW90889842 = aTWKdwbeLOUvMxWAyunW85024558;     aTWKdwbeLOUvMxWAyunW85024558 = aTWKdwbeLOUvMxWAyunW32164954;     aTWKdwbeLOUvMxWAyunW32164954 = aTWKdwbeLOUvMxWAyunW78164468;     aTWKdwbeLOUvMxWAyunW78164468 = aTWKdwbeLOUvMxWAyunW88111659;     aTWKdwbeLOUvMxWAyunW88111659 = aTWKdwbeLOUvMxWAyunW88979149;     aTWKdwbeLOUvMxWAyunW88979149 = aTWKdwbeLOUvMxWAyunW64741034;     aTWKdwbeLOUvMxWAyunW64741034 = aTWKdwbeLOUvMxWAyunW79564548;     aTWKdwbeLOUvMxWAyunW79564548 = aTWKdwbeLOUvMxWAyunW58694987;     aTWKdwbeLOUvMxWAyunW58694987 = aTWKdwbeLOUvMxWAyunW83726027;     aTWKdwbeLOUvMxWAyunW83726027 = aTWKdwbeLOUvMxWAyunW52047015;     aTWKdwbeLOUvMxWAyunW52047015 = aTWKdwbeLOUvMxWAyunW65213908;     aTWKdwbeLOUvMxWAyunW65213908 = aTWKdwbeLOUvMxWAyunW34687508;     aTWKdwbeLOUvMxWAyunW34687508 = aTWKdwbeLOUvMxWAyunW38192848;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void HlrLuxwLtrCRpoZPEJpB58056128() {     double MvTBlpDIpvKRfwNvapJt63778124 = -857811712;    double MvTBlpDIpvKRfwNvapJt57420506 = -790625464;    double MvTBlpDIpvKRfwNvapJt80074056 = -667545272;    double MvTBlpDIpvKRfwNvapJt91038601 = -780755718;    double MvTBlpDIpvKRfwNvapJt33958121 = -689452261;    double MvTBlpDIpvKRfwNvapJt1439910 = -217139489;    double MvTBlpDIpvKRfwNvapJt97734868 = -393295453;    double MvTBlpDIpvKRfwNvapJt10396928 = -79519561;    double MvTBlpDIpvKRfwNvapJt81337878 = -621295349;    double MvTBlpDIpvKRfwNvapJt90765164 = -155634219;    double MvTBlpDIpvKRfwNvapJt99476262 = -621900496;    double MvTBlpDIpvKRfwNvapJt10545504 = -595794157;    double MvTBlpDIpvKRfwNvapJt86803556 = -428251001;    double MvTBlpDIpvKRfwNvapJt67797199 = -576208305;    double MvTBlpDIpvKRfwNvapJt39103047 = -359593691;    double MvTBlpDIpvKRfwNvapJt90581219 = 70844197;    double MvTBlpDIpvKRfwNvapJt67393237 = 79489281;    double MvTBlpDIpvKRfwNvapJt94209449 = -500455706;    double MvTBlpDIpvKRfwNvapJt49863593 = -724304709;    double MvTBlpDIpvKRfwNvapJt25962075 = -157677947;    double MvTBlpDIpvKRfwNvapJt9251084 = -451777440;    double MvTBlpDIpvKRfwNvapJt99873542 = 14195634;    double MvTBlpDIpvKRfwNvapJt56628157 = -846602416;    double MvTBlpDIpvKRfwNvapJt90455465 = -65912190;    double MvTBlpDIpvKRfwNvapJt50407940 = -365687749;    double MvTBlpDIpvKRfwNvapJt10372644 = -34433151;    double MvTBlpDIpvKRfwNvapJt64359220 = -127738674;    double MvTBlpDIpvKRfwNvapJt33759251 = -570994682;    double MvTBlpDIpvKRfwNvapJt93793069 = -941378869;    double MvTBlpDIpvKRfwNvapJt40236669 = -635481898;    double MvTBlpDIpvKRfwNvapJt37078810 = -434140050;    double MvTBlpDIpvKRfwNvapJt96186968 = -996793866;    double MvTBlpDIpvKRfwNvapJt75167915 = -723339839;    double MvTBlpDIpvKRfwNvapJt60831683 = -30716888;    double MvTBlpDIpvKRfwNvapJt73392353 = -19042713;    double MvTBlpDIpvKRfwNvapJt91115322 = -490340598;    double MvTBlpDIpvKRfwNvapJt96210502 = -895411339;    double MvTBlpDIpvKRfwNvapJt14363707 = -495001769;    double MvTBlpDIpvKRfwNvapJt89397221 = -527449594;    double MvTBlpDIpvKRfwNvapJt90807544 = -506442501;    double MvTBlpDIpvKRfwNvapJt28399842 = -758476697;    double MvTBlpDIpvKRfwNvapJt37206564 = -232520764;    double MvTBlpDIpvKRfwNvapJt70341642 = -903746806;    double MvTBlpDIpvKRfwNvapJt92581794 = -612072762;    double MvTBlpDIpvKRfwNvapJt52814057 = -173734018;    double MvTBlpDIpvKRfwNvapJt48303797 = -410653132;    double MvTBlpDIpvKRfwNvapJt45005922 = -399237970;    double MvTBlpDIpvKRfwNvapJt24628003 = -612728189;    double MvTBlpDIpvKRfwNvapJt67919022 = -916477124;    double MvTBlpDIpvKRfwNvapJt22602329 = -379848795;    double MvTBlpDIpvKRfwNvapJt24198668 = -270822819;    double MvTBlpDIpvKRfwNvapJt59967187 = -172914533;    double MvTBlpDIpvKRfwNvapJt42736089 = -954050528;    double MvTBlpDIpvKRfwNvapJt31989929 = -314472223;    double MvTBlpDIpvKRfwNvapJt48805045 = 96549826;    double MvTBlpDIpvKRfwNvapJt63904582 = -772007346;    double MvTBlpDIpvKRfwNvapJt792349 = -944023049;    double MvTBlpDIpvKRfwNvapJt89618590 = -501633083;    double MvTBlpDIpvKRfwNvapJt40630661 = -315067970;    double MvTBlpDIpvKRfwNvapJt23585477 = -555019111;    double MvTBlpDIpvKRfwNvapJt37080690 = 10599185;    double MvTBlpDIpvKRfwNvapJt63975618 = -822300771;    double MvTBlpDIpvKRfwNvapJt16603859 = -138140693;    double MvTBlpDIpvKRfwNvapJt41101210 = -985813452;    double MvTBlpDIpvKRfwNvapJt53686355 = -721494169;    double MvTBlpDIpvKRfwNvapJt3289295 = -625106631;    double MvTBlpDIpvKRfwNvapJt35377589 = -872454319;    double MvTBlpDIpvKRfwNvapJt25971873 = -297534113;    double MvTBlpDIpvKRfwNvapJt94404845 = -457165593;    double MvTBlpDIpvKRfwNvapJt47987724 = -869253093;    double MvTBlpDIpvKRfwNvapJt94370716 = -33744465;    double MvTBlpDIpvKRfwNvapJt53029530 = -425508951;    double MvTBlpDIpvKRfwNvapJt4812228 = -973006113;    double MvTBlpDIpvKRfwNvapJt59056048 = -117862208;    double MvTBlpDIpvKRfwNvapJt97562233 = -399201250;    double MvTBlpDIpvKRfwNvapJt72044519 = -119256676;    double MvTBlpDIpvKRfwNvapJt29531900 = -82057561;    double MvTBlpDIpvKRfwNvapJt64046363 = -134529654;    double MvTBlpDIpvKRfwNvapJt37641409 = -892178173;    double MvTBlpDIpvKRfwNvapJt2104144 = -955034617;    double MvTBlpDIpvKRfwNvapJt65366722 = -635195182;    double MvTBlpDIpvKRfwNvapJt39731217 = -515010486;    double MvTBlpDIpvKRfwNvapJt65840228 = -654517558;    double MvTBlpDIpvKRfwNvapJt71190740 = -461530074;    double MvTBlpDIpvKRfwNvapJt16038001 = -264659079;    double MvTBlpDIpvKRfwNvapJt77111623 = -161225517;    double MvTBlpDIpvKRfwNvapJt53450880 = 57256662;    double MvTBlpDIpvKRfwNvapJt43177986 = -308867617;    double MvTBlpDIpvKRfwNvapJt12026639 = -27266715;    double MvTBlpDIpvKRfwNvapJt9487772 = -247035367;    double MvTBlpDIpvKRfwNvapJt90322973 = -546317550;    double MvTBlpDIpvKRfwNvapJt6591912 = -293778257;    double MvTBlpDIpvKRfwNvapJt73733046 = -79933799;    double MvTBlpDIpvKRfwNvapJt65811744 = -972430484;    double MvTBlpDIpvKRfwNvapJt53726855 = -417041686;    double MvTBlpDIpvKRfwNvapJt64424224 = -936175927;    double MvTBlpDIpvKRfwNvapJt20602706 = 5619928;    double MvTBlpDIpvKRfwNvapJt29240433 = -917933354;    double MvTBlpDIpvKRfwNvapJt38895439 = -890578593;    double MvTBlpDIpvKRfwNvapJt49524762 = -857811712;     MvTBlpDIpvKRfwNvapJt63778124 = MvTBlpDIpvKRfwNvapJt57420506;     MvTBlpDIpvKRfwNvapJt57420506 = MvTBlpDIpvKRfwNvapJt80074056;     MvTBlpDIpvKRfwNvapJt80074056 = MvTBlpDIpvKRfwNvapJt91038601;     MvTBlpDIpvKRfwNvapJt91038601 = MvTBlpDIpvKRfwNvapJt33958121;     MvTBlpDIpvKRfwNvapJt33958121 = MvTBlpDIpvKRfwNvapJt1439910;     MvTBlpDIpvKRfwNvapJt1439910 = MvTBlpDIpvKRfwNvapJt97734868;     MvTBlpDIpvKRfwNvapJt97734868 = MvTBlpDIpvKRfwNvapJt10396928;     MvTBlpDIpvKRfwNvapJt10396928 = MvTBlpDIpvKRfwNvapJt81337878;     MvTBlpDIpvKRfwNvapJt81337878 = MvTBlpDIpvKRfwNvapJt90765164;     MvTBlpDIpvKRfwNvapJt90765164 = MvTBlpDIpvKRfwNvapJt99476262;     MvTBlpDIpvKRfwNvapJt99476262 = MvTBlpDIpvKRfwNvapJt10545504;     MvTBlpDIpvKRfwNvapJt10545504 = MvTBlpDIpvKRfwNvapJt86803556;     MvTBlpDIpvKRfwNvapJt86803556 = MvTBlpDIpvKRfwNvapJt67797199;     MvTBlpDIpvKRfwNvapJt67797199 = MvTBlpDIpvKRfwNvapJt39103047;     MvTBlpDIpvKRfwNvapJt39103047 = MvTBlpDIpvKRfwNvapJt90581219;     MvTBlpDIpvKRfwNvapJt90581219 = MvTBlpDIpvKRfwNvapJt67393237;     MvTBlpDIpvKRfwNvapJt67393237 = MvTBlpDIpvKRfwNvapJt94209449;     MvTBlpDIpvKRfwNvapJt94209449 = MvTBlpDIpvKRfwNvapJt49863593;     MvTBlpDIpvKRfwNvapJt49863593 = MvTBlpDIpvKRfwNvapJt25962075;     MvTBlpDIpvKRfwNvapJt25962075 = MvTBlpDIpvKRfwNvapJt9251084;     MvTBlpDIpvKRfwNvapJt9251084 = MvTBlpDIpvKRfwNvapJt99873542;     MvTBlpDIpvKRfwNvapJt99873542 = MvTBlpDIpvKRfwNvapJt56628157;     MvTBlpDIpvKRfwNvapJt56628157 = MvTBlpDIpvKRfwNvapJt90455465;     MvTBlpDIpvKRfwNvapJt90455465 = MvTBlpDIpvKRfwNvapJt50407940;     MvTBlpDIpvKRfwNvapJt50407940 = MvTBlpDIpvKRfwNvapJt10372644;     MvTBlpDIpvKRfwNvapJt10372644 = MvTBlpDIpvKRfwNvapJt64359220;     MvTBlpDIpvKRfwNvapJt64359220 = MvTBlpDIpvKRfwNvapJt33759251;     MvTBlpDIpvKRfwNvapJt33759251 = MvTBlpDIpvKRfwNvapJt93793069;     MvTBlpDIpvKRfwNvapJt93793069 = MvTBlpDIpvKRfwNvapJt40236669;     MvTBlpDIpvKRfwNvapJt40236669 = MvTBlpDIpvKRfwNvapJt37078810;     MvTBlpDIpvKRfwNvapJt37078810 = MvTBlpDIpvKRfwNvapJt96186968;     MvTBlpDIpvKRfwNvapJt96186968 = MvTBlpDIpvKRfwNvapJt75167915;     MvTBlpDIpvKRfwNvapJt75167915 = MvTBlpDIpvKRfwNvapJt60831683;     MvTBlpDIpvKRfwNvapJt60831683 = MvTBlpDIpvKRfwNvapJt73392353;     MvTBlpDIpvKRfwNvapJt73392353 = MvTBlpDIpvKRfwNvapJt91115322;     MvTBlpDIpvKRfwNvapJt91115322 = MvTBlpDIpvKRfwNvapJt96210502;     MvTBlpDIpvKRfwNvapJt96210502 = MvTBlpDIpvKRfwNvapJt14363707;     MvTBlpDIpvKRfwNvapJt14363707 = MvTBlpDIpvKRfwNvapJt89397221;     MvTBlpDIpvKRfwNvapJt89397221 = MvTBlpDIpvKRfwNvapJt90807544;     MvTBlpDIpvKRfwNvapJt90807544 = MvTBlpDIpvKRfwNvapJt28399842;     MvTBlpDIpvKRfwNvapJt28399842 = MvTBlpDIpvKRfwNvapJt37206564;     MvTBlpDIpvKRfwNvapJt37206564 = MvTBlpDIpvKRfwNvapJt70341642;     MvTBlpDIpvKRfwNvapJt70341642 = MvTBlpDIpvKRfwNvapJt92581794;     MvTBlpDIpvKRfwNvapJt92581794 = MvTBlpDIpvKRfwNvapJt52814057;     MvTBlpDIpvKRfwNvapJt52814057 = MvTBlpDIpvKRfwNvapJt48303797;     MvTBlpDIpvKRfwNvapJt48303797 = MvTBlpDIpvKRfwNvapJt45005922;     MvTBlpDIpvKRfwNvapJt45005922 = MvTBlpDIpvKRfwNvapJt24628003;     MvTBlpDIpvKRfwNvapJt24628003 = MvTBlpDIpvKRfwNvapJt67919022;     MvTBlpDIpvKRfwNvapJt67919022 = MvTBlpDIpvKRfwNvapJt22602329;     MvTBlpDIpvKRfwNvapJt22602329 = MvTBlpDIpvKRfwNvapJt24198668;     MvTBlpDIpvKRfwNvapJt24198668 = MvTBlpDIpvKRfwNvapJt59967187;     MvTBlpDIpvKRfwNvapJt59967187 = MvTBlpDIpvKRfwNvapJt42736089;     MvTBlpDIpvKRfwNvapJt42736089 = MvTBlpDIpvKRfwNvapJt31989929;     MvTBlpDIpvKRfwNvapJt31989929 = MvTBlpDIpvKRfwNvapJt48805045;     MvTBlpDIpvKRfwNvapJt48805045 = MvTBlpDIpvKRfwNvapJt63904582;     MvTBlpDIpvKRfwNvapJt63904582 = MvTBlpDIpvKRfwNvapJt792349;     MvTBlpDIpvKRfwNvapJt792349 = MvTBlpDIpvKRfwNvapJt89618590;     MvTBlpDIpvKRfwNvapJt89618590 = MvTBlpDIpvKRfwNvapJt40630661;     MvTBlpDIpvKRfwNvapJt40630661 = MvTBlpDIpvKRfwNvapJt23585477;     MvTBlpDIpvKRfwNvapJt23585477 = MvTBlpDIpvKRfwNvapJt37080690;     MvTBlpDIpvKRfwNvapJt37080690 = MvTBlpDIpvKRfwNvapJt63975618;     MvTBlpDIpvKRfwNvapJt63975618 = MvTBlpDIpvKRfwNvapJt16603859;     MvTBlpDIpvKRfwNvapJt16603859 = MvTBlpDIpvKRfwNvapJt41101210;     MvTBlpDIpvKRfwNvapJt41101210 = MvTBlpDIpvKRfwNvapJt53686355;     MvTBlpDIpvKRfwNvapJt53686355 = MvTBlpDIpvKRfwNvapJt3289295;     MvTBlpDIpvKRfwNvapJt3289295 = MvTBlpDIpvKRfwNvapJt35377589;     MvTBlpDIpvKRfwNvapJt35377589 = MvTBlpDIpvKRfwNvapJt25971873;     MvTBlpDIpvKRfwNvapJt25971873 = MvTBlpDIpvKRfwNvapJt94404845;     MvTBlpDIpvKRfwNvapJt94404845 = MvTBlpDIpvKRfwNvapJt47987724;     MvTBlpDIpvKRfwNvapJt47987724 = MvTBlpDIpvKRfwNvapJt94370716;     MvTBlpDIpvKRfwNvapJt94370716 = MvTBlpDIpvKRfwNvapJt53029530;     MvTBlpDIpvKRfwNvapJt53029530 = MvTBlpDIpvKRfwNvapJt4812228;     MvTBlpDIpvKRfwNvapJt4812228 = MvTBlpDIpvKRfwNvapJt59056048;     MvTBlpDIpvKRfwNvapJt59056048 = MvTBlpDIpvKRfwNvapJt97562233;     MvTBlpDIpvKRfwNvapJt97562233 = MvTBlpDIpvKRfwNvapJt72044519;     MvTBlpDIpvKRfwNvapJt72044519 = MvTBlpDIpvKRfwNvapJt29531900;     MvTBlpDIpvKRfwNvapJt29531900 = MvTBlpDIpvKRfwNvapJt64046363;     MvTBlpDIpvKRfwNvapJt64046363 = MvTBlpDIpvKRfwNvapJt37641409;     MvTBlpDIpvKRfwNvapJt37641409 = MvTBlpDIpvKRfwNvapJt2104144;     MvTBlpDIpvKRfwNvapJt2104144 = MvTBlpDIpvKRfwNvapJt65366722;     MvTBlpDIpvKRfwNvapJt65366722 = MvTBlpDIpvKRfwNvapJt39731217;     MvTBlpDIpvKRfwNvapJt39731217 = MvTBlpDIpvKRfwNvapJt65840228;     MvTBlpDIpvKRfwNvapJt65840228 = MvTBlpDIpvKRfwNvapJt71190740;     MvTBlpDIpvKRfwNvapJt71190740 = MvTBlpDIpvKRfwNvapJt16038001;     MvTBlpDIpvKRfwNvapJt16038001 = MvTBlpDIpvKRfwNvapJt77111623;     MvTBlpDIpvKRfwNvapJt77111623 = MvTBlpDIpvKRfwNvapJt53450880;     MvTBlpDIpvKRfwNvapJt53450880 = MvTBlpDIpvKRfwNvapJt43177986;     MvTBlpDIpvKRfwNvapJt43177986 = MvTBlpDIpvKRfwNvapJt12026639;     MvTBlpDIpvKRfwNvapJt12026639 = MvTBlpDIpvKRfwNvapJt9487772;     MvTBlpDIpvKRfwNvapJt9487772 = MvTBlpDIpvKRfwNvapJt90322973;     MvTBlpDIpvKRfwNvapJt90322973 = MvTBlpDIpvKRfwNvapJt6591912;     MvTBlpDIpvKRfwNvapJt6591912 = MvTBlpDIpvKRfwNvapJt73733046;     MvTBlpDIpvKRfwNvapJt73733046 = MvTBlpDIpvKRfwNvapJt65811744;     MvTBlpDIpvKRfwNvapJt65811744 = MvTBlpDIpvKRfwNvapJt53726855;     MvTBlpDIpvKRfwNvapJt53726855 = MvTBlpDIpvKRfwNvapJt64424224;     MvTBlpDIpvKRfwNvapJt64424224 = MvTBlpDIpvKRfwNvapJt20602706;     MvTBlpDIpvKRfwNvapJt20602706 = MvTBlpDIpvKRfwNvapJt29240433;     MvTBlpDIpvKRfwNvapJt29240433 = MvTBlpDIpvKRfwNvapJt38895439;     MvTBlpDIpvKRfwNvapJt38895439 = MvTBlpDIpvKRfwNvapJt49524762;     MvTBlpDIpvKRfwNvapJt49524762 = MvTBlpDIpvKRfwNvapJt63778124;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void UYUJZyHmoWVRFVuwGxvd85650384() {     double iHbTIdzfVivPsXZorUBx89363401 = -282844813;    double iHbTIdzfVivPsXZorUBx53817649 = 53896759;    double iHbTIdzfVivPsXZorUBx12953233 = -470006808;    double iHbTIdzfVivPsXZorUBx36760531 = -788078612;    double iHbTIdzfVivPsXZorUBx3300126 = -736374549;    double iHbTIdzfVivPsXZorUBx18449186 = -372120492;    double iHbTIdzfVivPsXZorUBx3055714 = -975566660;    double iHbTIdzfVivPsXZorUBx25407052 = -460201440;    double iHbTIdzfVivPsXZorUBx6667171 = -17519394;    double iHbTIdzfVivPsXZorUBx9923547 = -490195743;    double iHbTIdzfVivPsXZorUBx13809817 = -932328383;    double iHbTIdzfVivPsXZorUBx45863160 = 60496936;    double iHbTIdzfVivPsXZorUBx63384334 = -192499366;    double iHbTIdzfVivPsXZorUBx59356916 = -550732599;    double iHbTIdzfVivPsXZorUBx95371934 = -248139649;    double iHbTIdzfVivPsXZorUBx98302261 = -365347280;    double iHbTIdzfVivPsXZorUBx67191354 = -708830513;    double iHbTIdzfVivPsXZorUBx15325354 = 83266933;    double iHbTIdzfVivPsXZorUBx11328315 = -407278566;    double iHbTIdzfVivPsXZorUBx87123541 = -132117322;    double iHbTIdzfVivPsXZorUBx10260349 = -913477221;    double iHbTIdzfVivPsXZorUBx1376650 = -405537296;    double iHbTIdzfVivPsXZorUBx60391955 = -890694521;    double iHbTIdzfVivPsXZorUBx92169508 = 75238830;    double iHbTIdzfVivPsXZorUBx5311080 = 66816109;    double iHbTIdzfVivPsXZorUBx39861780 = -698883693;    double iHbTIdzfVivPsXZorUBx46747129 = -793551809;    double iHbTIdzfVivPsXZorUBx97658051 = -800656914;    double iHbTIdzfVivPsXZorUBx1670282 = -77672719;    double iHbTIdzfVivPsXZorUBx93559159 = -544391459;    double iHbTIdzfVivPsXZorUBx59125444 = -212787081;    double iHbTIdzfVivPsXZorUBx33376901 = -170589700;    double iHbTIdzfVivPsXZorUBx96354966 = -71160976;    double iHbTIdzfVivPsXZorUBx14418755 = -86861860;    double iHbTIdzfVivPsXZorUBx74797342 = -720478624;    double iHbTIdzfVivPsXZorUBx95907173 = 92064686;    double iHbTIdzfVivPsXZorUBx45855888 = -275923283;    double iHbTIdzfVivPsXZorUBx89936393 = -343716594;    double iHbTIdzfVivPsXZorUBx30320800 = -985787916;    double iHbTIdzfVivPsXZorUBx99591218 = -976412820;    double iHbTIdzfVivPsXZorUBx75551122 = -233536887;    double iHbTIdzfVivPsXZorUBx81216154 = -741559268;    double iHbTIdzfVivPsXZorUBx19541862 = -932353188;    double iHbTIdzfVivPsXZorUBx63375073 = 19511639;    double iHbTIdzfVivPsXZorUBx44794933 = -112755135;    double iHbTIdzfVivPsXZorUBx14704120 = -933200670;    double iHbTIdzfVivPsXZorUBx44774376 = -431706830;    double iHbTIdzfVivPsXZorUBx32740773 = -805911036;    double iHbTIdzfVivPsXZorUBx18730343 = 83846649;    double iHbTIdzfVivPsXZorUBx76412324 = -227871948;    double iHbTIdzfVivPsXZorUBx34334915 = -519481421;    double iHbTIdzfVivPsXZorUBx87225766 = -350993713;    double iHbTIdzfVivPsXZorUBx60809692 = -421949077;    double iHbTIdzfVivPsXZorUBx888838 = -629536004;    double iHbTIdzfVivPsXZorUBx75390036 = -378540592;    double iHbTIdzfVivPsXZorUBx87986752 = -877307517;    double iHbTIdzfVivPsXZorUBx93425694 = -55408720;    double iHbTIdzfVivPsXZorUBx20783724 = -445245639;    double iHbTIdzfVivPsXZorUBx31449452 = -754894721;    double iHbTIdzfVivPsXZorUBx63438345 = 62509144;    double iHbTIdzfVivPsXZorUBx71702057 = -578568684;    double iHbTIdzfVivPsXZorUBx5397663 = -74909746;    double iHbTIdzfVivPsXZorUBx23736770 = -282528721;    double iHbTIdzfVivPsXZorUBx13108012 = -473127935;    double iHbTIdzfVivPsXZorUBx50798103 = -177408662;    double iHbTIdzfVivPsXZorUBx80432915 = -661738684;    double iHbTIdzfVivPsXZorUBx49508193 = -868342088;    double iHbTIdzfVivPsXZorUBx48965580 = -5637507;    double iHbTIdzfVivPsXZorUBx84559574 = -830253975;    double iHbTIdzfVivPsXZorUBx99464761 = -240204335;    double iHbTIdzfVivPsXZorUBx52446374 = 10576003;    double iHbTIdzfVivPsXZorUBx77254960 = -265113920;    double iHbTIdzfVivPsXZorUBx85004553 = 69054849;    double iHbTIdzfVivPsXZorUBx11737096 = -430865747;    double iHbTIdzfVivPsXZorUBx11572419 = -898580436;    double iHbTIdzfVivPsXZorUBx29044195 = -71917953;    double iHbTIdzfVivPsXZorUBx81834787 = -473184109;    double iHbTIdzfVivPsXZorUBx97016881 = -810206161;    double iHbTIdzfVivPsXZorUBx47374575 = -812006035;    double iHbTIdzfVivPsXZorUBx90606960 = 16779;    double iHbTIdzfVivPsXZorUBx95087404 = -167176863;    double iHbTIdzfVivPsXZorUBx14006356 = -987640774;    double iHbTIdzfVivPsXZorUBx78927708 = -784503564;    double iHbTIdzfVivPsXZorUBx25257958 = -849800772;    double iHbTIdzfVivPsXZorUBx59224244 = 75089961;    double iHbTIdzfVivPsXZorUBx71899677 = -861793369;    double iHbTIdzfVivPsXZorUBx72567208 = -748640623;    double iHbTIdzfVivPsXZorUBx95466129 = -441624972;    double iHbTIdzfVivPsXZorUBx39028718 = -708321269;    double iHbTIdzfVivPsXZorUBx86810589 = -843171108;    double iHbTIdzfVivPsXZorUBx2481479 = -852526595;    double iHbTIdzfVivPsXZorUBx25072164 = -830677645;    double iHbTIdzfVivPsXZorUBx58486942 = -588821873;    double iHbTIdzfVivPsXZorUBx66882455 = -948297060;    double iHbTIdzfVivPsXZorUBx27889161 = -297844137;    double iHbTIdzfVivPsXZorUBx70153460 = -58627142;    double iHbTIdzfVivPsXZorUBx57479384 = -359030547;    double iHbTIdzfVivPsXZorUBx6433851 = -359225253;    double iHbTIdzfVivPsXZorUBx12576971 = -803079699;    double iHbTIdzfVivPsXZorUBx64362017 = -282844813;     iHbTIdzfVivPsXZorUBx89363401 = iHbTIdzfVivPsXZorUBx53817649;     iHbTIdzfVivPsXZorUBx53817649 = iHbTIdzfVivPsXZorUBx12953233;     iHbTIdzfVivPsXZorUBx12953233 = iHbTIdzfVivPsXZorUBx36760531;     iHbTIdzfVivPsXZorUBx36760531 = iHbTIdzfVivPsXZorUBx3300126;     iHbTIdzfVivPsXZorUBx3300126 = iHbTIdzfVivPsXZorUBx18449186;     iHbTIdzfVivPsXZorUBx18449186 = iHbTIdzfVivPsXZorUBx3055714;     iHbTIdzfVivPsXZorUBx3055714 = iHbTIdzfVivPsXZorUBx25407052;     iHbTIdzfVivPsXZorUBx25407052 = iHbTIdzfVivPsXZorUBx6667171;     iHbTIdzfVivPsXZorUBx6667171 = iHbTIdzfVivPsXZorUBx9923547;     iHbTIdzfVivPsXZorUBx9923547 = iHbTIdzfVivPsXZorUBx13809817;     iHbTIdzfVivPsXZorUBx13809817 = iHbTIdzfVivPsXZorUBx45863160;     iHbTIdzfVivPsXZorUBx45863160 = iHbTIdzfVivPsXZorUBx63384334;     iHbTIdzfVivPsXZorUBx63384334 = iHbTIdzfVivPsXZorUBx59356916;     iHbTIdzfVivPsXZorUBx59356916 = iHbTIdzfVivPsXZorUBx95371934;     iHbTIdzfVivPsXZorUBx95371934 = iHbTIdzfVivPsXZorUBx98302261;     iHbTIdzfVivPsXZorUBx98302261 = iHbTIdzfVivPsXZorUBx67191354;     iHbTIdzfVivPsXZorUBx67191354 = iHbTIdzfVivPsXZorUBx15325354;     iHbTIdzfVivPsXZorUBx15325354 = iHbTIdzfVivPsXZorUBx11328315;     iHbTIdzfVivPsXZorUBx11328315 = iHbTIdzfVivPsXZorUBx87123541;     iHbTIdzfVivPsXZorUBx87123541 = iHbTIdzfVivPsXZorUBx10260349;     iHbTIdzfVivPsXZorUBx10260349 = iHbTIdzfVivPsXZorUBx1376650;     iHbTIdzfVivPsXZorUBx1376650 = iHbTIdzfVivPsXZorUBx60391955;     iHbTIdzfVivPsXZorUBx60391955 = iHbTIdzfVivPsXZorUBx92169508;     iHbTIdzfVivPsXZorUBx92169508 = iHbTIdzfVivPsXZorUBx5311080;     iHbTIdzfVivPsXZorUBx5311080 = iHbTIdzfVivPsXZorUBx39861780;     iHbTIdzfVivPsXZorUBx39861780 = iHbTIdzfVivPsXZorUBx46747129;     iHbTIdzfVivPsXZorUBx46747129 = iHbTIdzfVivPsXZorUBx97658051;     iHbTIdzfVivPsXZorUBx97658051 = iHbTIdzfVivPsXZorUBx1670282;     iHbTIdzfVivPsXZorUBx1670282 = iHbTIdzfVivPsXZorUBx93559159;     iHbTIdzfVivPsXZorUBx93559159 = iHbTIdzfVivPsXZorUBx59125444;     iHbTIdzfVivPsXZorUBx59125444 = iHbTIdzfVivPsXZorUBx33376901;     iHbTIdzfVivPsXZorUBx33376901 = iHbTIdzfVivPsXZorUBx96354966;     iHbTIdzfVivPsXZorUBx96354966 = iHbTIdzfVivPsXZorUBx14418755;     iHbTIdzfVivPsXZorUBx14418755 = iHbTIdzfVivPsXZorUBx74797342;     iHbTIdzfVivPsXZorUBx74797342 = iHbTIdzfVivPsXZorUBx95907173;     iHbTIdzfVivPsXZorUBx95907173 = iHbTIdzfVivPsXZorUBx45855888;     iHbTIdzfVivPsXZorUBx45855888 = iHbTIdzfVivPsXZorUBx89936393;     iHbTIdzfVivPsXZorUBx89936393 = iHbTIdzfVivPsXZorUBx30320800;     iHbTIdzfVivPsXZorUBx30320800 = iHbTIdzfVivPsXZorUBx99591218;     iHbTIdzfVivPsXZorUBx99591218 = iHbTIdzfVivPsXZorUBx75551122;     iHbTIdzfVivPsXZorUBx75551122 = iHbTIdzfVivPsXZorUBx81216154;     iHbTIdzfVivPsXZorUBx81216154 = iHbTIdzfVivPsXZorUBx19541862;     iHbTIdzfVivPsXZorUBx19541862 = iHbTIdzfVivPsXZorUBx63375073;     iHbTIdzfVivPsXZorUBx63375073 = iHbTIdzfVivPsXZorUBx44794933;     iHbTIdzfVivPsXZorUBx44794933 = iHbTIdzfVivPsXZorUBx14704120;     iHbTIdzfVivPsXZorUBx14704120 = iHbTIdzfVivPsXZorUBx44774376;     iHbTIdzfVivPsXZorUBx44774376 = iHbTIdzfVivPsXZorUBx32740773;     iHbTIdzfVivPsXZorUBx32740773 = iHbTIdzfVivPsXZorUBx18730343;     iHbTIdzfVivPsXZorUBx18730343 = iHbTIdzfVivPsXZorUBx76412324;     iHbTIdzfVivPsXZorUBx76412324 = iHbTIdzfVivPsXZorUBx34334915;     iHbTIdzfVivPsXZorUBx34334915 = iHbTIdzfVivPsXZorUBx87225766;     iHbTIdzfVivPsXZorUBx87225766 = iHbTIdzfVivPsXZorUBx60809692;     iHbTIdzfVivPsXZorUBx60809692 = iHbTIdzfVivPsXZorUBx888838;     iHbTIdzfVivPsXZorUBx888838 = iHbTIdzfVivPsXZorUBx75390036;     iHbTIdzfVivPsXZorUBx75390036 = iHbTIdzfVivPsXZorUBx87986752;     iHbTIdzfVivPsXZorUBx87986752 = iHbTIdzfVivPsXZorUBx93425694;     iHbTIdzfVivPsXZorUBx93425694 = iHbTIdzfVivPsXZorUBx20783724;     iHbTIdzfVivPsXZorUBx20783724 = iHbTIdzfVivPsXZorUBx31449452;     iHbTIdzfVivPsXZorUBx31449452 = iHbTIdzfVivPsXZorUBx63438345;     iHbTIdzfVivPsXZorUBx63438345 = iHbTIdzfVivPsXZorUBx71702057;     iHbTIdzfVivPsXZorUBx71702057 = iHbTIdzfVivPsXZorUBx5397663;     iHbTIdzfVivPsXZorUBx5397663 = iHbTIdzfVivPsXZorUBx23736770;     iHbTIdzfVivPsXZorUBx23736770 = iHbTIdzfVivPsXZorUBx13108012;     iHbTIdzfVivPsXZorUBx13108012 = iHbTIdzfVivPsXZorUBx50798103;     iHbTIdzfVivPsXZorUBx50798103 = iHbTIdzfVivPsXZorUBx80432915;     iHbTIdzfVivPsXZorUBx80432915 = iHbTIdzfVivPsXZorUBx49508193;     iHbTIdzfVivPsXZorUBx49508193 = iHbTIdzfVivPsXZorUBx48965580;     iHbTIdzfVivPsXZorUBx48965580 = iHbTIdzfVivPsXZorUBx84559574;     iHbTIdzfVivPsXZorUBx84559574 = iHbTIdzfVivPsXZorUBx99464761;     iHbTIdzfVivPsXZorUBx99464761 = iHbTIdzfVivPsXZorUBx52446374;     iHbTIdzfVivPsXZorUBx52446374 = iHbTIdzfVivPsXZorUBx77254960;     iHbTIdzfVivPsXZorUBx77254960 = iHbTIdzfVivPsXZorUBx85004553;     iHbTIdzfVivPsXZorUBx85004553 = iHbTIdzfVivPsXZorUBx11737096;     iHbTIdzfVivPsXZorUBx11737096 = iHbTIdzfVivPsXZorUBx11572419;     iHbTIdzfVivPsXZorUBx11572419 = iHbTIdzfVivPsXZorUBx29044195;     iHbTIdzfVivPsXZorUBx29044195 = iHbTIdzfVivPsXZorUBx81834787;     iHbTIdzfVivPsXZorUBx81834787 = iHbTIdzfVivPsXZorUBx97016881;     iHbTIdzfVivPsXZorUBx97016881 = iHbTIdzfVivPsXZorUBx47374575;     iHbTIdzfVivPsXZorUBx47374575 = iHbTIdzfVivPsXZorUBx90606960;     iHbTIdzfVivPsXZorUBx90606960 = iHbTIdzfVivPsXZorUBx95087404;     iHbTIdzfVivPsXZorUBx95087404 = iHbTIdzfVivPsXZorUBx14006356;     iHbTIdzfVivPsXZorUBx14006356 = iHbTIdzfVivPsXZorUBx78927708;     iHbTIdzfVivPsXZorUBx78927708 = iHbTIdzfVivPsXZorUBx25257958;     iHbTIdzfVivPsXZorUBx25257958 = iHbTIdzfVivPsXZorUBx59224244;     iHbTIdzfVivPsXZorUBx59224244 = iHbTIdzfVivPsXZorUBx71899677;     iHbTIdzfVivPsXZorUBx71899677 = iHbTIdzfVivPsXZorUBx72567208;     iHbTIdzfVivPsXZorUBx72567208 = iHbTIdzfVivPsXZorUBx95466129;     iHbTIdzfVivPsXZorUBx95466129 = iHbTIdzfVivPsXZorUBx39028718;     iHbTIdzfVivPsXZorUBx39028718 = iHbTIdzfVivPsXZorUBx86810589;     iHbTIdzfVivPsXZorUBx86810589 = iHbTIdzfVivPsXZorUBx2481479;     iHbTIdzfVivPsXZorUBx2481479 = iHbTIdzfVivPsXZorUBx25072164;     iHbTIdzfVivPsXZorUBx25072164 = iHbTIdzfVivPsXZorUBx58486942;     iHbTIdzfVivPsXZorUBx58486942 = iHbTIdzfVivPsXZorUBx66882455;     iHbTIdzfVivPsXZorUBx66882455 = iHbTIdzfVivPsXZorUBx27889161;     iHbTIdzfVivPsXZorUBx27889161 = iHbTIdzfVivPsXZorUBx70153460;     iHbTIdzfVivPsXZorUBx70153460 = iHbTIdzfVivPsXZorUBx57479384;     iHbTIdzfVivPsXZorUBx57479384 = iHbTIdzfVivPsXZorUBx6433851;     iHbTIdzfVivPsXZorUBx6433851 = iHbTIdzfVivPsXZorUBx12576971;     iHbTIdzfVivPsXZorUBx12576971 = iHbTIdzfVivPsXZorUBx64362017;     iHbTIdzfVivPsXZorUBx64362017 = iHbTIdzfVivPsXZorUBx89363401;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void poyIeLHyZkKkTGzUdyfM13244642() {     double aUyItufZoKflauPRrjTW14948679 = -807877913;    double aUyItufZoKflauPRrjTW50214793 = -201581017;    double aUyItufZoKflauPRrjTW45832408 = -272468344;    double aUyItufZoKflauPRrjTW82482460 = -795401505;    double aUyItufZoKflauPRrjTW72642130 = -783296836;    double aUyItufZoKflauPRrjTW35458463 = -527101495;    double aUyItufZoKflauPRrjTW8376558 = -457837866;    double aUyItufZoKflauPRrjTW40417176 = -840883319;    double aUyItufZoKflauPRrjTW31996464 = -513743439;    double aUyItufZoKflauPRrjTW29081930 = -824757266;    double aUyItufZoKflauPRrjTW28143371 = -142756269;    double aUyItufZoKflauPRrjTW81180815 = -383211971;    double aUyItufZoKflauPRrjTW39965112 = 43252268;    double aUyItufZoKflauPRrjTW50916633 = -525256892;    double aUyItufZoKflauPRrjTW51640823 = -136685607;    double aUyItufZoKflauPRrjTW6023305 = -801538757;    double aUyItufZoKflauPRrjTW66989470 = -397150307;    double aUyItufZoKflauPRrjTW36441258 = -433010427;    double aUyItufZoKflauPRrjTW72793035 = -90252423;    double aUyItufZoKflauPRrjTW48285008 = -106556698;    double aUyItufZoKflauPRrjTW11269614 = -275177002;    double aUyItufZoKflauPRrjTW2879756 = -825270225;    double aUyItufZoKflauPRrjTW64155753 = -934786626;    double aUyItufZoKflauPRrjTW93883550 = -883610149;    double aUyItufZoKflauPRrjTW60214219 = -600680033;    double aUyItufZoKflauPRrjTW69350917 = -263334234;    double aUyItufZoKflauPRrjTW29135038 = -359364944;    double aUyItufZoKflauPRrjTW61556851 = 69680853;    double aUyItufZoKflauPRrjTW9547495 = -313966570;    double aUyItufZoKflauPRrjTW46881650 = -453301021;    double aUyItufZoKflauPRrjTW81172078 = 8565888;    double aUyItufZoKflauPRrjTW70566833 = -444385533;    double aUyItufZoKflauPRrjTW17542019 = -518982113;    double aUyItufZoKflauPRrjTW68005825 = -143006831;    double aUyItufZoKflauPRrjTW76202330 = -321914536;    double aUyItufZoKflauPRrjTW699024 = -425530030;    double aUyItufZoKflauPRrjTW95501273 = -756435228;    double aUyItufZoKflauPRrjTW65509080 = -192431419;    double aUyItufZoKflauPRrjTW71244378 = -344126238;    double aUyItufZoKflauPRrjTW8374893 = -346383139;    double aUyItufZoKflauPRrjTW22702404 = -808597077;    double aUyItufZoKflauPRrjTW25225744 = -150597772;    double aUyItufZoKflauPRrjTW68742081 = -960959569;    double aUyItufZoKflauPRrjTW34168353 = -448903960;    double aUyItufZoKflauPRrjTW36775809 = -51776252;    double aUyItufZoKflauPRrjTW81104441 = -355748208;    double aUyItufZoKflauPRrjTW44542830 = -464175691;    double aUyItufZoKflauPRrjTW40853544 = -999093882;    double aUyItufZoKflauPRrjTW69541662 = -15829577;    double aUyItufZoKflauPRrjTW30222319 = -75895101;    double aUyItufZoKflauPRrjTW44471163 = -768140023;    double aUyItufZoKflauPRrjTW14484346 = -529072892;    double aUyItufZoKflauPRrjTW78883296 = -989847626;    double aUyItufZoKflauPRrjTW69787746 = -944599786;    double aUyItufZoKflauPRrjTW1975028 = -853631009;    double aUyItufZoKflauPRrjTW12068923 = -982607688;    double aUyItufZoKflauPRrjTW86059039 = -266794392;    double aUyItufZoKflauPRrjTW51948857 = -388858195;    double aUyItufZoKflauPRrjTW22268242 = -94721472;    double aUyItufZoKflauPRrjTW3291214 = -419962602;    double aUyItufZoKflauPRrjTW6323425 = -67736552;    double aUyItufZoKflauPRrjTW46819706 = -427518720;    double aUyItufZoKflauPRrjTW30869682 = -426916750;    double aUyItufZoKflauPRrjTW85114813 = 39557582;    double aUyItufZoKflauPRrjTW47909851 = -733323154;    double aUyItufZoKflauPRrjTW57576537 = -698370736;    double aUyItufZoKflauPRrjTW63638797 = -864229858;    double aUyItufZoKflauPRrjTW71959286 = -813740901;    double aUyItufZoKflauPRrjTW74714303 = -103342357;    double aUyItufZoKflauPRrjTW50941799 = -711155577;    double aUyItufZoKflauPRrjTW10522032 = 54896470;    double aUyItufZoKflauPRrjTW1480390 = -104718889;    double aUyItufZoKflauPRrjTW65196879 = 11115810;    double aUyItufZoKflauPRrjTW64418143 = -743869285;    double aUyItufZoKflauPRrjTW25582605 = -297959621;    double aUyItufZoKflauPRrjTW86043870 = -24579230;    double aUyItufZoKflauPRrjTW34137675 = -864310657;    double aUyItufZoKflauPRrjTW29987401 = -385882667;    double aUyItufZoKflauPRrjTW57107742 = -731833897;    double aUyItufZoKflauPRrjTW79109777 = -144931826;    double aUyItufZoKflauPRrjTW24808087 = -799158544;    double aUyItufZoKflauPRrjTW88281493 = -360271062;    double aUyItufZoKflauPRrjTW92015188 = -914489570;    double aUyItufZoKflauPRrjTW79325175 = -138071469;    double aUyItufZoKflauPRrjTW2410488 = -685160998;    double aUyItufZoKflauPRrjTW66687732 = -462361220;    double aUyItufZoKflauPRrjTW91683537 = -454537908;    double aUyItufZoKflauPRrjTW47754272 = -574382328;    double aUyItufZoKflauPRrjTW66030798 = -289375823;    double aUyItufZoKflauPRrjTW64133407 = -339306848;    double aUyItufZoKflauPRrjTW14639984 = -58735639;    double aUyItufZoKflauPRrjTW43552416 = -267577033;    double aUyItufZoKflauPRrjTW43240839 = 2290053;    double aUyItufZoKflauPRrjTW67953165 = -924163636;    double aUyItufZoKflauPRrjTW2051468 = -178646587;    double aUyItufZoKflauPRrjTW75882697 = -281078357;    double aUyItufZoKflauPRrjTW94356061 = -723681022;    double aUyItufZoKflauPRrjTW83627268 = -900517151;    double aUyItufZoKflauPRrjTW86258501 = -715580806;    double aUyItufZoKflauPRrjTW79199272 = -807877913;     aUyItufZoKflauPRrjTW14948679 = aUyItufZoKflauPRrjTW50214793;     aUyItufZoKflauPRrjTW50214793 = aUyItufZoKflauPRrjTW45832408;     aUyItufZoKflauPRrjTW45832408 = aUyItufZoKflauPRrjTW82482460;     aUyItufZoKflauPRrjTW82482460 = aUyItufZoKflauPRrjTW72642130;     aUyItufZoKflauPRrjTW72642130 = aUyItufZoKflauPRrjTW35458463;     aUyItufZoKflauPRrjTW35458463 = aUyItufZoKflauPRrjTW8376558;     aUyItufZoKflauPRrjTW8376558 = aUyItufZoKflauPRrjTW40417176;     aUyItufZoKflauPRrjTW40417176 = aUyItufZoKflauPRrjTW31996464;     aUyItufZoKflauPRrjTW31996464 = aUyItufZoKflauPRrjTW29081930;     aUyItufZoKflauPRrjTW29081930 = aUyItufZoKflauPRrjTW28143371;     aUyItufZoKflauPRrjTW28143371 = aUyItufZoKflauPRrjTW81180815;     aUyItufZoKflauPRrjTW81180815 = aUyItufZoKflauPRrjTW39965112;     aUyItufZoKflauPRrjTW39965112 = aUyItufZoKflauPRrjTW50916633;     aUyItufZoKflauPRrjTW50916633 = aUyItufZoKflauPRrjTW51640823;     aUyItufZoKflauPRrjTW51640823 = aUyItufZoKflauPRrjTW6023305;     aUyItufZoKflauPRrjTW6023305 = aUyItufZoKflauPRrjTW66989470;     aUyItufZoKflauPRrjTW66989470 = aUyItufZoKflauPRrjTW36441258;     aUyItufZoKflauPRrjTW36441258 = aUyItufZoKflauPRrjTW72793035;     aUyItufZoKflauPRrjTW72793035 = aUyItufZoKflauPRrjTW48285008;     aUyItufZoKflauPRrjTW48285008 = aUyItufZoKflauPRrjTW11269614;     aUyItufZoKflauPRrjTW11269614 = aUyItufZoKflauPRrjTW2879756;     aUyItufZoKflauPRrjTW2879756 = aUyItufZoKflauPRrjTW64155753;     aUyItufZoKflauPRrjTW64155753 = aUyItufZoKflauPRrjTW93883550;     aUyItufZoKflauPRrjTW93883550 = aUyItufZoKflauPRrjTW60214219;     aUyItufZoKflauPRrjTW60214219 = aUyItufZoKflauPRrjTW69350917;     aUyItufZoKflauPRrjTW69350917 = aUyItufZoKflauPRrjTW29135038;     aUyItufZoKflauPRrjTW29135038 = aUyItufZoKflauPRrjTW61556851;     aUyItufZoKflauPRrjTW61556851 = aUyItufZoKflauPRrjTW9547495;     aUyItufZoKflauPRrjTW9547495 = aUyItufZoKflauPRrjTW46881650;     aUyItufZoKflauPRrjTW46881650 = aUyItufZoKflauPRrjTW81172078;     aUyItufZoKflauPRrjTW81172078 = aUyItufZoKflauPRrjTW70566833;     aUyItufZoKflauPRrjTW70566833 = aUyItufZoKflauPRrjTW17542019;     aUyItufZoKflauPRrjTW17542019 = aUyItufZoKflauPRrjTW68005825;     aUyItufZoKflauPRrjTW68005825 = aUyItufZoKflauPRrjTW76202330;     aUyItufZoKflauPRrjTW76202330 = aUyItufZoKflauPRrjTW699024;     aUyItufZoKflauPRrjTW699024 = aUyItufZoKflauPRrjTW95501273;     aUyItufZoKflauPRrjTW95501273 = aUyItufZoKflauPRrjTW65509080;     aUyItufZoKflauPRrjTW65509080 = aUyItufZoKflauPRrjTW71244378;     aUyItufZoKflauPRrjTW71244378 = aUyItufZoKflauPRrjTW8374893;     aUyItufZoKflauPRrjTW8374893 = aUyItufZoKflauPRrjTW22702404;     aUyItufZoKflauPRrjTW22702404 = aUyItufZoKflauPRrjTW25225744;     aUyItufZoKflauPRrjTW25225744 = aUyItufZoKflauPRrjTW68742081;     aUyItufZoKflauPRrjTW68742081 = aUyItufZoKflauPRrjTW34168353;     aUyItufZoKflauPRrjTW34168353 = aUyItufZoKflauPRrjTW36775809;     aUyItufZoKflauPRrjTW36775809 = aUyItufZoKflauPRrjTW81104441;     aUyItufZoKflauPRrjTW81104441 = aUyItufZoKflauPRrjTW44542830;     aUyItufZoKflauPRrjTW44542830 = aUyItufZoKflauPRrjTW40853544;     aUyItufZoKflauPRrjTW40853544 = aUyItufZoKflauPRrjTW69541662;     aUyItufZoKflauPRrjTW69541662 = aUyItufZoKflauPRrjTW30222319;     aUyItufZoKflauPRrjTW30222319 = aUyItufZoKflauPRrjTW44471163;     aUyItufZoKflauPRrjTW44471163 = aUyItufZoKflauPRrjTW14484346;     aUyItufZoKflauPRrjTW14484346 = aUyItufZoKflauPRrjTW78883296;     aUyItufZoKflauPRrjTW78883296 = aUyItufZoKflauPRrjTW69787746;     aUyItufZoKflauPRrjTW69787746 = aUyItufZoKflauPRrjTW1975028;     aUyItufZoKflauPRrjTW1975028 = aUyItufZoKflauPRrjTW12068923;     aUyItufZoKflauPRrjTW12068923 = aUyItufZoKflauPRrjTW86059039;     aUyItufZoKflauPRrjTW86059039 = aUyItufZoKflauPRrjTW51948857;     aUyItufZoKflauPRrjTW51948857 = aUyItufZoKflauPRrjTW22268242;     aUyItufZoKflauPRrjTW22268242 = aUyItufZoKflauPRrjTW3291214;     aUyItufZoKflauPRrjTW3291214 = aUyItufZoKflauPRrjTW6323425;     aUyItufZoKflauPRrjTW6323425 = aUyItufZoKflauPRrjTW46819706;     aUyItufZoKflauPRrjTW46819706 = aUyItufZoKflauPRrjTW30869682;     aUyItufZoKflauPRrjTW30869682 = aUyItufZoKflauPRrjTW85114813;     aUyItufZoKflauPRrjTW85114813 = aUyItufZoKflauPRrjTW47909851;     aUyItufZoKflauPRrjTW47909851 = aUyItufZoKflauPRrjTW57576537;     aUyItufZoKflauPRrjTW57576537 = aUyItufZoKflauPRrjTW63638797;     aUyItufZoKflauPRrjTW63638797 = aUyItufZoKflauPRrjTW71959286;     aUyItufZoKflauPRrjTW71959286 = aUyItufZoKflauPRrjTW74714303;     aUyItufZoKflauPRrjTW74714303 = aUyItufZoKflauPRrjTW50941799;     aUyItufZoKflauPRrjTW50941799 = aUyItufZoKflauPRrjTW10522032;     aUyItufZoKflauPRrjTW10522032 = aUyItufZoKflauPRrjTW1480390;     aUyItufZoKflauPRrjTW1480390 = aUyItufZoKflauPRrjTW65196879;     aUyItufZoKflauPRrjTW65196879 = aUyItufZoKflauPRrjTW64418143;     aUyItufZoKflauPRrjTW64418143 = aUyItufZoKflauPRrjTW25582605;     aUyItufZoKflauPRrjTW25582605 = aUyItufZoKflauPRrjTW86043870;     aUyItufZoKflauPRrjTW86043870 = aUyItufZoKflauPRrjTW34137675;     aUyItufZoKflauPRrjTW34137675 = aUyItufZoKflauPRrjTW29987401;     aUyItufZoKflauPRrjTW29987401 = aUyItufZoKflauPRrjTW57107742;     aUyItufZoKflauPRrjTW57107742 = aUyItufZoKflauPRrjTW79109777;     aUyItufZoKflauPRrjTW79109777 = aUyItufZoKflauPRrjTW24808087;     aUyItufZoKflauPRrjTW24808087 = aUyItufZoKflauPRrjTW88281493;     aUyItufZoKflauPRrjTW88281493 = aUyItufZoKflauPRrjTW92015188;     aUyItufZoKflauPRrjTW92015188 = aUyItufZoKflauPRrjTW79325175;     aUyItufZoKflauPRrjTW79325175 = aUyItufZoKflauPRrjTW2410488;     aUyItufZoKflauPRrjTW2410488 = aUyItufZoKflauPRrjTW66687732;     aUyItufZoKflauPRrjTW66687732 = aUyItufZoKflauPRrjTW91683537;     aUyItufZoKflauPRrjTW91683537 = aUyItufZoKflauPRrjTW47754272;     aUyItufZoKflauPRrjTW47754272 = aUyItufZoKflauPRrjTW66030798;     aUyItufZoKflauPRrjTW66030798 = aUyItufZoKflauPRrjTW64133407;     aUyItufZoKflauPRrjTW64133407 = aUyItufZoKflauPRrjTW14639984;     aUyItufZoKflauPRrjTW14639984 = aUyItufZoKflauPRrjTW43552416;     aUyItufZoKflauPRrjTW43552416 = aUyItufZoKflauPRrjTW43240839;     aUyItufZoKflauPRrjTW43240839 = aUyItufZoKflauPRrjTW67953165;     aUyItufZoKflauPRrjTW67953165 = aUyItufZoKflauPRrjTW2051468;     aUyItufZoKflauPRrjTW2051468 = aUyItufZoKflauPRrjTW75882697;     aUyItufZoKflauPRrjTW75882697 = aUyItufZoKflauPRrjTW94356061;     aUyItufZoKflauPRrjTW94356061 = aUyItufZoKflauPRrjTW83627268;     aUyItufZoKflauPRrjTW83627268 = aUyItufZoKflauPRrjTW86258501;     aUyItufZoKflauPRrjTW86258501 = aUyItufZoKflauPRrjTW79199272;     aUyItufZoKflauPRrjTW79199272 = aUyItufZoKflauPRrjTW14948679;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void JRxnOtZcycNjlwyackdK40838898() {     double CFHHvVUUmeJQIWcmMjJq40533956 = -232911014;    double CFHHvVUUmeJQIWcmMjJq46611936 = -457058794;    double CFHHvVUUmeJQIWcmMjJq78711583 = -74929880;    double CFHHvVUUmeJQIWcmMjJq28204391 = -802724398;    double CFHHvVUUmeJQIWcmMjJq41984135 = -830219123;    double CFHHvVUUmeJQIWcmMjJq52467739 = -682082499;    double CFHHvVUUmeJQIWcmMjJq13697402 = 59890927;    double CFHHvVUUmeJQIWcmMjJq55427300 = -121565198;    double CFHHvVUUmeJQIWcmMjJq57325756 = 90032517;    double CFHHvVUUmeJQIWcmMjJq48240312 = -59318789;    double CFHHvVUUmeJQIWcmMjJq42476924 = -453184156;    double CFHHvVUUmeJQIWcmMjJq16498472 = -826920877;    double CFHHvVUUmeJQIWcmMjJq16545890 = -820996097;    double CFHHvVUUmeJQIWcmMjJq42476350 = -499781186;    double CFHHvVUUmeJQIWcmMjJq7909711 = -25231564;    double CFHHvVUUmeJQIWcmMjJq13744348 = -137730234;    double CFHHvVUUmeJQIWcmMjJq66787586 = -85470101;    double CFHHvVUUmeJQIWcmMjJq57557162 = -949287788;    double CFHHvVUUmeJQIWcmMjJq34257756 = -873226281;    double CFHHvVUUmeJQIWcmMjJq9446475 = -80996073;    double CFHHvVUUmeJQIWcmMjJq12278879 = -736876783;    double CFHHvVUUmeJQIWcmMjJq4382863 = -145003155;    double CFHHvVUUmeJQIWcmMjJq67919551 = -978878731;    double CFHHvVUUmeJQIWcmMjJq95597593 = -742459129;    double CFHHvVUUmeJQIWcmMjJq15117358 = -168176176;    double CFHHvVUUmeJQIWcmMjJq98840053 = -927784776;    double CFHHvVUUmeJQIWcmMjJq11522947 = 74821922;    double CFHHvVUUmeJQIWcmMjJq25455652 = -159981380;    double CFHHvVUUmeJQIWcmMjJq17424707 = -550260420;    double CFHHvVUUmeJQIWcmMjJq204142 = -362210583;    double CFHHvVUUmeJQIWcmMjJq3218713 = -870081143;    double CFHHvVUUmeJQIWcmMjJq7756766 = -718181367;    double CFHHvVUUmeJQIWcmMjJq38729071 = -966803250;    double CFHHvVUUmeJQIWcmMjJq21592897 = -199151803;    double CFHHvVUUmeJQIWcmMjJq77607318 = 76649553;    double CFHHvVUUmeJQIWcmMjJq5490874 = -943124746;    double CFHHvVUUmeJQIWcmMjJq45146659 = -136947172;    double CFHHvVUUmeJQIWcmMjJq41081767 = -41146244;    double CFHHvVUUmeJQIWcmMjJq12167958 = -802464560;    double CFHHvVUUmeJQIWcmMjJq17158566 = -816353458;    double CFHHvVUUmeJQIWcmMjJq69853685 = -283657267;    double CFHHvVUUmeJQIWcmMjJq69235334 = -659636276;    double CFHHvVUUmeJQIWcmMjJq17942301 = -989565951;    double CFHHvVUUmeJQIWcmMjJq4961632 = -917319558;    double CFHHvVUUmeJQIWcmMjJq28756685 = 9202630;    double CFHHvVUUmeJQIWcmMjJq47504764 = -878295745;    double CFHHvVUUmeJQIWcmMjJq44311284 = -496644551;    double CFHHvVUUmeJQIWcmMjJq48966315 = -92276729;    double CFHHvVUUmeJQIWcmMjJq20352983 = -115505804;    double CFHHvVUUmeJQIWcmMjJq84032314 = 76081746;    double CFHHvVUUmeJQIWcmMjJq54607410 = 83201374;    double CFHHvVUUmeJQIWcmMjJq41742926 = -707152072;    double CFHHvVUUmeJQIWcmMjJq96956899 = -457746175;    double CFHHvVUUmeJQIWcmMjJq38686655 = -159663567;    double CFHHvVUUmeJQIWcmMjJq28560019 = -228721427;    double CFHHvVUUmeJQIWcmMjJq36151093 = 12092140;    double CFHHvVUUmeJQIWcmMjJq78692385 = -478180063;    double CFHHvVUUmeJQIWcmMjJq83113990 = -332470752;    double CFHHvVUUmeJQIWcmMjJq13087033 = -534548223;    double CFHHvVUUmeJQIWcmMjJq43144082 = -902434348;    double CFHHvVUUmeJQIWcmMjJq40944792 = -656904421;    double CFHHvVUUmeJQIWcmMjJq88241750 = -780127694;    double CFHHvVUUmeJQIWcmMjJq38002594 = -571304779;    double CFHHvVUUmeJQIWcmMjJq57121615 = -547756901;    double CFHHvVUUmeJQIWcmMjJq45021599 = -189237647;    double CFHHvVUUmeJQIWcmMjJq34720159 = -735002789;    double CFHHvVUUmeJQIWcmMjJq77769401 = -860117628;    double CFHHvVUUmeJQIWcmMjJq94952993 = -521844294;    double CFHHvVUUmeJQIWcmMjJq64869031 = -476430739;    double CFHHvVUUmeJQIWcmMjJq2418837 = -82106819;    double CFHHvVUUmeJQIWcmMjJq68597689 = 99216937;    double CFHHvVUUmeJQIWcmMjJq25705819 = 55676142;    double CFHHvVUUmeJQIWcmMjJq45389205 = -46823229;    double CFHHvVUUmeJQIWcmMjJq17099191 = 43127177;    double CFHHvVUUmeJQIWcmMjJq39592790 = -797338807;    double CFHHvVUUmeJQIWcmMjJq43043545 = 22759492;    double CFHHvVUUmeJQIWcmMjJq86440561 = -155437205;    double CFHHvVUUmeJQIWcmMjJq62957919 = 38440827;    double CFHHvVUUmeJQIWcmMjJq66840908 = -651661760;    double CFHHvVUUmeJQIWcmMjJq67612594 = -289880431;    double CFHHvVUUmeJQIWcmMjJq54528769 = -331140225;    double CFHHvVUUmeJQIWcmMjJq62556632 = -832901349;    double CFHHvVUUmeJQIWcmMjJq5102669 = 55524424;    double CFHHvVUUmeJQIWcmMjJq33392393 = -526342166;    double CFHHvVUUmeJQIWcmMjJq45596731 = -345411958;    double CFHHvVUUmeJQIWcmMjJq61475787 = -62929072;    double CFHHvVUUmeJQIWcmMjJq10799866 = -160435193;    double CFHHvVUUmeJQIWcmMjJq42416 = -707139684;    double CFHHvVUUmeJQIWcmMjJq93032877 = -970430377;    double CFHHvVUUmeJQIWcmMjJq41456225 = -935442588;    double CFHHvVUUmeJQIWcmMjJq26798489 = -364944683;    double CFHHvVUUmeJQIWcmMjJq62032668 = -804476421;    double CFHHvVUUmeJQIWcmMjJq27994735 = -506598021;    double CFHHvVUUmeJQIWcmMjJq69023876 = -900030213;    double CFHHvVUUmeJQIWcmMjJq76213774 = -59449038;    double CFHHvVUUmeJQIWcmMjJq81611934 = -503529573;    double CFHHvVUUmeJQIWcmMjJq31232740 = 11668502;    double CFHHvVUUmeJQIWcmMjJq60820686 = -341809050;    double CFHHvVUUmeJQIWcmMjJq59940033 = -628081912;    double CFHHvVUUmeJQIWcmMjJq94036526 = -232911014;     CFHHvVUUmeJQIWcmMjJq40533956 = CFHHvVUUmeJQIWcmMjJq46611936;     CFHHvVUUmeJQIWcmMjJq46611936 = CFHHvVUUmeJQIWcmMjJq78711583;     CFHHvVUUmeJQIWcmMjJq78711583 = CFHHvVUUmeJQIWcmMjJq28204391;     CFHHvVUUmeJQIWcmMjJq28204391 = CFHHvVUUmeJQIWcmMjJq41984135;     CFHHvVUUmeJQIWcmMjJq41984135 = CFHHvVUUmeJQIWcmMjJq52467739;     CFHHvVUUmeJQIWcmMjJq52467739 = CFHHvVUUmeJQIWcmMjJq13697402;     CFHHvVUUmeJQIWcmMjJq13697402 = CFHHvVUUmeJQIWcmMjJq55427300;     CFHHvVUUmeJQIWcmMjJq55427300 = CFHHvVUUmeJQIWcmMjJq57325756;     CFHHvVUUmeJQIWcmMjJq57325756 = CFHHvVUUmeJQIWcmMjJq48240312;     CFHHvVUUmeJQIWcmMjJq48240312 = CFHHvVUUmeJQIWcmMjJq42476924;     CFHHvVUUmeJQIWcmMjJq42476924 = CFHHvVUUmeJQIWcmMjJq16498472;     CFHHvVUUmeJQIWcmMjJq16498472 = CFHHvVUUmeJQIWcmMjJq16545890;     CFHHvVUUmeJQIWcmMjJq16545890 = CFHHvVUUmeJQIWcmMjJq42476350;     CFHHvVUUmeJQIWcmMjJq42476350 = CFHHvVUUmeJQIWcmMjJq7909711;     CFHHvVUUmeJQIWcmMjJq7909711 = CFHHvVUUmeJQIWcmMjJq13744348;     CFHHvVUUmeJQIWcmMjJq13744348 = CFHHvVUUmeJQIWcmMjJq66787586;     CFHHvVUUmeJQIWcmMjJq66787586 = CFHHvVUUmeJQIWcmMjJq57557162;     CFHHvVUUmeJQIWcmMjJq57557162 = CFHHvVUUmeJQIWcmMjJq34257756;     CFHHvVUUmeJQIWcmMjJq34257756 = CFHHvVUUmeJQIWcmMjJq9446475;     CFHHvVUUmeJQIWcmMjJq9446475 = CFHHvVUUmeJQIWcmMjJq12278879;     CFHHvVUUmeJQIWcmMjJq12278879 = CFHHvVUUmeJQIWcmMjJq4382863;     CFHHvVUUmeJQIWcmMjJq4382863 = CFHHvVUUmeJQIWcmMjJq67919551;     CFHHvVUUmeJQIWcmMjJq67919551 = CFHHvVUUmeJQIWcmMjJq95597593;     CFHHvVUUmeJQIWcmMjJq95597593 = CFHHvVUUmeJQIWcmMjJq15117358;     CFHHvVUUmeJQIWcmMjJq15117358 = CFHHvVUUmeJQIWcmMjJq98840053;     CFHHvVUUmeJQIWcmMjJq98840053 = CFHHvVUUmeJQIWcmMjJq11522947;     CFHHvVUUmeJQIWcmMjJq11522947 = CFHHvVUUmeJQIWcmMjJq25455652;     CFHHvVUUmeJQIWcmMjJq25455652 = CFHHvVUUmeJQIWcmMjJq17424707;     CFHHvVUUmeJQIWcmMjJq17424707 = CFHHvVUUmeJQIWcmMjJq204142;     CFHHvVUUmeJQIWcmMjJq204142 = CFHHvVUUmeJQIWcmMjJq3218713;     CFHHvVUUmeJQIWcmMjJq3218713 = CFHHvVUUmeJQIWcmMjJq7756766;     CFHHvVUUmeJQIWcmMjJq7756766 = CFHHvVUUmeJQIWcmMjJq38729071;     CFHHvVUUmeJQIWcmMjJq38729071 = CFHHvVUUmeJQIWcmMjJq21592897;     CFHHvVUUmeJQIWcmMjJq21592897 = CFHHvVUUmeJQIWcmMjJq77607318;     CFHHvVUUmeJQIWcmMjJq77607318 = CFHHvVUUmeJQIWcmMjJq5490874;     CFHHvVUUmeJQIWcmMjJq5490874 = CFHHvVUUmeJQIWcmMjJq45146659;     CFHHvVUUmeJQIWcmMjJq45146659 = CFHHvVUUmeJQIWcmMjJq41081767;     CFHHvVUUmeJQIWcmMjJq41081767 = CFHHvVUUmeJQIWcmMjJq12167958;     CFHHvVUUmeJQIWcmMjJq12167958 = CFHHvVUUmeJQIWcmMjJq17158566;     CFHHvVUUmeJQIWcmMjJq17158566 = CFHHvVUUmeJQIWcmMjJq69853685;     CFHHvVUUmeJQIWcmMjJq69853685 = CFHHvVUUmeJQIWcmMjJq69235334;     CFHHvVUUmeJQIWcmMjJq69235334 = CFHHvVUUmeJQIWcmMjJq17942301;     CFHHvVUUmeJQIWcmMjJq17942301 = CFHHvVUUmeJQIWcmMjJq4961632;     CFHHvVUUmeJQIWcmMjJq4961632 = CFHHvVUUmeJQIWcmMjJq28756685;     CFHHvVUUmeJQIWcmMjJq28756685 = CFHHvVUUmeJQIWcmMjJq47504764;     CFHHvVUUmeJQIWcmMjJq47504764 = CFHHvVUUmeJQIWcmMjJq44311284;     CFHHvVUUmeJQIWcmMjJq44311284 = CFHHvVUUmeJQIWcmMjJq48966315;     CFHHvVUUmeJQIWcmMjJq48966315 = CFHHvVUUmeJQIWcmMjJq20352983;     CFHHvVUUmeJQIWcmMjJq20352983 = CFHHvVUUmeJQIWcmMjJq84032314;     CFHHvVUUmeJQIWcmMjJq84032314 = CFHHvVUUmeJQIWcmMjJq54607410;     CFHHvVUUmeJQIWcmMjJq54607410 = CFHHvVUUmeJQIWcmMjJq41742926;     CFHHvVUUmeJQIWcmMjJq41742926 = CFHHvVUUmeJQIWcmMjJq96956899;     CFHHvVUUmeJQIWcmMjJq96956899 = CFHHvVUUmeJQIWcmMjJq38686655;     CFHHvVUUmeJQIWcmMjJq38686655 = CFHHvVUUmeJQIWcmMjJq28560019;     CFHHvVUUmeJQIWcmMjJq28560019 = CFHHvVUUmeJQIWcmMjJq36151093;     CFHHvVUUmeJQIWcmMjJq36151093 = CFHHvVUUmeJQIWcmMjJq78692385;     CFHHvVUUmeJQIWcmMjJq78692385 = CFHHvVUUmeJQIWcmMjJq83113990;     CFHHvVUUmeJQIWcmMjJq83113990 = CFHHvVUUmeJQIWcmMjJq13087033;     CFHHvVUUmeJQIWcmMjJq13087033 = CFHHvVUUmeJQIWcmMjJq43144082;     CFHHvVUUmeJQIWcmMjJq43144082 = CFHHvVUUmeJQIWcmMjJq40944792;     CFHHvVUUmeJQIWcmMjJq40944792 = CFHHvVUUmeJQIWcmMjJq88241750;     CFHHvVUUmeJQIWcmMjJq88241750 = CFHHvVUUmeJQIWcmMjJq38002594;     CFHHvVUUmeJQIWcmMjJq38002594 = CFHHvVUUmeJQIWcmMjJq57121615;     CFHHvVUUmeJQIWcmMjJq57121615 = CFHHvVUUmeJQIWcmMjJq45021599;     CFHHvVUUmeJQIWcmMjJq45021599 = CFHHvVUUmeJQIWcmMjJq34720159;     CFHHvVUUmeJQIWcmMjJq34720159 = CFHHvVUUmeJQIWcmMjJq77769401;     CFHHvVUUmeJQIWcmMjJq77769401 = CFHHvVUUmeJQIWcmMjJq94952993;     CFHHvVUUmeJQIWcmMjJq94952993 = CFHHvVUUmeJQIWcmMjJq64869031;     CFHHvVUUmeJQIWcmMjJq64869031 = CFHHvVUUmeJQIWcmMjJq2418837;     CFHHvVUUmeJQIWcmMjJq2418837 = CFHHvVUUmeJQIWcmMjJq68597689;     CFHHvVUUmeJQIWcmMjJq68597689 = CFHHvVUUmeJQIWcmMjJq25705819;     CFHHvVUUmeJQIWcmMjJq25705819 = CFHHvVUUmeJQIWcmMjJq45389205;     CFHHvVUUmeJQIWcmMjJq45389205 = CFHHvVUUmeJQIWcmMjJq17099191;     CFHHvVUUmeJQIWcmMjJq17099191 = CFHHvVUUmeJQIWcmMjJq39592790;     CFHHvVUUmeJQIWcmMjJq39592790 = CFHHvVUUmeJQIWcmMjJq43043545;     CFHHvVUUmeJQIWcmMjJq43043545 = CFHHvVUUmeJQIWcmMjJq86440561;     CFHHvVUUmeJQIWcmMjJq86440561 = CFHHvVUUmeJQIWcmMjJq62957919;     CFHHvVUUmeJQIWcmMjJq62957919 = CFHHvVUUmeJQIWcmMjJq66840908;     CFHHvVUUmeJQIWcmMjJq66840908 = CFHHvVUUmeJQIWcmMjJq67612594;     CFHHvVUUmeJQIWcmMjJq67612594 = CFHHvVUUmeJQIWcmMjJq54528769;     CFHHvVUUmeJQIWcmMjJq54528769 = CFHHvVUUmeJQIWcmMjJq62556632;     CFHHvVUUmeJQIWcmMjJq62556632 = CFHHvVUUmeJQIWcmMjJq5102669;     CFHHvVUUmeJQIWcmMjJq5102669 = CFHHvVUUmeJQIWcmMjJq33392393;     CFHHvVUUmeJQIWcmMjJq33392393 = CFHHvVUUmeJQIWcmMjJq45596731;     CFHHvVUUmeJQIWcmMjJq45596731 = CFHHvVUUmeJQIWcmMjJq61475787;     CFHHvVUUmeJQIWcmMjJq61475787 = CFHHvVUUmeJQIWcmMjJq10799866;     CFHHvVUUmeJQIWcmMjJq10799866 = CFHHvVUUmeJQIWcmMjJq42416;     CFHHvVUUmeJQIWcmMjJq42416 = CFHHvVUUmeJQIWcmMjJq93032877;     CFHHvVUUmeJQIWcmMjJq93032877 = CFHHvVUUmeJQIWcmMjJq41456225;     CFHHvVUUmeJQIWcmMjJq41456225 = CFHHvVUUmeJQIWcmMjJq26798489;     CFHHvVUUmeJQIWcmMjJq26798489 = CFHHvVUUmeJQIWcmMjJq62032668;     CFHHvVUUmeJQIWcmMjJq62032668 = CFHHvVUUmeJQIWcmMjJq27994735;     CFHHvVUUmeJQIWcmMjJq27994735 = CFHHvVUUmeJQIWcmMjJq69023876;     CFHHvVUUmeJQIWcmMjJq69023876 = CFHHvVUUmeJQIWcmMjJq76213774;     CFHHvVUUmeJQIWcmMjJq76213774 = CFHHvVUUmeJQIWcmMjJq81611934;     CFHHvVUUmeJQIWcmMjJq81611934 = CFHHvVUUmeJQIWcmMjJq31232740;     CFHHvVUUmeJQIWcmMjJq31232740 = CFHHvVUUmeJQIWcmMjJq60820686;     CFHHvVUUmeJQIWcmMjJq60820686 = CFHHvVUUmeJQIWcmMjJq59940033;     CFHHvVUUmeJQIWcmMjJq59940033 = CFHHvVUUmeJQIWcmMjJq94036526;     CFHHvVUUmeJQIWcmMjJq94036526 = CFHHvVUUmeJQIWcmMjJq40533956;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void znMhqAUkeBOXHsoVTpOZ68433155() {     double nALDNVSXJuKTuvJKMzCP66119232 = -757944115;    double nALDNVSXJuKTuvJKMzCP43009079 = -712536571;    double nALDNVSXJuKTuvJKMzCP11590759 = -977391416;    double nALDNVSXJuKTuvJKMzCP73926320 = -810047292;    double nALDNVSXJuKTuvJKMzCP11326140 = -877141410;    double nALDNVSXJuKTuvJKMzCP69477015 = -837063502;    double nALDNVSXJuKTuvJKMzCP19018247 = -522380280;    double nALDNVSXJuKTuvJKMzCP70437424 = -502247077;    double nALDNVSXJuKTuvJKMzCP82655049 = -406191528;    double nALDNVSXJuKTuvJKMzCP67398694 = -393880313;    double nALDNVSXJuKTuvJKMzCP56810478 = -763612043;    double nALDNVSXJuKTuvJKMzCP51816128 = -170629784;    double nALDNVSXJuKTuvJKMzCP93126667 = -585244462;    double nALDNVSXJuKTuvJKMzCP34036067 = -474305480;    double nALDNVSXJuKTuvJKMzCP64178598 = 86222478;    double nALDNVSXJuKTuvJKMzCP21465391 = -573921712;    double nALDNVSXJuKTuvJKMzCP66585702 = -873789895;    double nALDNVSXJuKTuvJKMzCP78673066 = -365565149;    double nALDNVSXJuKTuvJKMzCP95722477 = -556200138;    double nALDNVSXJuKTuvJKMzCP70607941 = -55435449;    double nALDNVSXJuKTuvJKMzCP13288145 = -98576564;    double nALDNVSXJuKTuvJKMzCP5885970 = -564736084;    double nALDNVSXJuKTuvJKMzCP71683349 = 77029164;    double nALDNVSXJuKTuvJKMzCP97311635 = -601308109;    double nALDNVSXJuKTuvJKMzCP70020497 = -835672318;    double nALDNVSXJuKTuvJKMzCP28329190 = -492235317;    double nALDNVSXJuKTuvJKMzCP93910855 = -590991213;    double nALDNVSXJuKTuvJKMzCP89354451 = -389643612;    double nALDNVSXJuKTuvJKMzCP25301919 = -786554271;    double nALDNVSXJuKTuvJKMzCP53526632 = -271120145;    double nALDNVSXJuKTuvJKMzCP25265348 = -648728174;    double nALDNVSXJuKTuvJKMzCP44946698 = -991977201;    double nALDNVSXJuKTuvJKMzCP59916122 = -314624387;    double nALDNVSXJuKTuvJKMzCP75179968 = -255296775;    double nALDNVSXJuKTuvJKMzCP79012306 = -624786359;    double nALDNVSXJuKTuvJKMzCP10282725 = -360719462;    double nALDNVSXJuKTuvJKMzCP94792044 = -617459117;    double nALDNVSXJuKTuvJKMzCP16654454 = -989861069;    double nALDNVSXJuKTuvJKMzCP53091536 = -160802882;    double nALDNVSXJuKTuvJKMzCP25942240 = -186323777;    double nALDNVSXJuKTuvJKMzCP17004966 = -858717457;    double nALDNVSXJuKTuvJKMzCP13244924 = -68674780;    double nALDNVSXJuKTuvJKMzCP67142520 = 81827668;    double nALDNVSXJuKTuvJKMzCP75754911 = -285735157;    double nALDNVSXJuKTuvJKMzCP20737562 = 70181513;    double nALDNVSXJuKTuvJKMzCP13905086 = -300843283;    double nALDNVSXJuKTuvJKMzCP44079739 = -529113411;    double nALDNVSXJuKTuvJKMzCP57079086 = -285459576;    double nALDNVSXJuKTuvJKMzCP71164302 = -215182031;    double nALDNVSXJuKTuvJKMzCP37842309 = -871941407;    double nALDNVSXJuKTuvJKMzCP64743657 = -165457228;    double nALDNVSXJuKTuvJKMzCP69001505 = -885231251;    double nALDNVSXJuKTuvJKMzCP15030504 = 74355277;    double nALDNVSXJuKTuvJKMzCP7585565 = -474727348;    double nALDNVSXJuKTuvJKMzCP55145010 = -703811844;    double nALDNVSXJuKTuvJKMzCP60233263 = -93208031;    double nALDNVSXJuKTuvJKMzCP71325730 = -689565735;    double nALDNVSXJuKTuvJKMzCP14279124 = -276083308;    double nALDNVSXJuKTuvJKMzCP3905823 = -974374974;    double nALDNVSXJuKTuvJKMzCP82996949 = -284906093;    double nALDNVSXJuKTuvJKMzCP75566159 = -146072290;    double nALDNVSXJuKTuvJKMzCP29663795 = -32736669;    double nALDNVSXJuKTuvJKMzCP45135506 = -715692807;    double nALDNVSXJuKTuvJKMzCP29128417 = -35071384;    double nALDNVSXJuKTuvJKMzCP42133347 = -745152139;    double nALDNVSXJuKTuvJKMzCP11863780 = -771634842;    double nALDNVSXJuKTuvJKMzCP91900005 = -856005397;    double nALDNVSXJuKTuvJKMzCP17946700 = -229947688;    double nALDNVSXJuKTuvJKMzCP55023760 = -849519121;    double nALDNVSXJuKTuvJKMzCP53895874 = -553058060;    double nALDNVSXJuKTuvJKMzCP26673347 = -956462595;    double nALDNVSXJuKTuvJKMzCP49931248 = -883928827;    double nALDNVSXJuKTuvJKMzCP25581531 = -104762268;    double nALDNVSXJuKTuvJKMzCP69780237 = -269876361;    double nALDNVSXJuKTuvJKMzCP53602976 = -196717993;    double nALDNVSXJuKTuvJKMzCP43221 = 70098215;    double nALDNVSXJuKTuvJKMzCP38743449 = -546563753;    double nALDNVSXJuKTuvJKMzCP95928437 = -637235680;    double nALDNVSXJuKTuvJKMzCP76574074 = -571489622;    double nALDNVSXJuKTuvJKMzCP56115412 = -434829036;    double nALDNVSXJuKTuvJKMzCP84249451 = -963121906;    double nALDNVSXJuKTuvJKMzCP36831770 = -205531637;    double nALDNVSXJuKTuvJKMzCP18190149 = -74461582;    double nALDNVSXJuKTuvJKMzCP87459610 = -914612864;    double nALDNVSXJuKTuvJKMzCP88782974 = -5662917;    double nALDNVSXJuKTuvJKMzCP56263842 = -763496923;    double nALDNVSXJuKTuvJKMzCP29916195 = -966332478;    double nALDNVSXJuKTuvJKMzCP52330558 = -839897039;    double nALDNVSXJuKTuvJKMzCP20034958 = -551484931;    double nALDNVSXJuKTuvJKMzCP18779043 = -431578328;    double nALDNVSXJuKTuvJKMzCP38956994 = -671153728;    double nALDNVSXJuKTuvJKMzCP80512920 = -241375809;    double nALDNVSXJuKTuvJKMzCP12748632 = 84513905;    double nALDNVSXJuKTuvJKMzCP70094586 = -875896789;    double nALDNVSXJuKTuvJKMzCP50376080 = 59748512;    double nALDNVSXJuKTuvJKMzCP87341170 = -725980788;    double nALDNVSXJuKTuvJKMzCP68109418 = -352981973;    double nALDNVSXJuKTuvJKMzCP38014104 = -883100949;    double nALDNVSXJuKTuvJKMzCP33621564 = -540583018;    double nALDNVSXJuKTuvJKMzCP8873782 = -757944115;     nALDNVSXJuKTuvJKMzCP66119232 = nALDNVSXJuKTuvJKMzCP43009079;     nALDNVSXJuKTuvJKMzCP43009079 = nALDNVSXJuKTuvJKMzCP11590759;     nALDNVSXJuKTuvJKMzCP11590759 = nALDNVSXJuKTuvJKMzCP73926320;     nALDNVSXJuKTuvJKMzCP73926320 = nALDNVSXJuKTuvJKMzCP11326140;     nALDNVSXJuKTuvJKMzCP11326140 = nALDNVSXJuKTuvJKMzCP69477015;     nALDNVSXJuKTuvJKMzCP69477015 = nALDNVSXJuKTuvJKMzCP19018247;     nALDNVSXJuKTuvJKMzCP19018247 = nALDNVSXJuKTuvJKMzCP70437424;     nALDNVSXJuKTuvJKMzCP70437424 = nALDNVSXJuKTuvJKMzCP82655049;     nALDNVSXJuKTuvJKMzCP82655049 = nALDNVSXJuKTuvJKMzCP67398694;     nALDNVSXJuKTuvJKMzCP67398694 = nALDNVSXJuKTuvJKMzCP56810478;     nALDNVSXJuKTuvJKMzCP56810478 = nALDNVSXJuKTuvJKMzCP51816128;     nALDNVSXJuKTuvJKMzCP51816128 = nALDNVSXJuKTuvJKMzCP93126667;     nALDNVSXJuKTuvJKMzCP93126667 = nALDNVSXJuKTuvJKMzCP34036067;     nALDNVSXJuKTuvJKMzCP34036067 = nALDNVSXJuKTuvJKMzCP64178598;     nALDNVSXJuKTuvJKMzCP64178598 = nALDNVSXJuKTuvJKMzCP21465391;     nALDNVSXJuKTuvJKMzCP21465391 = nALDNVSXJuKTuvJKMzCP66585702;     nALDNVSXJuKTuvJKMzCP66585702 = nALDNVSXJuKTuvJKMzCP78673066;     nALDNVSXJuKTuvJKMzCP78673066 = nALDNVSXJuKTuvJKMzCP95722477;     nALDNVSXJuKTuvJKMzCP95722477 = nALDNVSXJuKTuvJKMzCP70607941;     nALDNVSXJuKTuvJKMzCP70607941 = nALDNVSXJuKTuvJKMzCP13288145;     nALDNVSXJuKTuvJKMzCP13288145 = nALDNVSXJuKTuvJKMzCP5885970;     nALDNVSXJuKTuvJKMzCP5885970 = nALDNVSXJuKTuvJKMzCP71683349;     nALDNVSXJuKTuvJKMzCP71683349 = nALDNVSXJuKTuvJKMzCP97311635;     nALDNVSXJuKTuvJKMzCP97311635 = nALDNVSXJuKTuvJKMzCP70020497;     nALDNVSXJuKTuvJKMzCP70020497 = nALDNVSXJuKTuvJKMzCP28329190;     nALDNVSXJuKTuvJKMzCP28329190 = nALDNVSXJuKTuvJKMzCP93910855;     nALDNVSXJuKTuvJKMzCP93910855 = nALDNVSXJuKTuvJKMzCP89354451;     nALDNVSXJuKTuvJKMzCP89354451 = nALDNVSXJuKTuvJKMzCP25301919;     nALDNVSXJuKTuvJKMzCP25301919 = nALDNVSXJuKTuvJKMzCP53526632;     nALDNVSXJuKTuvJKMzCP53526632 = nALDNVSXJuKTuvJKMzCP25265348;     nALDNVSXJuKTuvJKMzCP25265348 = nALDNVSXJuKTuvJKMzCP44946698;     nALDNVSXJuKTuvJKMzCP44946698 = nALDNVSXJuKTuvJKMzCP59916122;     nALDNVSXJuKTuvJKMzCP59916122 = nALDNVSXJuKTuvJKMzCP75179968;     nALDNVSXJuKTuvJKMzCP75179968 = nALDNVSXJuKTuvJKMzCP79012306;     nALDNVSXJuKTuvJKMzCP79012306 = nALDNVSXJuKTuvJKMzCP10282725;     nALDNVSXJuKTuvJKMzCP10282725 = nALDNVSXJuKTuvJKMzCP94792044;     nALDNVSXJuKTuvJKMzCP94792044 = nALDNVSXJuKTuvJKMzCP16654454;     nALDNVSXJuKTuvJKMzCP16654454 = nALDNVSXJuKTuvJKMzCP53091536;     nALDNVSXJuKTuvJKMzCP53091536 = nALDNVSXJuKTuvJKMzCP25942240;     nALDNVSXJuKTuvJKMzCP25942240 = nALDNVSXJuKTuvJKMzCP17004966;     nALDNVSXJuKTuvJKMzCP17004966 = nALDNVSXJuKTuvJKMzCP13244924;     nALDNVSXJuKTuvJKMzCP13244924 = nALDNVSXJuKTuvJKMzCP67142520;     nALDNVSXJuKTuvJKMzCP67142520 = nALDNVSXJuKTuvJKMzCP75754911;     nALDNVSXJuKTuvJKMzCP75754911 = nALDNVSXJuKTuvJKMzCP20737562;     nALDNVSXJuKTuvJKMzCP20737562 = nALDNVSXJuKTuvJKMzCP13905086;     nALDNVSXJuKTuvJKMzCP13905086 = nALDNVSXJuKTuvJKMzCP44079739;     nALDNVSXJuKTuvJKMzCP44079739 = nALDNVSXJuKTuvJKMzCP57079086;     nALDNVSXJuKTuvJKMzCP57079086 = nALDNVSXJuKTuvJKMzCP71164302;     nALDNVSXJuKTuvJKMzCP71164302 = nALDNVSXJuKTuvJKMzCP37842309;     nALDNVSXJuKTuvJKMzCP37842309 = nALDNVSXJuKTuvJKMzCP64743657;     nALDNVSXJuKTuvJKMzCP64743657 = nALDNVSXJuKTuvJKMzCP69001505;     nALDNVSXJuKTuvJKMzCP69001505 = nALDNVSXJuKTuvJKMzCP15030504;     nALDNVSXJuKTuvJKMzCP15030504 = nALDNVSXJuKTuvJKMzCP7585565;     nALDNVSXJuKTuvJKMzCP7585565 = nALDNVSXJuKTuvJKMzCP55145010;     nALDNVSXJuKTuvJKMzCP55145010 = nALDNVSXJuKTuvJKMzCP60233263;     nALDNVSXJuKTuvJKMzCP60233263 = nALDNVSXJuKTuvJKMzCP71325730;     nALDNVSXJuKTuvJKMzCP71325730 = nALDNVSXJuKTuvJKMzCP14279124;     nALDNVSXJuKTuvJKMzCP14279124 = nALDNVSXJuKTuvJKMzCP3905823;     nALDNVSXJuKTuvJKMzCP3905823 = nALDNVSXJuKTuvJKMzCP82996949;     nALDNVSXJuKTuvJKMzCP82996949 = nALDNVSXJuKTuvJKMzCP75566159;     nALDNVSXJuKTuvJKMzCP75566159 = nALDNVSXJuKTuvJKMzCP29663795;     nALDNVSXJuKTuvJKMzCP29663795 = nALDNVSXJuKTuvJKMzCP45135506;     nALDNVSXJuKTuvJKMzCP45135506 = nALDNVSXJuKTuvJKMzCP29128417;     nALDNVSXJuKTuvJKMzCP29128417 = nALDNVSXJuKTuvJKMzCP42133347;     nALDNVSXJuKTuvJKMzCP42133347 = nALDNVSXJuKTuvJKMzCP11863780;     nALDNVSXJuKTuvJKMzCP11863780 = nALDNVSXJuKTuvJKMzCP91900005;     nALDNVSXJuKTuvJKMzCP91900005 = nALDNVSXJuKTuvJKMzCP17946700;     nALDNVSXJuKTuvJKMzCP17946700 = nALDNVSXJuKTuvJKMzCP55023760;     nALDNVSXJuKTuvJKMzCP55023760 = nALDNVSXJuKTuvJKMzCP53895874;     nALDNVSXJuKTuvJKMzCP53895874 = nALDNVSXJuKTuvJKMzCP26673347;     nALDNVSXJuKTuvJKMzCP26673347 = nALDNVSXJuKTuvJKMzCP49931248;     nALDNVSXJuKTuvJKMzCP49931248 = nALDNVSXJuKTuvJKMzCP25581531;     nALDNVSXJuKTuvJKMzCP25581531 = nALDNVSXJuKTuvJKMzCP69780237;     nALDNVSXJuKTuvJKMzCP69780237 = nALDNVSXJuKTuvJKMzCP53602976;     nALDNVSXJuKTuvJKMzCP53602976 = nALDNVSXJuKTuvJKMzCP43221;     nALDNVSXJuKTuvJKMzCP43221 = nALDNVSXJuKTuvJKMzCP38743449;     nALDNVSXJuKTuvJKMzCP38743449 = nALDNVSXJuKTuvJKMzCP95928437;     nALDNVSXJuKTuvJKMzCP95928437 = nALDNVSXJuKTuvJKMzCP76574074;     nALDNVSXJuKTuvJKMzCP76574074 = nALDNVSXJuKTuvJKMzCP56115412;     nALDNVSXJuKTuvJKMzCP56115412 = nALDNVSXJuKTuvJKMzCP84249451;     nALDNVSXJuKTuvJKMzCP84249451 = nALDNVSXJuKTuvJKMzCP36831770;     nALDNVSXJuKTuvJKMzCP36831770 = nALDNVSXJuKTuvJKMzCP18190149;     nALDNVSXJuKTuvJKMzCP18190149 = nALDNVSXJuKTuvJKMzCP87459610;     nALDNVSXJuKTuvJKMzCP87459610 = nALDNVSXJuKTuvJKMzCP88782974;     nALDNVSXJuKTuvJKMzCP88782974 = nALDNVSXJuKTuvJKMzCP56263842;     nALDNVSXJuKTuvJKMzCP56263842 = nALDNVSXJuKTuvJKMzCP29916195;     nALDNVSXJuKTuvJKMzCP29916195 = nALDNVSXJuKTuvJKMzCP52330558;     nALDNVSXJuKTuvJKMzCP52330558 = nALDNVSXJuKTuvJKMzCP20034958;     nALDNVSXJuKTuvJKMzCP20034958 = nALDNVSXJuKTuvJKMzCP18779043;     nALDNVSXJuKTuvJKMzCP18779043 = nALDNVSXJuKTuvJKMzCP38956994;     nALDNVSXJuKTuvJKMzCP38956994 = nALDNVSXJuKTuvJKMzCP80512920;     nALDNVSXJuKTuvJKMzCP80512920 = nALDNVSXJuKTuvJKMzCP12748632;     nALDNVSXJuKTuvJKMzCP12748632 = nALDNVSXJuKTuvJKMzCP70094586;     nALDNVSXJuKTuvJKMzCP70094586 = nALDNVSXJuKTuvJKMzCP50376080;     nALDNVSXJuKTuvJKMzCP50376080 = nALDNVSXJuKTuvJKMzCP87341170;     nALDNVSXJuKTuvJKMzCP87341170 = nALDNVSXJuKTuvJKMzCP68109418;     nALDNVSXJuKTuvJKMzCP68109418 = nALDNVSXJuKTuvJKMzCP38014104;     nALDNVSXJuKTuvJKMzCP38014104 = nALDNVSXJuKTuvJKMzCP33621564;     nALDNVSXJuKTuvJKMzCP33621564 = nALDNVSXJuKTuvJKMzCP8873782;     nALDNVSXJuKTuvJKMzCP8873782 = nALDNVSXJuKTuvJKMzCP66119232;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void zmexGriwjTXLwjyMGoOd96027411() {     double rHkeIcOjCwwCenzJBWDT91704509 = -182977215;    double rHkeIcOjCwwCenzJBWDT39406222 = -968014348;    double rHkeIcOjCwwCenzJBWDT44469935 = -779852953;    double rHkeIcOjCwwCenzJBWDT19648250 = -817370185;    double rHkeIcOjCwwCenzJBWDT80668144 = -924063697;    double rHkeIcOjCwwCenzJBWDT86486292 = -992044505;    double rHkeIcOjCwwCenzJBWDT24339091 = -4651487;    double rHkeIcOjCwwCenzJBWDT85447548 = -882928956;    double rHkeIcOjCwwCenzJBWDT7984342 = -902415573;    double rHkeIcOjCwwCenzJBWDT86557076 = -728441836;    double rHkeIcOjCwwCenzJBWDT71144032 = 25960071;    double rHkeIcOjCwwCenzJBWDT87133783 = -614338691;    double rHkeIcOjCwwCenzJBWDT69707445 = -349492828;    double rHkeIcOjCwwCenzJBWDT25595784 = -448829773;    double rHkeIcOjCwwCenzJBWDT20447487 = -902323480;    double rHkeIcOjCwwCenzJBWDT29186434 = 89886811;    double rHkeIcOjCwwCenzJBWDT66383819 = -562109689;    double rHkeIcOjCwwCenzJBWDT99788970 = -881842510;    double rHkeIcOjCwwCenzJBWDT57187198 = -239173996;    double rHkeIcOjCwwCenzJBWDT31769408 = -29874824;    double rHkeIcOjCwwCenzJBWDT14297410 = -560276345;    double rHkeIcOjCwwCenzJBWDT7389076 = -984469014;    double rHkeIcOjCwwCenzJBWDT75447146 = 32937058;    double rHkeIcOjCwwCenzJBWDT99025678 = -460157088;    double rHkeIcOjCwwCenzJBWDT24923637 = -403168460;    double rHkeIcOjCwwCenzJBWDT57818327 = -56685859;    double rHkeIcOjCwwCenzJBWDT76298765 = -156804347;    double rHkeIcOjCwwCenzJBWDT53253252 = -619305845;    double rHkeIcOjCwwCenzJBWDT33179132 = 77151879;    double rHkeIcOjCwwCenzJBWDT6849123 = -180029707;    double rHkeIcOjCwwCenzJBWDT47311982 = -427375205;    double rHkeIcOjCwwCenzJBWDT82136630 = -165773035;    double rHkeIcOjCwwCenzJBWDT81103174 = -762445524;    double rHkeIcOjCwwCenzJBWDT28767039 = -311441746;    double rHkeIcOjCwwCenzJBWDT80417294 = -226222270;    double rHkeIcOjCwwCenzJBWDT15074575 = -878314178;    double rHkeIcOjCwwCenzJBWDT44437429 = 2028938;    double rHkeIcOjCwwCenzJBWDT92227140 = -838575894;    double rHkeIcOjCwwCenzJBWDT94015114 = -619141204;    double rHkeIcOjCwwCenzJBWDT34725914 = -656294096;    double rHkeIcOjCwwCenzJBWDT64156247 = -333777646;    double rHkeIcOjCwwCenzJBWDT57254514 = -577713284;    double rHkeIcOjCwwCenzJBWDT16342740 = 53221287;    double rHkeIcOjCwwCenzJBWDT46548190 = -754150756;    double rHkeIcOjCwwCenzJBWDT12718438 = -968839605;    double rHkeIcOjCwwCenzJBWDT80305408 = -823390820;    double rHkeIcOjCwwCenzJBWDT43848193 = -561582272;    double rHkeIcOjCwwCenzJBWDT65191857 = -478642422;    double rHkeIcOjCwwCenzJBWDT21975623 = -314858257;    double rHkeIcOjCwwCenzJBWDT91652304 = -719964561;    double rHkeIcOjCwwCenzJBWDT74879905 = -414115830;    double rHkeIcOjCwwCenzJBWDT96260084 = 36689569;    double rHkeIcOjCwwCenzJBWDT33104107 = -493543272;    double rHkeIcOjCwwCenzJBWDT76484473 = -789791130;    double rHkeIcOjCwwCenzJBWDT81730002 = -78902261;    double rHkeIcOjCwwCenzJBWDT84315433 = -198508202;    double rHkeIcOjCwwCenzJBWDT63959076 = -900951407;    double rHkeIcOjCwwCenzJBWDT45444257 = -219695865;    double rHkeIcOjCwwCenzJBWDT94724613 = -314201725;    double rHkeIcOjCwwCenzJBWDT22849818 = -767377839;    double rHkeIcOjCwwCenzJBWDT10187528 = -735240158;    double rHkeIcOjCwwCenzJBWDT71085839 = -385345643;    double rHkeIcOjCwwCenzJBWDT52268417 = -860080836;    double rHkeIcOjCwwCenzJBWDT1135219 = -622385867;    double rHkeIcOjCwwCenzJBWDT39245095 = -201066632;    double rHkeIcOjCwwCenzJBWDT89007401 = -808266895;    double rHkeIcOjCwwCenzJBWDT6030610 = -851893167;    double rHkeIcOjCwwCenzJBWDT40940406 = 61948918;    double rHkeIcOjCwwCenzJBWDT45178489 = -122607504;    double rHkeIcOjCwwCenzJBWDT5372912 = 75990698;    double rHkeIcOjCwwCenzJBWDT84749004 = -912142128;    double rHkeIcOjCwwCenzJBWDT74156678 = -723533796;    double rHkeIcOjCwwCenzJBWDT5773857 = -162701307;    double rHkeIcOjCwwCenzJBWDT22461285 = -582879900;    double rHkeIcOjCwwCenzJBWDT67613161 = -696097179;    double rHkeIcOjCwwCenzJBWDT57042896 = -982563062;    double rHkeIcOjCwwCenzJBWDT91046336 = -937690301;    double rHkeIcOjCwwCenzJBWDT28898957 = -212912186;    double rHkeIcOjCwwCenzJBWDT86307240 = -491317484;    double rHkeIcOjCwwCenzJBWDT44618229 = -579777640;    double rHkeIcOjCwwCenzJBWDT13970134 = -495103588;    double rHkeIcOjCwwCenzJBWDT11106908 = -678161925;    double rHkeIcOjCwwCenzJBWDT31277629 = -204447588;    double rHkeIcOjCwwCenzJBWDT41526827 = -202883561;    double rHkeIcOjCwwCenzJBWDT31969218 = -765913877;    double rHkeIcOjCwwCenzJBWDT51051897 = -364064774;    double rHkeIcOjCwwCenzJBWDT49032524 = -672229763;    double rHkeIcOjCwwCenzJBWDT4618702 = -972654395;    double rHkeIcOjCwwCenzJBWDT47037037 = -132539485;    double rHkeIcOjCwwCenzJBWDT96101860 = 72285932;    double rHkeIcOjCwwCenzJBWDT51115499 = -977362772;    double rHkeIcOjCwwCenzJBWDT98993172 = -778275197;    double rHkeIcOjCwwCenzJBWDT97502527 = -424374169;    double rHkeIcOjCwwCenzJBWDT71165296 = -851763365;    double rHkeIcOjCwwCenzJBWDT24538387 = -921053939;    double rHkeIcOjCwwCenzJBWDT93070407 = -948432004;    double rHkeIcOjCwwCenzJBWDT4986097 = -717632449;    double rHkeIcOjCwwCenzJBWDT15207522 = -324392847;    double rHkeIcOjCwwCenzJBWDT7303096 = -453084125;    double rHkeIcOjCwwCenzJBWDT23711036 = -182977215;     rHkeIcOjCwwCenzJBWDT91704509 = rHkeIcOjCwwCenzJBWDT39406222;     rHkeIcOjCwwCenzJBWDT39406222 = rHkeIcOjCwwCenzJBWDT44469935;     rHkeIcOjCwwCenzJBWDT44469935 = rHkeIcOjCwwCenzJBWDT19648250;     rHkeIcOjCwwCenzJBWDT19648250 = rHkeIcOjCwwCenzJBWDT80668144;     rHkeIcOjCwwCenzJBWDT80668144 = rHkeIcOjCwwCenzJBWDT86486292;     rHkeIcOjCwwCenzJBWDT86486292 = rHkeIcOjCwwCenzJBWDT24339091;     rHkeIcOjCwwCenzJBWDT24339091 = rHkeIcOjCwwCenzJBWDT85447548;     rHkeIcOjCwwCenzJBWDT85447548 = rHkeIcOjCwwCenzJBWDT7984342;     rHkeIcOjCwwCenzJBWDT7984342 = rHkeIcOjCwwCenzJBWDT86557076;     rHkeIcOjCwwCenzJBWDT86557076 = rHkeIcOjCwwCenzJBWDT71144032;     rHkeIcOjCwwCenzJBWDT71144032 = rHkeIcOjCwwCenzJBWDT87133783;     rHkeIcOjCwwCenzJBWDT87133783 = rHkeIcOjCwwCenzJBWDT69707445;     rHkeIcOjCwwCenzJBWDT69707445 = rHkeIcOjCwwCenzJBWDT25595784;     rHkeIcOjCwwCenzJBWDT25595784 = rHkeIcOjCwwCenzJBWDT20447487;     rHkeIcOjCwwCenzJBWDT20447487 = rHkeIcOjCwwCenzJBWDT29186434;     rHkeIcOjCwwCenzJBWDT29186434 = rHkeIcOjCwwCenzJBWDT66383819;     rHkeIcOjCwwCenzJBWDT66383819 = rHkeIcOjCwwCenzJBWDT99788970;     rHkeIcOjCwwCenzJBWDT99788970 = rHkeIcOjCwwCenzJBWDT57187198;     rHkeIcOjCwwCenzJBWDT57187198 = rHkeIcOjCwwCenzJBWDT31769408;     rHkeIcOjCwwCenzJBWDT31769408 = rHkeIcOjCwwCenzJBWDT14297410;     rHkeIcOjCwwCenzJBWDT14297410 = rHkeIcOjCwwCenzJBWDT7389076;     rHkeIcOjCwwCenzJBWDT7389076 = rHkeIcOjCwwCenzJBWDT75447146;     rHkeIcOjCwwCenzJBWDT75447146 = rHkeIcOjCwwCenzJBWDT99025678;     rHkeIcOjCwwCenzJBWDT99025678 = rHkeIcOjCwwCenzJBWDT24923637;     rHkeIcOjCwwCenzJBWDT24923637 = rHkeIcOjCwwCenzJBWDT57818327;     rHkeIcOjCwwCenzJBWDT57818327 = rHkeIcOjCwwCenzJBWDT76298765;     rHkeIcOjCwwCenzJBWDT76298765 = rHkeIcOjCwwCenzJBWDT53253252;     rHkeIcOjCwwCenzJBWDT53253252 = rHkeIcOjCwwCenzJBWDT33179132;     rHkeIcOjCwwCenzJBWDT33179132 = rHkeIcOjCwwCenzJBWDT6849123;     rHkeIcOjCwwCenzJBWDT6849123 = rHkeIcOjCwwCenzJBWDT47311982;     rHkeIcOjCwwCenzJBWDT47311982 = rHkeIcOjCwwCenzJBWDT82136630;     rHkeIcOjCwwCenzJBWDT82136630 = rHkeIcOjCwwCenzJBWDT81103174;     rHkeIcOjCwwCenzJBWDT81103174 = rHkeIcOjCwwCenzJBWDT28767039;     rHkeIcOjCwwCenzJBWDT28767039 = rHkeIcOjCwwCenzJBWDT80417294;     rHkeIcOjCwwCenzJBWDT80417294 = rHkeIcOjCwwCenzJBWDT15074575;     rHkeIcOjCwwCenzJBWDT15074575 = rHkeIcOjCwwCenzJBWDT44437429;     rHkeIcOjCwwCenzJBWDT44437429 = rHkeIcOjCwwCenzJBWDT92227140;     rHkeIcOjCwwCenzJBWDT92227140 = rHkeIcOjCwwCenzJBWDT94015114;     rHkeIcOjCwwCenzJBWDT94015114 = rHkeIcOjCwwCenzJBWDT34725914;     rHkeIcOjCwwCenzJBWDT34725914 = rHkeIcOjCwwCenzJBWDT64156247;     rHkeIcOjCwwCenzJBWDT64156247 = rHkeIcOjCwwCenzJBWDT57254514;     rHkeIcOjCwwCenzJBWDT57254514 = rHkeIcOjCwwCenzJBWDT16342740;     rHkeIcOjCwwCenzJBWDT16342740 = rHkeIcOjCwwCenzJBWDT46548190;     rHkeIcOjCwwCenzJBWDT46548190 = rHkeIcOjCwwCenzJBWDT12718438;     rHkeIcOjCwwCenzJBWDT12718438 = rHkeIcOjCwwCenzJBWDT80305408;     rHkeIcOjCwwCenzJBWDT80305408 = rHkeIcOjCwwCenzJBWDT43848193;     rHkeIcOjCwwCenzJBWDT43848193 = rHkeIcOjCwwCenzJBWDT65191857;     rHkeIcOjCwwCenzJBWDT65191857 = rHkeIcOjCwwCenzJBWDT21975623;     rHkeIcOjCwwCenzJBWDT21975623 = rHkeIcOjCwwCenzJBWDT91652304;     rHkeIcOjCwwCenzJBWDT91652304 = rHkeIcOjCwwCenzJBWDT74879905;     rHkeIcOjCwwCenzJBWDT74879905 = rHkeIcOjCwwCenzJBWDT96260084;     rHkeIcOjCwwCenzJBWDT96260084 = rHkeIcOjCwwCenzJBWDT33104107;     rHkeIcOjCwwCenzJBWDT33104107 = rHkeIcOjCwwCenzJBWDT76484473;     rHkeIcOjCwwCenzJBWDT76484473 = rHkeIcOjCwwCenzJBWDT81730002;     rHkeIcOjCwwCenzJBWDT81730002 = rHkeIcOjCwwCenzJBWDT84315433;     rHkeIcOjCwwCenzJBWDT84315433 = rHkeIcOjCwwCenzJBWDT63959076;     rHkeIcOjCwwCenzJBWDT63959076 = rHkeIcOjCwwCenzJBWDT45444257;     rHkeIcOjCwwCenzJBWDT45444257 = rHkeIcOjCwwCenzJBWDT94724613;     rHkeIcOjCwwCenzJBWDT94724613 = rHkeIcOjCwwCenzJBWDT22849818;     rHkeIcOjCwwCenzJBWDT22849818 = rHkeIcOjCwwCenzJBWDT10187528;     rHkeIcOjCwwCenzJBWDT10187528 = rHkeIcOjCwwCenzJBWDT71085839;     rHkeIcOjCwwCenzJBWDT71085839 = rHkeIcOjCwwCenzJBWDT52268417;     rHkeIcOjCwwCenzJBWDT52268417 = rHkeIcOjCwwCenzJBWDT1135219;     rHkeIcOjCwwCenzJBWDT1135219 = rHkeIcOjCwwCenzJBWDT39245095;     rHkeIcOjCwwCenzJBWDT39245095 = rHkeIcOjCwwCenzJBWDT89007401;     rHkeIcOjCwwCenzJBWDT89007401 = rHkeIcOjCwwCenzJBWDT6030610;     rHkeIcOjCwwCenzJBWDT6030610 = rHkeIcOjCwwCenzJBWDT40940406;     rHkeIcOjCwwCenzJBWDT40940406 = rHkeIcOjCwwCenzJBWDT45178489;     rHkeIcOjCwwCenzJBWDT45178489 = rHkeIcOjCwwCenzJBWDT5372912;     rHkeIcOjCwwCenzJBWDT5372912 = rHkeIcOjCwwCenzJBWDT84749004;     rHkeIcOjCwwCenzJBWDT84749004 = rHkeIcOjCwwCenzJBWDT74156678;     rHkeIcOjCwwCenzJBWDT74156678 = rHkeIcOjCwwCenzJBWDT5773857;     rHkeIcOjCwwCenzJBWDT5773857 = rHkeIcOjCwwCenzJBWDT22461285;     rHkeIcOjCwwCenzJBWDT22461285 = rHkeIcOjCwwCenzJBWDT67613161;     rHkeIcOjCwwCenzJBWDT67613161 = rHkeIcOjCwwCenzJBWDT57042896;     rHkeIcOjCwwCenzJBWDT57042896 = rHkeIcOjCwwCenzJBWDT91046336;     rHkeIcOjCwwCenzJBWDT91046336 = rHkeIcOjCwwCenzJBWDT28898957;     rHkeIcOjCwwCenzJBWDT28898957 = rHkeIcOjCwwCenzJBWDT86307240;     rHkeIcOjCwwCenzJBWDT86307240 = rHkeIcOjCwwCenzJBWDT44618229;     rHkeIcOjCwwCenzJBWDT44618229 = rHkeIcOjCwwCenzJBWDT13970134;     rHkeIcOjCwwCenzJBWDT13970134 = rHkeIcOjCwwCenzJBWDT11106908;     rHkeIcOjCwwCenzJBWDT11106908 = rHkeIcOjCwwCenzJBWDT31277629;     rHkeIcOjCwwCenzJBWDT31277629 = rHkeIcOjCwwCenzJBWDT41526827;     rHkeIcOjCwwCenzJBWDT41526827 = rHkeIcOjCwwCenzJBWDT31969218;     rHkeIcOjCwwCenzJBWDT31969218 = rHkeIcOjCwwCenzJBWDT51051897;     rHkeIcOjCwwCenzJBWDT51051897 = rHkeIcOjCwwCenzJBWDT49032524;     rHkeIcOjCwwCenzJBWDT49032524 = rHkeIcOjCwwCenzJBWDT4618702;     rHkeIcOjCwwCenzJBWDT4618702 = rHkeIcOjCwwCenzJBWDT47037037;     rHkeIcOjCwwCenzJBWDT47037037 = rHkeIcOjCwwCenzJBWDT96101860;     rHkeIcOjCwwCenzJBWDT96101860 = rHkeIcOjCwwCenzJBWDT51115499;     rHkeIcOjCwwCenzJBWDT51115499 = rHkeIcOjCwwCenzJBWDT98993172;     rHkeIcOjCwwCenzJBWDT98993172 = rHkeIcOjCwwCenzJBWDT97502527;     rHkeIcOjCwwCenzJBWDT97502527 = rHkeIcOjCwwCenzJBWDT71165296;     rHkeIcOjCwwCenzJBWDT71165296 = rHkeIcOjCwwCenzJBWDT24538387;     rHkeIcOjCwwCenzJBWDT24538387 = rHkeIcOjCwwCenzJBWDT93070407;     rHkeIcOjCwwCenzJBWDT93070407 = rHkeIcOjCwwCenzJBWDT4986097;     rHkeIcOjCwwCenzJBWDT4986097 = rHkeIcOjCwwCenzJBWDT15207522;     rHkeIcOjCwwCenzJBWDT15207522 = rHkeIcOjCwwCenzJBWDT7303096;     rHkeIcOjCwwCenzJBWDT7303096 = rHkeIcOjCwwCenzJBWDT23711036;     rHkeIcOjCwwCenzJBWDT23711036 = rHkeIcOjCwwCenzJBWDT91704509;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void cgYQypUGSHJrZRzzUyuG23621669() {     double pLIXSuUsqQXBvGCCyUUd17289787 = -708010316;    double pLIXSuUsqQXBvGCCyUUd35803366 = -123492125;    double pLIXSuUsqQXBvGCCyUUd77349110 = -582314489;    double pLIXSuUsqQXBvGCCyUUd65370179 = -824693078;    double pLIXSuUsqQXBvGCCyUUd50010149 = -970985984;    double pLIXSuUsqQXBvGCCyUUd3495569 = -47025509;    double pLIXSuUsqQXBvGCCyUUd29659936 = -586922694;    double pLIXSuUsqQXBvGCCyUUd457674 = -163610836;    double pLIXSuUsqQXBvGCCyUUd33313634 = -298639618;    double pLIXSuUsqQXBvGCCyUUd5715460 = 36996640;    double pLIXSuUsqQXBvGCCyUUd85477586 = -284467816;    double pLIXSuUsqQXBvGCCyUUd22451440 = 41952403;    double pLIXSuUsqQXBvGCCyUUd46288223 = -113741193;    double pLIXSuUsqQXBvGCCyUUd17155501 = -423354067;    double pLIXSuUsqQXBvGCCyUUd76716374 = -790869438;    double pLIXSuUsqQXBvGCCyUUd36907476 = -346304666;    double pLIXSuUsqQXBvGCCyUUd66181935 = -250429484;    double pLIXSuUsqQXBvGCCyUUd20904875 = -298119871;    double pLIXSuUsqQXBvGCCyUUd18651920 = 77852147;    double pLIXSuUsqQXBvGCCyUUd92930874 = -4314200;    double pLIXSuUsqQXBvGCCyUUd15306675 = 78023874;    double pLIXSuUsqQXBvGCCyUUd8892183 = -304201943;    double pLIXSuUsqQXBvGCCyUUd79210944 = -11155047;    double pLIXSuUsqQXBvGCCyUUd739721 = -319006068;    double pLIXSuUsqQXBvGCCyUUd79826776 = 29335397;    double pLIXSuUsqQXBvGCCyUUd87307463 = -721136401;    double pLIXSuUsqQXBvGCCyUUd58686674 = -822617482;    double pLIXSuUsqQXBvGCCyUUd17152052 = -848968077;    double pLIXSuUsqQXBvGCCyUUd41056344 = -159141972;    double pLIXSuUsqQXBvGCCyUUd60171614 = -88939268;    double pLIXSuUsqQXBvGCCyUUd69358616 = -206022236;    double pLIXSuUsqQXBvGCCyUUd19326563 = -439568869;    double pLIXSuUsqQXBvGCCyUUd2290227 = -110266661;    double pLIXSuUsqQXBvGCCyUUd82354110 = -367586718;    double pLIXSuUsqQXBvGCCyUUd81822282 = -927658181;    double pLIXSuUsqQXBvGCCyUUd19866426 = -295908894;    double pLIXSuUsqQXBvGCCyUUd94082814 = -478483006;    double pLIXSuUsqQXBvGCCyUUd67799827 = -687290719;    double pLIXSuUsqQXBvGCCyUUd34938693 = 22520474;    double pLIXSuUsqQXBvGCCyUUd43509588 = -26264416;    double pLIXSuUsqQXBvGCCyUUd11307528 = -908837836;    double pLIXSuUsqQXBvGCCyUUd1264104 = 13248212;    double pLIXSuUsqQXBvGCCyUUd65542959 = 24614905;    double pLIXSuUsqQXBvGCCyUUd17341469 = -122566355;    double pLIXSuUsqQXBvGCCyUUd4699314 = -907860722;    double pLIXSuUsqQXBvGCCyUUd46705730 = -245938358;    double pLIXSuUsqQXBvGCCyUUd43616647 = -594051132;    double pLIXSuUsqQXBvGCCyUUd73304628 = -671825269;    double pLIXSuUsqQXBvGCCyUUd72786942 = -414534484;    double pLIXSuUsqQXBvGCCyUUd45462299 = -567987714;    double pLIXSuUsqQXBvGCCyUUd85016152 = -662774433;    double pLIXSuUsqQXBvGCCyUUd23518665 = -141389610;    double pLIXSuUsqQXBvGCCyUUd51177710 = 38558179;    double pLIXSuUsqQXBvGCCyUUd45383382 = -4854911;    double pLIXSuUsqQXBvGCCyUUd8314994 = -553992679;    double pLIXSuUsqQXBvGCCyUUd8397604 = -303808373;    double pLIXSuUsqQXBvGCCyUUd56592421 = -12337078;    double pLIXSuUsqQXBvGCCyUUd76609389 = -163308421;    double pLIXSuUsqQXBvGCCyUUd85543403 = -754028476;    double pLIXSuUsqQXBvGCCyUUd62702686 = -149849584;    double pLIXSuUsqQXBvGCCyUUd44808895 = -224408027;    double pLIXSuUsqQXBvGCCyUUd12507884 = -737954617;    double pLIXSuUsqQXBvGCCyUUd59401329 = 95531135;    double pLIXSuUsqQXBvGCCyUUd73142020 = -109700350;    double pLIXSuUsqQXBvGCCyUUd36356843 = -756981125;    double pLIXSuUsqQXBvGCCyUUd66151023 = -844898947;    double pLIXSuUsqQXBvGCCyUUd20161214 = -847780937;    double pLIXSuUsqQXBvGCCyUUd63934113 = -746154476;    double pLIXSuUsqQXBvGCCyUUd35333218 = -495695886;    double pLIXSuUsqQXBvGCCyUUd56849949 = -394960544;    double pLIXSuUsqQXBvGCCyUUd42824661 = -867821660;    double pLIXSuUsqQXBvGCCyUUd98382107 = -563138765;    double pLIXSuUsqQXBvGCCyUUd85966182 = -220640345;    double pLIXSuUsqQXBvGCCyUUd75142332 = -895883438;    double pLIXSuUsqQXBvGCCyUUd81623347 = -95476364;    double pLIXSuUsqQXBvGCCyUUd14042572 = -935224339;    double pLIXSuUsqQXBvGCCyUUd43349224 = -228816849;    double pLIXSuUsqQXBvGCCyUUd61869475 = -888588692;    double pLIXSuUsqQXBvGCCyUUd96040407 = -411145347;    double pLIXSuUsqQXBvGCCyUUd33121046 = -724726245;    double pLIXSuUsqQXBvGCCyUUd43690817 = -27085269;    double pLIXSuUsqQXBvGCCyUUd85382046 = -50792213;    double pLIXSuUsqQXBvGCCyUUd44365109 = -334433594;    double pLIXSuUsqQXBvGCCyUUd95594044 = -591154258;    double pLIXSuUsqQXBvGCCyUUd75155461 = -426164836;    double pLIXSuUsqQXBvGCCyUUd45839952 = 35367374;    double pLIXSuUsqQXBvGCCyUUd68148852 = -378127049;    double pLIXSuUsqQXBvGCCyUUd56906844 = -5411750;    double pLIXSuUsqQXBvGCCyUUd74039117 = -813594039;    double pLIXSuUsqQXBvGCCyUUd73424678 = -523849809;    double pLIXSuUsqQXBvGCCyUUd63274004 = -183571817;    double pLIXSuUsqQXBvGCCyUUd17473425 = -215174585;    double pLIXSuUsqQXBvGCCyUUd82256424 = -933262243;    double pLIXSuUsqQXBvGCCyUUd72236007 = -827629942;    double pLIXSuUsqQXBvGCCyUUd98700692 = -801856389;    double pLIXSuUsqQXBvGCCyUUd98799644 = -70883219;    double pLIXSuUsqQXBvGCCyUUd41862775 = 17717076;    double pLIXSuUsqQXBvGCCyUUd92400938 = -865684746;    double pLIXSuUsqQXBvGCCyUUd80984626 = -365585231;    double pLIXSuUsqQXBvGCCyUUd38548291 = -708010316;     pLIXSuUsqQXBvGCCyUUd17289787 = pLIXSuUsqQXBvGCCyUUd35803366;     pLIXSuUsqQXBvGCCyUUd35803366 = pLIXSuUsqQXBvGCCyUUd77349110;     pLIXSuUsqQXBvGCCyUUd77349110 = pLIXSuUsqQXBvGCCyUUd65370179;     pLIXSuUsqQXBvGCCyUUd65370179 = pLIXSuUsqQXBvGCCyUUd50010149;     pLIXSuUsqQXBvGCCyUUd50010149 = pLIXSuUsqQXBvGCCyUUd3495569;     pLIXSuUsqQXBvGCCyUUd3495569 = pLIXSuUsqQXBvGCCyUUd29659936;     pLIXSuUsqQXBvGCCyUUd29659936 = pLIXSuUsqQXBvGCCyUUd457674;     pLIXSuUsqQXBvGCCyUUd457674 = pLIXSuUsqQXBvGCCyUUd33313634;     pLIXSuUsqQXBvGCCyUUd33313634 = pLIXSuUsqQXBvGCCyUUd5715460;     pLIXSuUsqQXBvGCCyUUd5715460 = pLIXSuUsqQXBvGCCyUUd85477586;     pLIXSuUsqQXBvGCCyUUd85477586 = pLIXSuUsqQXBvGCCyUUd22451440;     pLIXSuUsqQXBvGCCyUUd22451440 = pLIXSuUsqQXBvGCCyUUd46288223;     pLIXSuUsqQXBvGCCyUUd46288223 = pLIXSuUsqQXBvGCCyUUd17155501;     pLIXSuUsqQXBvGCCyUUd17155501 = pLIXSuUsqQXBvGCCyUUd76716374;     pLIXSuUsqQXBvGCCyUUd76716374 = pLIXSuUsqQXBvGCCyUUd36907476;     pLIXSuUsqQXBvGCCyUUd36907476 = pLIXSuUsqQXBvGCCyUUd66181935;     pLIXSuUsqQXBvGCCyUUd66181935 = pLIXSuUsqQXBvGCCyUUd20904875;     pLIXSuUsqQXBvGCCyUUd20904875 = pLIXSuUsqQXBvGCCyUUd18651920;     pLIXSuUsqQXBvGCCyUUd18651920 = pLIXSuUsqQXBvGCCyUUd92930874;     pLIXSuUsqQXBvGCCyUUd92930874 = pLIXSuUsqQXBvGCCyUUd15306675;     pLIXSuUsqQXBvGCCyUUd15306675 = pLIXSuUsqQXBvGCCyUUd8892183;     pLIXSuUsqQXBvGCCyUUd8892183 = pLIXSuUsqQXBvGCCyUUd79210944;     pLIXSuUsqQXBvGCCyUUd79210944 = pLIXSuUsqQXBvGCCyUUd739721;     pLIXSuUsqQXBvGCCyUUd739721 = pLIXSuUsqQXBvGCCyUUd79826776;     pLIXSuUsqQXBvGCCyUUd79826776 = pLIXSuUsqQXBvGCCyUUd87307463;     pLIXSuUsqQXBvGCCyUUd87307463 = pLIXSuUsqQXBvGCCyUUd58686674;     pLIXSuUsqQXBvGCCyUUd58686674 = pLIXSuUsqQXBvGCCyUUd17152052;     pLIXSuUsqQXBvGCCyUUd17152052 = pLIXSuUsqQXBvGCCyUUd41056344;     pLIXSuUsqQXBvGCCyUUd41056344 = pLIXSuUsqQXBvGCCyUUd60171614;     pLIXSuUsqQXBvGCCyUUd60171614 = pLIXSuUsqQXBvGCCyUUd69358616;     pLIXSuUsqQXBvGCCyUUd69358616 = pLIXSuUsqQXBvGCCyUUd19326563;     pLIXSuUsqQXBvGCCyUUd19326563 = pLIXSuUsqQXBvGCCyUUd2290227;     pLIXSuUsqQXBvGCCyUUd2290227 = pLIXSuUsqQXBvGCCyUUd82354110;     pLIXSuUsqQXBvGCCyUUd82354110 = pLIXSuUsqQXBvGCCyUUd81822282;     pLIXSuUsqQXBvGCCyUUd81822282 = pLIXSuUsqQXBvGCCyUUd19866426;     pLIXSuUsqQXBvGCCyUUd19866426 = pLIXSuUsqQXBvGCCyUUd94082814;     pLIXSuUsqQXBvGCCyUUd94082814 = pLIXSuUsqQXBvGCCyUUd67799827;     pLIXSuUsqQXBvGCCyUUd67799827 = pLIXSuUsqQXBvGCCyUUd34938693;     pLIXSuUsqQXBvGCCyUUd34938693 = pLIXSuUsqQXBvGCCyUUd43509588;     pLIXSuUsqQXBvGCCyUUd43509588 = pLIXSuUsqQXBvGCCyUUd11307528;     pLIXSuUsqQXBvGCCyUUd11307528 = pLIXSuUsqQXBvGCCyUUd1264104;     pLIXSuUsqQXBvGCCyUUd1264104 = pLIXSuUsqQXBvGCCyUUd65542959;     pLIXSuUsqQXBvGCCyUUd65542959 = pLIXSuUsqQXBvGCCyUUd17341469;     pLIXSuUsqQXBvGCCyUUd17341469 = pLIXSuUsqQXBvGCCyUUd4699314;     pLIXSuUsqQXBvGCCyUUd4699314 = pLIXSuUsqQXBvGCCyUUd46705730;     pLIXSuUsqQXBvGCCyUUd46705730 = pLIXSuUsqQXBvGCCyUUd43616647;     pLIXSuUsqQXBvGCCyUUd43616647 = pLIXSuUsqQXBvGCCyUUd73304628;     pLIXSuUsqQXBvGCCyUUd73304628 = pLIXSuUsqQXBvGCCyUUd72786942;     pLIXSuUsqQXBvGCCyUUd72786942 = pLIXSuUsqQXBvGCCyUUd45462299;     pLIXSuUsqQXBvGCCyUUd45462299 = pLIXSuUsqQXBvGCCyUUd85016152;     pLIXSuUsqQXBvGCCyUUd85016152 = pLIXSuUsqQXBvGCCyUUd23518665;     pLIXSuUsqQXBvGCCyUUd23518665 = pLIXSuUsqQXBvGCCyUUd51177710;     pLIXSuUsqQXBvGCCyUUd51177710 = pLIXSuUsqQXBvGCCyUUd45383382;     pLIXSuUsqQXBvGCCyUUd45383382 = pLIXSuUsqQXBvGCCyUUd8314994;     pLIXSuUsqQXBvGCCyUUd8314994 = pLIXSuUsqQXBvGCCyUUd8397604;     pLIXSuUsqQXBvGCCyUUd8397604 = pLIXSuUsqQXBvGCCyUUd56592421;     pLIXSuUsqQXBvGCCyUUd56592421 = pLIXSuUsqQXBvGCCyUUd76609389;     pLIXSuUsqQXBvGCCyUUd76609389 = pLIXSuUsqQXBvGCCyUUd85543403;     pLIXSuUsqQXBvGCCyUUd85543403 = pLIXSuUsqQXBvGCCyUUd62702686;     pLIXSuUsqQXBvGCCyUUd62702686 = pLIXSuUsqQXBvGCCyUUd44808895;     pLIXSuUsqQXBvGCCyUUd44808895 = pLIXSuUsqQXBvGCCyUUd12507884;     pLIXSuUsqQXBvGCCyUUd12507884 = pLIXSuUsqQXBvGCCyUUd59401329;     pLIXSuUsqQXBvGCCyUUd59401329 = pLIXSuUsqQXBvGCCyUUd73142020;     pLIXSuUsqQXBvGCCyUUd73142020 = pLIXSuUsqQXBvGCCyUUd36356843;     pLIXSuUsqQXBvGCCyUUd36356843 = pLIXSuUsqQXBvGCCyUUd66151023;     pLIXSuUsqQXBvGCCyUUd66151023 = pLIXSuUsqQXBvGCCyUUd20161214;     pLIXSuUsqQXBvGCCyUUd20161214 = pLIXSuUsqQXBvGCCyUUd63934113;     pLIXSuUsqQXBvGCCyUUd63934113 = pLIXSuUsqQXBvGCCyUUd35333218;     pLIXSuUsqQXBvGCCyUUd35333218 = pLIXSuUsqQXBvGCCyUUd56849949;     pLIXSuUsqQXBvGCCyUUd56849949 = pLIXSuUsqQXBvGCCyUUd42824661;     pLIXSuUsqQXBvGCCyUUd42824661 = pLIXSuUsqQXBvGCCyUUd98382107;     pLIXSuUsqQXBvGCCyUUd98382107 = pLIXSuUsqQXBvGCCyUUd85966182;     pLIXSuUsqQXBvGCCyUUd85966182 = pLIXSuUsqQXBvGCCyUUd75142332;     pLIXSuUsqQXBvGCCyUUd75142332 = pLIXSuUsqQXBvGCCyUUd81623347;     pLIXSuUsqQXBvGCCyUUd81623347 = pLIXSuUsqQXBvGCCyUUd14042572;     pLIXSuUsqQXBvGCCyUUd14042572 = pLIXSuUsqQXBvGCCyUUd43349224;     pLIXSuUsqQXBvGCCyUUd43349224 = pLIXSuUsqQXBvGCCyUUd61869475;     pLIXSuUsqQXBvGCCyUUd61869475 = pLIXSuUsqQXBvGCCyUUd96040407;     pLIXSuUsqQXBvGCCyUUd96040407 = pLIXSuUsqQXBvGCCyUUd33121046;     pLIXSuUsqQXBvGCCyUUd33121046 = pLIXSuUsqQXBvGCCyUUd43690817;     pLIXSuUsqQXBvGCCyUUd43690817 = pLIXSuUsqQXBvGCCyUUd85382046;     pLIXSuUsqQXBvGCCyUUd85382046 = pLIXSuUsqQXBvGCCyUUd44365109;     pLIXSuUsqQXBvGCCyUUd44365109 = pLIXSuUsqQXBvGCCyUUd95594044;     pLIXSuUsqQXBvGCCyUUd95594044 = pLIXSuUsqQXBvGCCyUUd75155461;     pLIXSuUsqQXBvGCCyUUd75155461 = pLIXSuUsqQXBvGCCyUUd45839952;     pLIXSuUsqQXBvGCCyUUd45839952 = pLIXSuUsqQXBvGCCyUUd68148852;     pLIXSuUsqQXBvGCCyUUd68148852 = pLIXSuUsqQXBvGCCyUUd56906844;     pLIXSuUsqQXBvGCCyUUd56906844 = pLIXSuUsqQXBvGCCyUUd74039117;     pLIXSuUsqQXBvGCCyUUd74039117 = pLIXSuUsqQXBvGCCyUUd73424678;     pLIXSuUsqQXBvGCCyUUd73424678 = pLIXSuUsqQXBvGCCyUUd63274004;     pLIXSuUsqQXBvGCCyUUd63274004 = pLIXSuUsqQXBvGCCyUUd17473425;     pLIXSuUsqQXBvGCCyUUd17473425 = pLIXSuUsqQXBvGCCyUUd82256424;     pLIXSuUsqQXBvGCCyUUd82256424 = pLIXSuUsqQXBvGCCyUUd72236007;     pLIXSuUsqQXBvGCCyUUd72236007 = pLIXSuUsqQXBvGCCyUUd98700692;     pLIXSuUsqQXBvGCCyUUd98700692 = pLIXSuUsqQXBvGCCyUUd98799644;     pLIXSuUsqQXBvGCCyUUd98799644 = pLIXSuUsqQXBvGCCyUUd41862775;     pLIXSuUsqQXBvGCCyUUd41862775 = pLIXSuUsqQXBvGCCyUUd92400938;     pLIXSuUsqQXBvGCCyUUd92400938 = pLIXSuUsqQXBvGCCyUUd80984626;     pLIXSuUsqQXBvGCCyUUd80984626 = pLIXSuUsqQXBvGCCyUUd38548291;     pLIXSuUsqQXBvGCCyUUd38548291 = pLIXSuUsqQXBvGCCyUUd17289787;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void uDfkYSJQPPnrOMPwLpxj51215925() {     double XuNucPYyJeHvgUTEhRUI42875064 = -133043417;    double XuNucPYyJeHvgUTEhRUI32200509 = -378969901;    double XuNucPYyJeHvgUTEhRUI10228286 = -384776025;    double XuNucPYyJeHvgUTEhRUI11092109 = -832015972;    double XuNucPYyJeHvgUTEhRUI19352154 = 82091728;    double XuNucPYyJeHvgUTEhRUI20504845 = -202006512;    double XuNucPYyJeHvgUTEhRUI34980780 = -69193901;    double XuNucPYyJeHvgUTEhRUI15467798 = -544292715;    double XuNucPYyJeHvgUTEhRUI58642927 = -794863662;    double XuNucPYyJeHvgUTEhRUI24873842 = -297564883;    double XuNucPYyJeHvgUTEhRUI99811139 = -594895703;    double XuNucPYyJeHvgUTEhRUI57769095 = -401756504;    double XuNucPYyJeHvgUTEhRUI22869001 = -977989558;    double XuNucPYyJeHvgUTEhRUI8715217 = -397878360;    double XuNucPYyJeHvgUTEhRUI32985263 = -679415396;    double XuNucPYyJeHvgUTEhRUI44628519 = -782496143;    double XuNucPYyJeHvgUTEhRUI65980051 = 61250722;    double XuNucPYyJeHvgUTEhRUI42020780 = -814397231;    double XuNucPYyJeHvgUTEhRUI80116640 = -705121710;    double XuNucPYyJeHvgUTEhRUI54092341 = 21246424;    double XuNucPYyJeHvgUTEhRUI16315941 = -383675907;    double XuNucPYyJeHvgUTEhRUI10395290 = -723934873;    double XuNucPYyJeHvgUTEhRUI82974742 = -55247152;    double XuNucPYyJeHvgUTEhRUI2453764 = -177855048;    double XuNucPYyJeHvgUTEhRUI34729915 = -638160745;    double XuNucPYyJeHvgUTEhRUI16796600 = -285586942;    double XuNucPYyJeHvgUTEhRUI41074583 = -388430617;    double XuNucPYyJeHvgUTEhRUI81050852 = 21369690;    double XuNucPYyJeHvgUTEhRUI48933556 = -395435822;    double XuNucPYyJeHvgUTEhRUI13494105 = 2151170;    double XuNucPYyJeHvgUTEhRUI91405250 = 15330734;    double XuNucPYyJeHvgUTEhRUI56516495 = -713364703;    double XuNucPYyJeHvgUTEhRUI23477278 = -558087798;    double XuNucPYyJeHvgUTEhRUI35941182 = -423731689;    double XuNucPYyJeHvgUTEhRUI83227270 = -529094093;    double XuNucPYyJeHvgUTEhRUI24658276 = -813503610;    double XuNucPYyJeHvgUTEhRUI43728200 = -958994951;    double XuNucPYyJeHvgUTEhRUI43372514 = -536005544;    double XuNucPYyJeHvgUTEhRUI75862271 = -435817848;    double XuNucPYyJeHvgUTEhRUI52293261 = -496234735;    double XuNucPYyJeHvgUTEhRUI58458809 = -383898026;    double XuNucPYyJeHvgUTEhRUI45273694 = -495790292;    double XuNucPYyJeHvgUTEhRUI14743179 = -3991476;    double XuNucPYyJeHvgUTEhRUI88134748 = -590981954;    double XuNucPYyJeHvgUTEhRUI96680189 = -846881839;    double XuNucPYyJeHvgUTEhRUI13106053 = -768485896;    double XuNucPYyJeHvgUTEhRUI43385101 = -626519993;    double XuNucPYyJeHvgUTEhRUI81417399 = -865008116;    double XuNucPYyJeHvgUTEhRUI23598263 = -514210711;    double XuNucPYyJeHvgUTEhRUI99272294 = -416010867;    double XuNucPYyJeHvgUTEhRUI95152399 = -911433035;    double XuNucPYyJeHvgUTEhRUI50777244 = -319468790;    double XuNucPYyJeHvgUTEhRUI69251314 = -529340370;    double XuNucPYyJeHvgUTEhRUI14282291 = -319918693;    double XuNucPYyJeHvgUTEhRUI34899985 = 70916904;    double XuNucPYyJeHvgUTEhRUI32479774 = -409108544;    double XuNucPYyJeHvgUTEhRUI49225767 = -223722750;    double XuNucPYyJeHvgUTEhRUI7774523 = -106920978;    double XuNucPYyJeHvgUTEhRUI76362194 = -93855227;    double XuNucPYyJeHvgUTEhRUI2555555 = -632321330;    double XuNucPYyJeHvgUTEhRUI79430262 = -813575896;    double XuNucPYyJeHvgUTEhRUI53929928 = 9436408;    double XuNucPYyJeHvgUTEhRUI66534241 = -48856893;    double XuNucPYyJeHvgUTEhRUI45148822 = -697014832;    double XuNucPYyJeHvgUTEhRUI33468591 = -212895617;    double XuNucPYyJeHvgUTEhRUI43294644 = -881531000;    double XuNucPYyJeHvgUTEhRUI34291817 = -843668706;    double XuNucPYyJeHvgUTEhRUI86927819 = -454257870;    double XuNucPYyJeHvgUTEhRUI25487947 = -868784268;    double XuNucPYyJeHvgUTEhRUI8326987 = -865911786;    double XuNucPYyJeHvgUTEhRUI900319 = -823501193;    double XuNucPYyJeHvgUTEhRUI22607537 = -402743734;    double XuNucPYyJeHvgUTEhRUI66158508 = -278579384;    double XuNucPYyJeHvgUTEhRUI27823379 = -108886976;    double XuNucPYyJeHvgUTEhRUI95633532 = -594855550;    double XuNucPYyJeHvgUTEhRUI71042247 = -887885616;    double XuNucPYyJeHvgUTEhRUI95652110 = -619943397;    double XuNucPYyJeHvgUTEhRUI94839994 = -464265199;    double XuNucPYyJeHvgUTEhRUI5773574 = -330973209;    double XuNucPYyJeHvgUTEhRUI21623863 = -869674850;    double XuNucPYyJeHvgUTEhRUI73411499 = -659066950;    double XuNucPYyJeHvgUTEhRUI59657184 = -523422501;    double XuNucPYyJeHvgUTEhRUI57452590 = -464419600;    double XuNucPYyJeHvgUTEhRUI49661262 = -979424956;    double XuNucPYyJeHvgUTEhRUI18341705 = -86415796;    double XuNucPYyJeHvgUTEhRUI40628007 = -665200477;    double XuNucPYyJeHvgUTEhRUI87265181 = -84024334;    double XuNucPYyJeHvgUTEhRUI9194988 = -138169106;    double XuNucPYyJeHvgUTEhRUI1041197 = -394648593;    double XuNucPYyJeHvgUTEhRUI50747496 = -19985549;    double XuNucPYyJeHvgUTEhRUI75432509 = -489780861;    double XuNucPYyJeHvgUTEhRUI35953677 = -752073974;    double XuNucPYyJeHvgUTEhRUI67010320 = -342150317;    double XuNucPYyJeHvgUTEhRUI73306717 = -803496518;    double XuNucPYyJeHvgUTEhRUI72862999 = -682658840;    double XuNucPYyJeHvgUTEhRUI4528881 = -293334435;    double XuNucPYyJeHvgUTEhRUI78739452 = -346933399;    double XuNucPYyJeHvgUTEhRUI69594356 = -306976644;    double XuNucPYyJeHvgUTEhRUI54666157 = -278086337;    double XuNucPYyJeHvgUTEhRUI53385545 = -133043417;     XuNucPYyJeHvgUTEhRUI42875064 = XuNucPYyJeHvgUTEhRUI32200509;     XuNucPYyJeHvgUTEhRUI32200509 = XuNucPYyJeHvgUTEhRUI10228286;     XuNucPYyJeHvgUTEhRUI10228286 = XuNucPYyJeHvgUTEhRUI11092109;     XuNucPYyJeHvgUTEhRUI11092109 = XuNucPYyJeHvgUTEhRUI19352154;     XuNucPYyJeHvgUTEhRUI19352154 = XuNucPYyJeHvgUTEhRUI20504845;     XuNucPYyJeHvgUTEhRUI20504845 = XuNucPYyJeHvgUTEhRUI34980780;     XuNucPYyJeHvgUTEhRUI34980780 = XuNucPYyJeHvgUTEhRUI15467798;     XuNucPYyJeHvgUTEhRUI15467798 = XuNucPYyJeHvgUTEhRUI58642927;     XuNucPYyJeHvgUTEhRUI58642927 = XuNucPYyJeHvgUTEhRUI24873842;     XuNucPYyJeHvgUTEhRUI24873842 = XuNucPYyJeHvgUTEhRUI99811139;     XuNucPYyJeHvgUTEhRUI99811139 = XuNucPYyJeHvgUTEhRUI57769095;     XuNucPYyJeHvgUTEhRUI57769095 = XuNucPYyJeHvgUTEhRUI22869001;     XuNucPYyJeHvgUTEhRUI22869001 = XuNucPYyJeHvgUTEhRUI8715217;     XuNucPYyJeHvgUTEhRUI8715217 = XuNucPYyJeHvgUTEhRUI32985263;     XuNucPYyJeHvgUTEhRUI32985263 = XuNucPYyJeHvgUTEhRUI44628519;     XuNucPYyJeHvgUTEhRUI44628519 = XuNucPYyJeHvgUTEhRUI65980051;     XuNucPYyJeHvgUTEhRUI65980051 = XuNucPYyJeHvgUTEhRUI42020780;     XuNucPYyJeHvgUTEhRUI42020780 = XuNucPYyJeHvgUTEhRUI80116640;     XuNucPYyJeHvgUTEhRUI80116640 = XuNucPYyJeHvgUTEhRUI54092341;     XuNucPYyJeHvgUTEhRUI54092341 = XuNucPYyJeHvgUTEhRUI16315941;     XuNucPYyJeHvgUTEhRUI16315941 = XuNucPYyJeHvgUTEhRUI10395290;     XuNucPYyJeHvgUTEhRUI10395290 = XuNucPYyJeHvgUTEhRUI82974742;     XuNucPYyJeHvgUTEhRUI82974742 = XuNucPYyJeHvgUTEhRUI2453764;     XuNucPYyJeHvgUTEhRUI2453764 = XuNucPYyJeHvgUTEhRUI34729915;     XuNucPYyJeHvgUTEhRUI34729915 = XuNucPYyJeHvgUTEhRUI16796600;     XuNucPYyJeHvgUTEhRUI16796600 = XuNucPYyJeHvgUTEhRUI41074583;     XuNucPYyJeHvgUTEhRUI41074583 = XuNucPYyJeHvgUTEhRUI81050852;     XuNucPYyJeHvgUTEhRUI81050852 = XuNucPYyJeHvgUTEhRUI48933556;     XuNucPYyJeHvgUTEhRUI48933556 = XuNucPYyJeHvgUTEhRUI13494105;     XuNucPYyJeHvgUTEhRUI13494105 = XuNucPYyJeHvgUTEhRUI91405250;     XuNucPYyJeHvgUTEhRUI91405250 = XuNucPYyJeHvgUTEhRUI56516495;     XuNucPYyJeHvgUTEhRUI56516495 = XuNucPYyJeHvgUTEhRUI23477278;     XuNucPYyJeHvgUTEhRUI23477278 = XuNucPYyJeHvgUTEhRUI35941182;     XuNucPYyJeHvgUTEhRUI35941182 = XuNucPYyJeHvgUTEhRUI83227270;     XuNucPYyJeHvgUTEhRUI83227270 = XuNucPYyJeHvgUTEhRUI24658276;     XuNucPYyJeHvgUTEhRUI24658276 = XuNucPYyJeHvgUTEhRUI43728200;     XuNucPYyJeHvgUTEhRUI43728200 = XuNucPYyJeHvgUTEhRUI43372514;     XuNucPYyJeHvgUTEhRUI43372514 = XuNucPYyJeHvgUTEhRUI75862271;     XuNucPYyJeHvgUTEhRUI75862271 = XuNucPYyJeHvgUTEhRUI52293261;     XuNucPYyJeHvgUTEhRUI52293261 = XuNucPYyJeHvgUTEhRUI58458809;     XuNucPYyJeHvgUTEhRUI58458809 = XuNucPYyJeHvgUTEhRUI45273694;     XuNucPYyJeHvgUTEhRUI45273694 = XuNucPYyJeHvgUTEhRUI14743179;     XuNucPYyJeHvgUTEhRUI14743179 = XuNucPYyJeHvgUTEhRUI88134748;     XuNucPYyJeHvgUTEhRUI88134748 = XuNucPYyJeHvgUTEhRUI96680189;     XuNucPYyJeHvgUTEhRUI96680189 = XuNucPYyJeHvgUTEhRUI13106053;     XuNucPYyJeHvgUTEhRUI13106053 = XuNucPYyJeHvgUTEhRUI43385101;     XuNucPYyJeHvgUTEhRUI43385101 = XuNucPYyJeHvgUTEhRUI81417399;     XuNucPYyJeHvgUTEhRUI81417399 = XuNucPYyJeHvgUTEhRUI23598263;     XuNucPYyJeHvgUTEhRUI23598263 = XuNucPYyJeHvgUTEhRUI99272294;     XuNucPYyJeHvgUTEhRUI99272294 = XuNucPYyJeHvgUTEhRUI95152399;     XuNucPYyJeHvgUTEhRUI95152399 = XuNucPYyJeHvgUTEhRUI50777244;     XuNucPYyJeHvgUTEhRUI50777244 = XuNucPYyJeHvgUTEhRUI69251314;     XuNucPYyJeHvgUTEhRUI69251314 = XuNucPYyJeHvgUTEhRUI14282291;     XuNucPYyJeHvgUTEhRUI14282291 = XuNucPYyJeHvgUTEhRUI34899985;     XuNucPYyJeHvgUTEhRUI34899985 = XuNucPYyJeHvgUTEhRUI32479774;     XuNucPYyJeHvgUTEhRUI32479774 = XuNucPYyJeHvgUTEhRUI49225767;     XuNucPYyJeHvgUTEhRUI49225767 = XuNucPYyJeHvgUTEhRUI7774523;     XuNucPYyJeHvgUTEhRUI7774523 = XuNucPYyJeHvgUTEhRUI76362194;     XuNucPYyJeHvgUTEhRUI76362194 = XuNucPYyJeHvgUTEhRUI2555555;     XuNucPYyJeHvgUTEhRUI2555555 = XuNucPYyJeHvgUTEhRUI79430262;     XuNucPYyJeHvgUTEhRUI79430262 = XuNucPYyJeHvgUTEhRUI53929928;     XuNucPYyJeHvgUTEhRUI53929928 = XuNucPYyJeHvgUTEhRUI66534241;     XuNucPYyJeHvgUTEhRUI66534241 = XuNucPYyJeHvgUTEhRUI45148822;     XuNucPYyJeHvgUTEhRUI45148822 = XuNucPYyJeHvgUTEhRUI33468591;     XuNucPYyJeHvgUTEhRUI33468591 = XuNucPYyJeHvgUTEhRUI43294644;     XuNucPYyJeHvgUTEhRUI43294644 = XuNucPYyJeHvgUTEhRUI34291817;     XuNucPYyJeHvgUTEhRUI34291817 = XuNucPYyJeHvgUTEhRUI86927819;     XuNucPYyJeHvgUTEhRUI86927819 = XuNucPYyJeHvgUTEhRUI25487947;     XuNucPYyJeHvgUTEhRUI25487947 = XuNucPYyJeHvgUTEhRUI8326987;     XuNucPYyJeHvgUTEhRUI8326987 = XuNucPYyJeHvgUTEhRUI900319;     XuNucPYyJeHvgUTEhRUI900319 = XuNucPYyJeHvgUTEhRUI22607537;     XuNucPYyJeHvgUTEhRUI22607537 = XuNucPYyJeHvgUTEhRUI66158508;     XuNucPYyJeHvgUTEhRUI66158508 = XuNucPYyJeHvgUTEhRUI27823379;     XuNucPYyJeHvgUTEhRUI27823379 = XuNucPYyJeHvgUTEhRUI95633532;     XuNucPYyJeHvgUTEhRUI95633532 = XuNucPYyJeHvgUTEhRUI71042247;     XuNucPYyJeHvgUTEhRUI71042247 = XuNucPYyJeHvgUTEhRUI95652110;     XuNucPYyJeHvgUTEhRUI95652110 = XuNucPYyJeHvgUTEhRUI94839994;     XuNucPYyJeHvgUTEhRUI94839994 = XuNucPYyJeHvgUTEhRUI5773574;     XuNucPYyJeHvgUTEhRUI5773574 = XuNucPYyJeHvgUTEhRUI21623863;     XuNucPYyJeHvgUTEhRUI21623863 = XuNucPYyJeHvgUTEhRUI73411499;     XuNucPYyJeHvgUTEhRUI73411499 = XuNucPYyJeHvgUTEhRUI59657184;     XuNucPYyJeHvgUTEhRUI59657184 = XuNucPYyJeHvgUTEhRUI57452590;     XuNucPYyJeHvgUTEhRUI57452590 = XuNucPYyJeHvgUTEhRUI49661262;     XuNucPYyJeHvgUTEhRUI49661262 = XuNucPYyJeHvgUTEhRUI18341705;     XuNucPYyJeHvgUTEhRUI18341705 = XuNucPYyJeHvgUTEhRUI40628007;     XuNucPYyJeHvgUTEhRUI40628007 = XuNucPYyJeHvgUTEhRUI87265181;     XuNucPYyJeHvgUTEhRUI87265181 = XuNucPYyJeHvgUTEhRUI9194988;     XuNucPYyJeHvgUTEhRUI9194988 = XuNucPYyJeHvgUTEhRUI1041197;     XuNucPYyJeHvgUTEhRUI1041197 = XuNucPYyJeHvgUTEhRUI50747496;     XuNucPYyJeHvgUTEhRUI50747496 = XuNucPYyJeHvgUTEhRUI75432509;     XuNucPYyJeHvgUTEhRUI75432509 = XuNucPYyJeHvgUTEhRUI35953677;     XuNucPYyJeHvgUTEhRUI35953677 = XuNucPYyJeHvgUTEhRUI67010320;     XuNucPYyJeHvgUTEhRUI67010320 = XuNucPYyJeHvgUTEhRUI73306717;     XuNucPYyJeHvgUTEhRUI73306717 = XuNucPYyJeHvgUTEhRUI72862999;     XuNucPYyJeHvgUTEhRUI72862999 = XuNucPYyJeHvgUTEhRUI4528881;     XuNucPYyJeHvgUTEhRUI4528881 = XuNucPYyJeHvgUTEhRUI78739452;     XuNucPYyJeHvgUTEhRUI78739452 = XuNucPYyJeHvgUTEhRUI69594356;     XuNucPYyJeHvgUTEhRUI69594356 = XuNucPYyJeHvgUTEhRUI54666157;     XuNucPYyJeHvgUTEhRUI54666157 = XuNucPYyJeHvgUTEhRUI53385545;     XuNucPYyJeHvgUTEhRUI53385545 = XuNucPYyJeHvgUTEhRUI42875064;}
// Junk Finished

// Junk Code By Troll Face & Thaisen's Gen
void utshxfloKgTHLItuNsGp78810182() {     double UJnQmosFaGRtWnSsYjAE68460341 = -658076517;    double UJnQmosFaGRtWnSsYjAE28597652 = -634447678;    double UJnQmosFaGRtWnSsYjAE43107462 = -187237561;    double UJnQmosFaGRtWnSsYjAE56814038 = -839338865;    double UJnQmosFaGRtWnSsYjAE88694159 = 35169441;    double UJnQmosFaGRtWnSsYjAE37514122 = -356987515;    double UJnQmosFaGRtWnSsYjAE40301625 = -651465108;    double UJnQmosFaGRtWnSsYjAE30477922 = -924974594;    double UJnQmosFaGRtWnSsYjAE83972219 = -191087707;    double UJnQmosFaGRtWnSsYjAE44032224 = -632126406;    double UJnQmosFaGRtWnSsYjAE14144694 = -905323589;    double UJnQmosFaGRtWnSsYjAE93086751 = -845465411;    double UJnQmosFaGRtWnSsYjAE99449778 = -742237924;    double UJnQmosFaGRtWnSsYjAE274934 = -372402654;    double UJnQmosFaGRtWnSsYjAE89254150 = -567961353;    double UJnQmosFaGRtWnSsYjAE52349562 = -118687620;    double UJnQmosFaGRtWnSsYjAE65778167 = -727069072;    double UJnQmosFaGRtWnSsYjAE63136684 = -230674592;    double UJnQmosFaGRtWnSsYjAE41581362 = -388095568;    double UJnQmosFaGRtWnSsYjAE15253808 = 46807049;    double UJnQmosFaGRtWnSsYjAE17325206 = -845375688;    double UJnQmosFaGRtWnSsYjAE11898397 = -43667802;    double UJnQmosFaGRtWnSsYjAE86738540 = -99339257;    double UJnQmosFaGRtWnSsYjAE4167806 = -36704027;    double UJnQmosFaGRtWnSsYjAE89633054 = -205656887;    double UJnQmosFaGRtWnSsYjAE46285737 = -950037484;    double UJnQmosFaGRtWnSsYjAE23462492 = 45756249;    double UJnQmosFaGRtWnSsYjAE44949652 = -208292543;    double UJnQmosFaGRtWnSsYjAE56810769 = -631729672;    double UJnQmosFaGRtWnSsYjAE66816595 = 93241608;    double UJnQmosFaGRtWnSsYjAE13451886 = -863316297;    double UJnQmosFaGRtWnSsYjAE93706427 = -987160537;    double UJnQmosFaGRtWnSsYjAE44664330 = 94091065;    double UJnQmosFaGRtWnSsYjAE89528252 = -479876661;    double UJnQmosFaGRtWnSsYjAE84632258 = -130530004;    double UJnQmosFaGRtWnSsYjAE29450126 = -231098326;    double UJnQmosFaGRtWnSsYjAE93373585 = -339506895;    double UJnQmosFaGRtWnSsYjAE18945201 = -384720369;    double UJnQmosFaGRtWnSsYjAE16785851 = -894156170;    double UJnQmosFaGRtWnSsYjAE61076935 = -966205054;    double UJnQmosFaGRtWnSsYjAE5610090 = -958958216;    double UJnQmosFaGRtWnSsYjAE89283283 = 95171204;    double UJnQmosFaGRtWnSsYjAE63943398 = -32597858;    double UJnQmosFaGRtWnSsYjAE58928027 = 40602447;    double UJnQmosFaGRtWnSsYjAE88661065 = -785902957;    double UJnQmosFaGRtWnSsYjAE79506374 = -191033433;    double UJnQmosFaGRtWnSsYjAE43153555 = -658988853;    double UJnQmosFaGRtWnSsYjAE89530169 = 41809037;    double UJnQmosFaGRtWnSsYjAE74409582 = -613886937;    double UJnQmosFaGRtWnSsYjAE53082289 = -264034020;    double UJnQmosFaGRtWnSsYjAE5288648 = -60091637;    double UJnQmosFaGRtWnSsYjAE78035823 = -497547969;    double UJnQmosFaGRtWnSsYjAE87324917 = 2761082;    double UJnQmosFaGRtWnSsYjAE83181199 = -634982474;    double UJnQmosFaGRtWnSsYjAE61484976 = -404173513;    double UJnQmosFaGRtWnSsYjAE56561944 = -514408716;    double UJnQmosFaGRtWnSsYjAE41859112 = -435108421;    double UJnQmosFaGRtWnSsYjAE38939656 = -50533534;    double UJnQmosFaGRtWnSsYjAE67180984 = -533681978;    double UJnQmosFaGRtWnSsYjAE42408422 = -14793075;    double UJnQmosFaGRtWnSsYjAE14051630 = -302743764;    double UJnQmosFaGRtWnSsYjAE95351972 = -343172566;    double UJnQmosFaGRtWnSsYjAE73667153 = -193244922;    double UJnQmosFaGRtWnSsYjAE17155624 = -184329315;    double UJnQmosFaGRtWnSsYjAE30580339 = -768810110;    double UJnQmosFaGRtWnSsYjAE20438266 = -918163053;    double UJnQmosFaGRtWnSsYjAE48422421 = -839556476;    double UJnQmosFaGRtWnSsYjAE9921526 = -162361263;    double UJnQmosFaGRtWnSsYjAE15642676 = -141872650;    double UJnQmosFaGRtWnSsYjAE59804024 = -236863028;    double UJnQmosFaGRtWnSsYjAE58975976 = -779180725;    double UJnQmosFaGRtWnSsYjAE46832967 = -242348703;    double UJnQmosFaGRtWnSsYjAE46350834 = -336518423;    double UJnQmosFaGRtWnSsYjAE80504426 = -421890514;    double UJnQmosFaGRtWnSsYjAE9643719 = 5765264;    double UJnQmosFaGRtWnSsYjAE28041922 = -840546893;    double UJnQmosFaGRtWnSsYjAE47954998 = 88930055;    double UJnQmosFaGRtWnSsYjAE27810513 = -39941705;    double UJnQmosFaGRtWnSsYjAE15506740 = -250801071;    double UJnQmosFaGRtWnSsYjAE10126680 = 85376546;    double UJnQmosFaGRtWnSsYjAE3132182 = -191048631;    double UJnQmosFaGRtWnSsYjAE33932322 = -996052789;    double UJnQmosFaGRtWnSsYjAE70540070 = -594405606;    double UJnQmosFaGRtWnSsYjAE3728480 = -267695653;    double UJnQmosFaGRtWnSsYjAE61527948 = -846666755;    double UJnQmosFaGRtWnSsYjAE35416062 = -265768329;    double UJnQmosFaGRtWnSsYjAE6381511 = -889921619;    double UJnQmosFaGRtWnSsYjAE61483130 = -270926461;    double UJnQmosFaGRtWnSsYjAE28043277 = 24296852;    double UJnQmosFaGRtWnSsYjAE28070314 = -616121289;    double UJnQmosFaGRtWnSsYjAE87591014 = -795989905;    double UJnQmosFaGRtWnSsYjAE54433929 = -188973362;    double UJnQmosFaGRtWnSsYjAE51764217 = -851038392;    double UJnQmosFaGRtWnSsYjAE74377428 = -779363095;    double UJnQmosFaGRtWnSsYjAE47025306 = -563461290;    double UJnQmosFaGRtWnSsYjAE10258118 = -515785650;    double UJnQmosFaGRtWnSsYjAE15616131 = -711583875;    double UJnQmosFaGRtWnSsYjAE46787774 = -848268543;    double UJnQmosFaGRtWnSsYjAE28347689 = -190587444;    double UJnQmosFaGRtWnSsYjAE68222800 = -658076517;     UJnQmosFaGRtWnSsYjAE68460341 = UJnQmosFaGRtWnSsYjAE28597652;     UJnQmosFaGRtWnSsYjAE28597652 = UJnQmosFaGRtWnSsYjAE43107462;     UJnQmosFaGRtWnSsYjAE43107462 = UJnQmosFaGRtWnSsYjAE56814038;     UJnQmosFaGRtWnSsYjAE56814038 = UJnQmosFaGRtWnSsYjAE88694159;     UJnQmosFaGRtWnSsYjAE88694159 = UJnQmosFaGRtWnSsYjAE37514122;     UJnQmosFaGRtWnSsYjAE37514122 = UJnQmosFaGRtWnSsYjAE40301625;     UJnQmosFaGRtWnSsYjAE40301625 = UJnQmosFaGRtWnSsYjAE30477922;     UJnQmosFaGRtWnSsYjAE30477922 = UJnQmosFaGRtWnSsYjAE83972219;     UJnQmosFaGRtWnSsYjAE83972219 = UJnQmosFaGRtWnSsYjAE44032224;     UJnQmosFaGRtWnSsYjAE44032224 = UJnQmosFaGRtWnSsYjAE14144694;     UJnQmosFaGRtWnSsYjAE14144694 = UJnQmosFaGRtWnSsYjAE93086751;     UJnQmosFaGRtWnSsYjAE93086751 = UJnQmosFaGRtWnSsYjAE99449778;     UJnQmosFaGRtWnSsYjAE99449778 = UJnQmosFaGRtWnSsYjAE274934;     UJnQmosFaGRtWnSsYjAE274934 = UJnQmosFaGRtWnSsYjAE89254150;     UJnQmosFaGRtWnSsYjAE89254150 = UJnQmosFaGRtWnSsYjAE52349562;     UJnQmosFaGRtWnSsYjAE52349562 = UJnQmosFaGRtWnSsYjAE65778167;     UJnQmosFaGRtWnSsYjAE65778167 = UJnQmosFaGRtWnSsYjAE63136684;     UJnQmosFaGRtWnSsYjAE63136684 = UJnQmosFaGRtWnSsYjAE41581362;     UJnQmosFaGRtWnSsYjAE41581362 = UJnQmosFaGRtWnSsYjAE15253808;     UJnQmosFaGRtWnSsYjAE15253808 = UJnQmosFaGRtWnSsYjAE17325206;     UJnQmosFaGRtWnSsYjAE17325206 = UJnQmosFaGRtWnSsYjAE11898397;     UJnQmosFaGRtWnSsYjAE11898397 = UJnQmosFaGRtWnSsYjAE86738540;     UJnQmosFaGRtWnSsYjAE86738540 = UJnQmosFaGRtWnSsYjAE4167806;     UJnQmosFaGRtWnSsYjAE4167806 = UJnQmosFaGRtWnSsYjAE89633054;     UJnQmosFaGRtWnSsYjAE89633054 = UJnQmosFaGRtWnSsYjAE46285737;     UJnQmosFaGRtWnSsYjAE46285737 = UJnQmosFaGRtWnSsYjAE23462492;     UJnQmosFaGRtWnSsYjAE23462492 = UJnQmosFaGRtWnSsYjAE44949652;     UJnQmosFaGRtWnSsYjAE44949652 = UJnQmosFaGRtWnSsYjAE56810769;     UJnQmosFaGRtWnSsYjAE56810769 = UJnQmosFaGRtWnSsYjAE66816595;     UJnQmosFaGRtWnSsYjAE66816595 = UJnQmosFaGRtWnSsYjAE13451886;     UJnQmosFaGRtWnSsYjAE13451886 = UJnQmosFaGRtWnSsYjAE93706427;     UJnQmosFaGRtWnSsYjAE93706427 = UJnQmosFaGRtWnSsYjAE44664330;     UJnQmosFaGRtWnSsYjAE44664330 = UJnQmosFaGRtWnSsYjAE89528252;     UJnQmosFaGRtWnSsYjAE89528252 = UJnQmosFaGRtWnSsYjAE84632258;     UJnQmosFaGRtWnSsYjAE84632258 = UJnQmosFaGRtWnSsYjAE29450126;     UJnQmosFaGRtWnSsYjAE29450126 = UJnQmosFaGRtWnSsYjAE93373585;     UJnQmosFaGRtWnSsYjAE93373585 = UJnQmosFaGRtWnSsYjAE18945201;     UJnQmosFaGRtWnSsYjAE18945201 = UJnQmosFaGRtWnSsYjAE16785851;     UJnQmosFaGRtWnSsYjAE16785851 = UJnQmosFaGRtWnSsYjAE61076935;     UJnQmosFaGRtWnSsYjAE61076935 = UJnQmosFaGRtWnSsYjAE5610090;     UJnQmosFaGRtWnSsYjAE5610090 = UJnQmosFaGRtWnSsYjAE89283283;     UJnQmosFaGRtWnSsYjAE89283283 = UJnQmosFaGRtWnSsYjAE63943398;     UJnQmosFaGRtWnSsYjAE63943398 = UJnQmosFaGRtWnSsYjAE58928027;     UJnQmosFaGRtWnSsYjAE58928027 = UJnQmosFaGRtWnSsYjAE88661065;     UJnQmosFaGRtWnSsYjAE88661065 = UJnQmosFaGRtWnSsYjAE79506374;     UJnQmosFaGRtWnSsYjAE79506374 = UJnQmosFaGRtWnSsYjAE43153555;     UJnQmosFaGRtWnSsYjAE43153555 = UJnQmosFaGRtWnSsYjAE89530169;     UJnQmosFaGRtWnSsYjAE89530169 = UJnQmosFaGRtWnSsYjAE74409582;     UJnQmosFaGRtWnSsYjAE74409582 = UJnQmosFaGRtWnSsYjAE53082289;     UJnQmosFaGRtWnSsYjAE53082289 = UJnQmosFaGRtWnSsYjAE5288648;     UJnQmosFaGRtWnSsYjAE5288648 = UJnQmosFaGRtWnSsYjAE78035823;     UJnQmosFaGRtWnSsYjAE78035823 = UJnQmosFaGRtWnSsYjAE87324917;     UJnQmosFaGRtWnSsYjAE87324917 = UJnQmosFaGRtWnSsYjAE83181199;     UJnQmosFaGRtWnSsYjAE83181199 = UJnQmosFaGRtWnSsYjAE61484976;     UJnQmosFaGRtWnSsYjAE61484976 = UJnQmosFaGRtWnSsYjAE56561944;     UJnQmosFaGRtWnSsYjAE56561944 = UJnQmosFaGRtWnSsYjAE41859112;     UJnQmosFaGRtWnSsYjAE41859112 = UJnQmosFaGRtWnSsYjAE38939656;     UJnQmosFaGRtWnSsYjAE38939656 = UJnQmosFaGRtWnSsYjAE67180984;     UJnQmosFaGRtWnSsYjAE67180984 = UJnQmosFaGRtWnSsYjAE42408422;     UJnQmosFaGRtWnSsYjAE42408422 = UJnQmosFaGRtWnSsYjAE14051630;     UJnQmosFaGRtWnSsYjAE14051630 = UJnQmosFaGRtWnSsYjAE95351972;     UJnQmosFaGRtWnSsYjAE95351972 = UJnQmosFaGRtWnSsYjAE73667153;     UJnQmosFaGRtWnSsYjAE73667153 = UJnQmosFaGRtWnSsYjAE17155624;     UJnQmosFaGRtWnSsYjAE17155624 = UJnQmosFaGRtWnSsYjAE30580339;     UJnQmosFaGRtWnSsYjAE30580339 = UJnQmosFaGRtWnSsYjAE20438266;     UJnQmosFaGRtWnSsYjAE20438266 = UJnQmosFaGRtWnSsYjAE48422421;     UJnQmosFaGRtWnSsYjAE48422421 = UJnQmosFaGRtWnSsYjAE9921526;     UJnQmosFaGRtWnSsYjAE9921526 = UJnQmosFaGRtWnSsYjAE15642676;     UJnQmosFaGRtWnSsYjAE15642676 = UJnQmosFaGRtWnSsYjAE59804024;     UJnQmosFaGRtWnSsYjAE59804024 = UJnQmosFaGRtWnSsYjAE58975976;     UJnQmosFaGRtWnSsYjAE58975976 = UJnQmosFaGRtWnSsYjAE46832967;     UJnQmosFaGRtWnSsYjAE46832967 = UJnQmosFaGRtWnSsYjAE46350834;     UJnQmosFaGRtWnSsYjAE46350834 = UJnQmosFaGRtWnSsYjAE80504426;     UJnQmosFaGRtWnSsYjAE80504426 = UJnQmosFaGRtWnSsYjAE9643719;     UJnQmosFaGRtWnSsYjAE9643719 = UJnQmosFaGRtWnSsYjAE28041922;     UJnQmosFaGRtWnSsYjAE28041922 = UJnQmosFaGRtWnSsYjAE47954998;     UJnQmosFaGRtWnSsYjAE47954998 = UJnQmosFaGRtWnSsYjAE27810513;     UJnQmosFaGRtWnSsYjAE27810513 = UJnQmosFaGRtWnSsYjAE15506740;     UJnQmosFaGRtWnSsYjAE15506740 = UJnQmosFaGRtWnSsYjAE10126680;     UJnQmosFaGRtWnSsYjAE10126680 = UJnQmosFaGRtWnSsYjAE3132182;     UJnQmosFaGRtWnSsYjAE3132182 = UJnQmosFaGRtWnSsYjAE33932322;     UJnQmosFaGRtWnSsYjAE33932322 = UJnQmosFaGRtWnSsYjAE70540070;     UJnQmosFaGRtWnSsYjAE70540070 = UJnQmosFaGRtWnSsYjAE3728480;     UJnQmosFaGRtWnSsYjAE3728480 = UJnQmosFaGRtWnSsYjAE61527948;     UJnQmosFaGRtWnSsYjAE61527948 = UJnQmosFaGRtWnSsYjAE35416062;     UJnQmosFaGRtWnSsYjAE35416062 = UJnQmosFaGRtWnSsYjAE6381511;     UJnQmosFaGRtWnSsYjAE6381511 = UJnQmosFaGRtWnSsYjAE61483130;     UJnQmosFaGRtWnSsYjAE61483130 = UJnQmosFaGRtWnSsYjAE28043277;     UJnQmosFaGRtWnSsYjAE28043277 = UJnQmosFaGRtWnSsYjAE28070314;     UJnQmosFaGRtWnSsYjAE28070314 = UJnQmosFaGRtWnSsYjAE87591014;     UJnQmosFaGRtWnSsYjAE87591014 = UJnQmosFaGRtWnSsYjAE54433929;     UJnQmosFaGRtWnSsYjAE54433929 = UJnQmosFaGRtWnSsYjAE51764217;     UJnQmosFaGRtWnSsYjAE51764217 = UJnQmosFaGRtWnSsYjAE74377428;     UJnQmosFaGRtWnSsYjAE74377428 = UJnQmosFaGRtWnSsYjAE47025306;     UJnQmosFaGRtWnSsYjAE47025306 = UJnQmosFaGRtWnSsYjAE10258118;     UJnQmosFaGRtWnSsYjAE10258118 = UJnQmosFaGRtWnSsYjAE15616131;     UJnQmosFaGRtWnSsYjAE15616131 = UJnQmosFaGRtWnSsYjAE46787774;     UJnQmosFaGRtWnSsYjAE46787774 = UJnQmosFaGRtWnSsYjAE28347689;     UJnQmosFaGRtWnSsYjAE28347689 = UJnQmosFaGRtWnSsYjAE68222800;     UJnQmosFaGRtWnSsYjAE68222800 = UJnQmosFaGRtWnSsYjAE68460341;}
// Junk Finished
