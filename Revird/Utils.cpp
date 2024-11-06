#include "Utils.hpp"

UNICODE_STRING CharToUnicode(const char* str)
{
    ANSI_STRING ansiString;
    UNICODE_STRING unicodeString;
    RtlInitAnsiString(&ansiString, str);
    RtlAnsiStringToUnicodeString(&unicodeString, &ansiString, TRUE);
    return unicodeString;
}