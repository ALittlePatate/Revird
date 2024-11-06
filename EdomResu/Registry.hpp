#pragma once
#include <iostream>
#include <Windows.h>

bool SetRegistryValue(LPCWSTR valueName, DWORD type, const void* data, DWORD dataSize);
bool SetRegistryValue(LPCWSTR valueName, const std::wstring& data);

bool GetRegistryValue(LPCWSTR valueName, DWORD type, void* data, DWORD dataSize);
bool GetRegistryValue(LPCWSTR valueName, std::wstring& data);
