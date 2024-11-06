#include "Memory.hpp"
#include "Registry.hpp"
#include "Output.hpp"

PEPROCESS last_process = nullptr;

static NTSTATUS KeReadVirtualMemory(PEPROCESS Process, PVOID SourceAddress, PVOID TargetAddress, SIZE_T Size)
{
	SIZE_T Bytes = 0;

	__try {
		ProbeForRead(SourceAddress, Size, (ULONG)Size);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint_("[KeReadVirtualMemory] Can't read at address 0x%p.\n", SourceAddress);
		return STATUS_ACCESS_VIOLATION;
	}

	if (NT_SUCCESS(MmCopyVirtualMemory(Process, SourceAddress, PsGetCurrentProcess(),
		TargetAddress, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

NTSTATUS KeWriteVirtualMemory(PEPROCESS Process, PVOID Source, PVOID Target, SIZE_T Size) {
	SIZE_T Bytes = 0;

	__try {
		ProbeForRead(Target, Size, (ULONG)Size);
	} __except (EXCEPTION_EXECUTE_HANDLER) {
		DbgPrint_("[KeReadVirtualMemory] Can't read at address 0x%p.\n", Target);
		return STATUS_ACCESS_VIOLATION;
	}

	if (NT_SUCCESS(MmCopyVirtualMemory(PsGetCurrentProcess(), Source, Process, Target, Size, KernelMode, &Bytes)))
		return STATUS_SUCCESS;
	else
		return STATUS_ACCESS_DENIED;
}

void RPM(QWORD address, ULONG size) {
	PVOID buffer = nullptr;

	DbgPrint_("[RPM] Reading %ld bytes at address 0x%p.\n", size, (void *)address);

	if (size <= 0) {
		DbgPrint_("[RPM] Size is not valid.\n");
		return;
	}

	if (last_process == nullptr) {
		DbgPrint_("[RPM] last_process was nulll.\n");
		return;
	}

	buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 'rpm');
	if (buffer == NULL)
	{
		DbgPrint_("[RPM] Failed to allocate buffer.\n");
		return;
	}

	NTSTATUS status = KeReadVirtualMemory(last_process, (void *)address, buffer, size);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[RPM] Could not read memory at 0x%p.\n", (void*)address);
		ExFreePool2(buffer, 'rpm', 0, 0);
	}

	DbgPrint_("[RPM] Success.\n");

	status = SetValue("Out", buffer, size);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[RPM] Failed to find SetValue\n");
		ExFreePool2(buffer, 'rpm', 0, 0);
		return;
	}

	ExFreePool2(buffer, 'rpm', 0, 0);
}

void WPM(QWORD address, ULONG size) {
	PVOID buffer = nullptr;

	if (last_process == nullptr) {
		DbgPrint_("[WPM] last_process was nulll.\n");
		return;
	}

	DbgPrint_("[WPM] Writing %ld bytes at address 0x%p.\n", size, (void *)address);
	if (size <= 0) {
		DbgPrint_("[WPM] Size or address is not valid.\n");
		return;
	}

	buffer = ExAllocatePool2(POOL_FLAG_NON_PAGED, size, 'wpm');
	if (buffer == NULL)
	{
		DbgPrint_("[WPM] Failed to allocate buffer.\n");
		return;
	}

	NTSTATUS status = GetValue("Data", buffer, size);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[WPM] Failed to GetValue.\n");
		ExFreePool2(buffer, 'wpm', 0, 0);
		return;
	}

	status = KeWriteVirtualMemory(last_process, buffer, (void*)address, size);
	if (!NT_SUCCESS(status))
	{
		DbgPrint_("[WPM] Could not write memory at 0x%p.\n", (void*)address);
		ExFreePool2(buffer, 'wpm', 0, 0);
	}

	DbgPrint_("[WPM] Success.\n");

	ExFreePool2(buffer, 'wpm', 0, 0);
}