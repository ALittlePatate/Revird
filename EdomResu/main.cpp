#include <stdio.h>
#include "Operations.hpp"

int main(void) {
	QWORD proc_addr = GetProcess(L"explorer.exe");
	QWORD mod_addr = GetProcessModule(L"explorer.exe", L"kernel32.dll");
	BYTE mz = RPM<BYTE>(proc_addr);

	printf("Address of explorer.exe : 0x%p\n", (void*)proc_addr);
	printf("Address of kernel32.dll in explorer.exe : 0x%p\n", (void*)mod_addr);
	printf("Header of explorer.exe : 0x%X\n", mz);
	printf("Unloading driver...");
	DriverUnload();
	printf("ok.\n");
	return 0;
}