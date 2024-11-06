#pragma once
#include "Utils.hpp"

NTSTATUS InitRegistry();
NTSTATUS QueryRegistryValue(const char* valueNameStr, PKEY_VALUE_PARTIAL_INFORMATION* kvpi, ULONG* resultLength);

NTSTATUS GetValue(const char* valueNameStr, PVOID buffer, ULONG bufferSize);
NTSTATUS GetValue(const char* valueNameStr, PWCHAR value, ULONG valueBufferSize);
NTSTATUS GetValue(const char* valueNameStr, ULONG* value);
NTSTATUS GetValue(const char* valueNameStr, ULONGLONG* value);
NTSTATUS GetValue(const char* valueNameStr, PUCHAR value);

NTSTATUS SetValue(const char* valueNameStr, PVOID buffer, ULONG bufferSize);
NTSTATUS SetValue(const char* valueNameStr, ULONG value);
NTSTATUS SetValue(const char* valueNameStr, ULONGLONG value);
NTSTATUS SetValue(const char* valueNameStr, UCHAR value);
