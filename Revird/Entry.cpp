#include <ntifs.h>
#include <wdm.h>

#include "Registry.hpp"
#include "Dispatcher.hpp"
#include "Output.hpp"

VOID Mainloop(IN PVOID Context)
{
	UNREFERENCED_PARAMETER(Context);
	UCHAR unload = false;
	UCHAR go = false;
	NTSTATUS status;

	DbgPrint_("[MAINLOOP] Hello from MainLoop.\n");
    DbgPrint_("[MAINLOOP] Init registry...\n");
    InitRegistry();
    DbgPrint_("[MAINLOOP] ok.\n");

	do {
		status = GetValue("Unload", &unload);
		if (!NT_SUCCESS(status)) {
			DbgPrint_("[MAINLOOP] Failed to GetValue.\n");
			break;
		}

		status = GetValue("Go", &go);
		if (!NT_SUCCESS(status)) {
			DbgPrint_("[MAINLOOP] Failed to GetValue.\n");
			break;
		}
		if (go) {
			DbgPrint_("[MAINLOOP] \"Go\" order.");
			DispatchOperation();
			SetValue("Go", (UCHAR)false);
		}
	} while (!unload);

	DbgPrint_("[MAINLOOP] Received unload order.\n");
	SetValue("Unload", (UCHAR)false);

    PsTerminateSystemThread(STATUS_SUCCESS);
}

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT     DriverObject, _In_ PUNICODE_STRING    RegistryPath)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	UNREFERENCED_PARAMETER(DriverObject);

	HANDLE hMainLoop = nullptr;

	DbgPrint_("[ENTRY] Driver loaded !\n");
	NTSTATUS status = PsCreateSystemThread(&hMainLoop, THREAD_ALL_ACCESS, NULL, NULL, NULL, (PKSTART_ROUTINE)Mainloop, NULL);
	if (!NT_SUCCESS(status)) {
		DbgPrint_("[ENTRY] MainLoop thread creation failed.\n");
		return STATUS_FAIL_CHECK;
	}
	DbgPrint_("[ENTRY] MainLoop thread created.\n");

	ZwClose(hMainLoop);

    return STATUS_SUCCESS;
}