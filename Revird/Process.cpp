#include "Process.hpp"
#include "Memory.hpp"
#include "Registry.hpp"
#include "Output.hpp"

extern "C" NTKERNELAPI PPEB NTAPI PsGetProcessPeb(IN PEPROCESS Process);
extern "C" NTKERNELAPI PVOID PsGetProcessSectionBaseAddress(__in PEPROCESS Process);

NTSTATUS GetProcessBaseAddress(PEPROCESS Process, PVOID* BaseAddress) {
	PVOID BaseAddress_ = PsGetProcessSectionBaseAddress(Process);
	if (BaseAddress_ != 0) {
		*BaseAddress = BaseAddress_;
		return STATUS_SUCCESS;
	}
	return STATUS_NOT_FOUND;
}

// if you crash in this function, update the offsets of the EPROCESS structure
// get it from here: https://www.vergiliusproject.com/kernels/x64/windows-11/23h2/_EPROCESS
NTSTATUS PsLookupProcessByProcessName(CHAR* ProcessName, PEPROCESS* Process) {
	if (ProcessName == nullptr || Process == nullptr)
		return STATUS_FWP_NULL_POINTER;

	PEPROCESS SystemProcess = PsInitialSystemProcess;
	PEPROCESS CurrentEntry = SystemProcess;
	CHAR ImageName[15] = { 0 };
	do {
		RtlCopyMemory((PVOID)(&ImageName), (PVOID)((ULONG64)CurrentEntry + 0x5a8) /*EPROCESS->ImageFileName*/, sizeof(ImageName));
		if (strstr(ImageName, ProcessName)) {
			ULONG ActiveThreads;
			RtlCopyMemory((PVOID)&ActiveThreads, (PVOID)((ULONG64)CurrentEntry + 0x5f0) /*EPROCESS->ActiveThreads*/, sizeof(ActiveThreads));
			if (ActiveThreads) {
				*Process = CurrentEntry;
				return STATUS_SUCCESS;
			}
		}
		PLIST_ENTRY list = (PLIST_ENTRY)((ULONG64)(CurrentEntry)+0x448); /*EPROCESS->ActiveProcessLinks*/;
		CurrentEntry = (PEPROCESS)((ULONG64)list->Flink - 0x448);
	} while (CurrentEntry != SystemProcess);
	return STATUS_NOT_FOUND;
}

NTSTATUS PsGetModuleBaseAddress(PEPROCESS TargetProcess, LPCWSTR ModuleName, PULONG64 Result) {
	KeAttachProcess(TargetProcess);

	PPEB PEB = PsGetProcessPeb(TargetProcess);
	if (!PEB) {
		KeDetachProcess();
		ObfDereferenceObject(TargetProcess);
		return STATUS_NOT_FOUND;
	}

	if (!PEB->Ldr || !PEB->Ldr->Initialized) {
		KeDetachProcess();
		ObfDereferenceObject(TargetProcess);
		return STATUS_NOT_FOUND;
	}

	UNICODE_STRING ModuleNameU;
	RtlInitUnicodeString(&ModuleNameU, ModuleName);
	for (PLIST_ENTRY PList = PEB->Ldr->InLoadOrderModuleList.Flink; PList != &PEB->Ldr->InLoadOrderModuleList; PList = PList->Flink) {
		PLDR_DATA_TABLE_ENTRY PLDREntry = CONTAINING_RECORD(PList, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
		if (RtlCompareUnicodeString(&PLDREntry->BaseDllName, &ModuleNameU, TRUE) == 0) {
			*Result = (ULONG64)PLDREntry->DllBase;
			KeDetachProcess();
			ObfDereferenceObject(TargetProcess);
			return STATUS_SUCCESS;
		}
	}
	return STATUS_NOT_FOUND;
}

void GetProcess(PWCHAR process) {
    PEPROCESS eProcess;
    NTSTATUS status;
	CHAR ansiProcessName[MODULE_NAME_SIZE];
	size_t convertedChars = 0;

	DbgPrint_("[GetProcess] Called, process: %ws\n", process);

	status = RtlUnicodeToMultiByteN(
		ansiProcessName,
		sizeof(ansiProcessName),
		(PULONG)&convertedChars,
		process,
		MODULE_NAME_SIZE);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcess] Failed to convert process name to ANSI, status: 0x%X\n", status);
		return;
	}

    status = PsLookupProcessByProcessName(ansiProcessName, &eProcess);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[GetProcess] Failed to find process: %ws, status: 0x%X\n", process, status);
        return;
    }

	void *addr = 0;
	status = GetProcessBaseAddress(eProcess, &addr);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[GetProcess] Failed to find process base address.\n");
		ObDereferenceObject(eProcess);
		return;
	}

    DbgPrint_("[GetProcess] Process: %ws, Base Address: 0x%p\n", process, (PVOID)addr);
	last_process = eProcess;

	status = SetValue("Out", (QWORD)addr);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcess] Failed to find SetValue\n");
		ObDereferenceObject(eProcess);
		return;
	}

    ObDereferenceObject(eProcess);
}

void GetProcessModule(PWCHAR process, PWCHAR module) {
	PEPROCESS eProcess;
	NTSTATUS status;
	ULONG64 addr = 0;
	size_t convertedChars = 0;
	CHAR ansiProcessName[MODULE_NAME_SIZE];

	DbgPrint_("[GetProcessModule] Called, process: %ws module: %ws\n", process, module);

	status = RtlUnicodeToMultiByteN(
		ansiProcessName,
		sizeof(ansiProcessName),
		(PULONG)&convertedChars,
		process,
		MODULE_NAME_SIZE);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcessModule] Failed to convert process name to ANSI, status: 0x%X\n", status);
		return;
	}

	status = PsLookupProcessByProcessName(ansiProcessName, &eProcess);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcessModule] Failed to find process: %ws, status: 0x%X\n", process, status);
		return;
	}

	DbgPrint_("[GetProcessModule] Process: %ws, Base Address: 0x%p\n", process, (PVOID)eProcess);
	last_process = eProcess;

	status = PsGetModuleBaseAddress(eProcess, module, &addr);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcessModule] Failed to find module %ws in process %ws, status: 0x%X\n", module, process, status);
		ObDereferenceObject(eProcess);
		return;
	}

	DbgPrint_("[GetProcessModule] Module : %ws, Base Address: 0x%p\n", module, (PVOID)addr);

	status = SetValue("Out", (QWORD)addr);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[GetProcessModule] Failed to find SetValue\n");
		ObDereferenceObject(eProcess);
		return;
	}

	ObDereferenceObject(eProcess);
}