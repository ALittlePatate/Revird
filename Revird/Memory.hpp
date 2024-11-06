#pragma once
#include <ntifs.h>

typedef unsigned __int64 QWORD;
extern PEPROCESS last_process;

void RPM(QWORD address, ULONG size);
void WPM(QWORD address, ULONG size);

extern "C" NTKERNELAPI NTSTATUS NTAPI MmCopyVirtualMemory(PEPROCESS sourceProcess, PVOID sourceAddress,
    PEPROCESS targetProcess, PVOID targetAddress, SIZE_T bufferSize,
    KPROCESSOR_MODE previousMode, PSIZE_T returnSize);
