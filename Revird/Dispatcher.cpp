#include "Dispatcher.hpp"
#include "Registry.hpp"
#include "Operation.hpp"
#include "Memory.hpp"
#include "Process.hpp"
#include "Output.hpp"
#include <ntifs.h>

void DispatchOperation(void) {
	UCHAR op = 0;
	QWORD address = 0;
	ULONG size = 0;
	WCHAR process[MODULE_NAME_SIZE] = { 0 };
	WCHAR module[MODULE_NAME_SIZE] = { 0 };

	NTSTATUS status = GetValue("Operation", &op);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[DispatchOperation] Failed to GetValue.\n");
		return;
	}

	status = GetValue("Address", &address);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[DispatchOperation] Failed to GetValue.\n");
		return;
	}

	status = GetValue("Size", &size);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[DispatchOperation] Failed to GetValue.\n");
		return;
	}

	status = GetValue("Process", process, MODULE_NAME_SIZE);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[DispatchOperation] Failed to GetValue.\n");
		return;
	}

	status = GetValue("Module", module, MODULE_NAME_SIZE);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[DispatchOperation] Failed to GetValue.\n");
		return;
	}

	switch (op) {
	case OPERATION_WPM:
		WPM(address, size);
		break;
	case OPERATION_RPM:
		RPM(address, size);
		break;
	case OPERATION_GET_PROCESS:
		GetProcess(process);
		break;
	case OPERATION_GET_PROCESS_MODULE:
		GetProcessModule(process, module);
		break;
	default:
		DbgPrint_("[DispatchOperation] Operation \"%d\" is not a valid operation.\n", op);
		return;
	}

	return;
}