#include "Operations.hpp"
#include "Registry.hpp"

QWORD GetProcess(const std::wstring &process_name) {
	bool status = true;

	status = SetRegistryValue(L"Process", process_name);
	if (!status) return 0;

	BYTE operation = OPERATION_GET_PROCESS;
	status = SetRegistryValue(L"Operation", REG_BINARY, &operation, sizeof(BYTE));
	if (!status) return 0;

	BYTE go = 0x01;
	status = SetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
	if (!status) return 0;

	do {
		status = GetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
		if (!status) return 0;
	} while (go == 0x01);

	QWORD res = 0;
	GetRegistryValue(L"Out", REG_QWORD, &res, sizeof(QWORD));

	return res;
}

QWORD GetProcessModule(const std::wstring &process_name, const std::wstring &module_name) {
	bool status = true;

	status = SetRegistryValue(L"Process", process_name);
	if (!status) return 0;

	status = SetRegistryValue(L"Module", module_name);
	if (!status) return 0;

	BYTE operation = OPERATION_GET_PROCESS_MODULE;
	status = SetRegistryValue(L"Operation", REG_BINARY, &operation, sizeof(BYTE));
	if (!status) return 0;

	BYTE go = 0x01;
	status = SetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
	if (!status) return 0;

	do {
		status = GetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
		if (!status) return 0;
	} while (go == 0x01);

	QWORD res = 0;
	GetRegistryValue(L"Out", REG_QWORD, &res, sizeof(QWORD));

	return res;
}

void DriverUnload(void) {
	bool status = true;

	BYTE unload = 0x01;
	status = SetRegistryValue(L"Unload", REG_BINARY, &unload, sizeof(BYTE));
	if (!status) return;

	do {
		status = GetRegistryValue(L"Unload", REG_BINARY, &unload, sizeof(BYTE));
		if (!status) return;
	} while (unload == 0x01);
}