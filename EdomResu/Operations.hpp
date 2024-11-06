#pragma once
#include <Windows.h>
#include <iostream>

#include "Registry.hpp"

typedef unsigned __int64 QWORD;

enum Operation {
    OPERATION_WPM,
    OPERATION_RPM,
    OPERATION_GET_PROCESS,
    OPERATION_GET_PROCESS_MODULE
};

template<typename TYPE>
TYPE RPM(QWORD address) {
    TYPE buffer = { 0 };
    bool status = true;

    BYTE operation = OPERATION_RPM;
    status = SetRegistryValue(L"Operation", REG_BINARY, &operation, sizeof(BYTE));
    if (!status) return 0;

    SIZE_T size = sizeof(TYPE);
    status = SetRegistryValue(L"Size", REG_DWORD, &size, sizeof(DWORD));
    if (!status) return 0;

    status = SetRegistryValue(L"Address", REG_QWORD, &address, sizeof(QWORD));
    if (!status) return 0;

    BYTE go = 0x01;
    status = SetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
    if (!status) return 0;

    do {
        status = GetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
        if (!status) return 0;
    } while (go == 0x01);

    GetRegistryValue(L"Out", REG_QWORD, &buffer, sizeof(QWORD));

    return buffer;
}

template<typename TYPE>
void WPM(QWORD address, TYPE data) {
    bool status = true;

    BYTE operation = OPERATION_WPM;
    status = SetRegistryValue(L"Operation", REG_BINARY, &operation, sizeof(BYTE));
    if (!status) return;

    SIZE_T size = sizeof(TYPE);
    status = SetRegistryValue(L"Size", REG_DWORD, &size, sizeof(DWORD));
    if (!status) return;

    status = SetRegistryValue(L"Address", REG_QWORD, &address, sizeof(QWORD));
    if (!status) return;

    status = SetRegistryValue(L"Data", REG_BINARY, &data, sizeof(TYPE));
    if (!status) return;

    BYTE go = 0x01;
    status = SetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
    if (!status) return;

    do {
        status = GetRegistryValue(L"Go", REG_BINARY, &go, sizeof(BYTE));
        if (!status) return;
    } while (go == 0x01);
}

QWORD GetProcess(const std::wstring& process_name);
QWORD GetProcessModule(const std::wstring& process_name, const std::wstring& module_name);
void DriverUnload(void);
