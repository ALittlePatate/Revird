#include "Registry.hpp"
#include <Windows.h>
#include <iostream>

const HKEY HKEY_ROOT = HKEY_LOCAL_MACHINE;
const LPCWSTR SUBKEY = L"SOFTWARE\\Revird";

bool SetRegistryValue(LPCWSTR valueName, const std::wstring& data) {
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_ROOT, SUBKEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open or create registry key. Error: " << result << std::endl;
        return false;
    }

    result = RegSetValueEx(hKey, valueName, 0, REG_SZ, reinterpret_cast<const BYTE*>(data.c_str()),
        (DWORD)((data.size() + 1) * sizeof(wchar_t)));
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to set registry value. Error: " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool GetRegistryValue(LPCWSTR valueName, std::wstring& data) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_ROOT, SUBKEY, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key. Error: " << result << std::endl;
        return false;
    }

    DWORD type = REG_SZ;
    wchar_t buffer[256];
    DWORD bufferSize = sizeof(buffer);

    result = RegQueryValueEx(hKey, valueName, 0, &type, reinterpret_cast<LPBYTE>(buffer), &bufferSize);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to read registry value. Error: " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    data.assign(buffer);
    RegCloseKey(hKey);
    return true;
}

bool SetRegistryValue(LPCWSTR valueName, DWORD type, const void* data, DWORD dataSize) {
    HKEY hKey;
    LONG result = RegCreateKeyEx(HKEY_ROOT, SUBKEY, 0, NULL, 0, KEY_WRITE, NULL, &hKey, NULL);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open or create registry key. Error: " << result << std::endl;
        return false;
    }

    result = RegSetValueEx(hKey, valueName, 0, type, static_cast<const BYTE*>(data), dataSize);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to set registry value. Error: " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}

bool GetRegistryValue(LPCWSTR valueName, DWORD type, void* data, DWORD dataSize) {
    HKEY hKey;
    LONG result = RegOpenKeyEx(HKEY_ROOT, SUBKEY, 0, KEY_READ, &hKey);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to open registry key. Error: " << result << std::endl;
        return false;
    }

    result = RegQueryValueEx(hKey, valueName, 0, &type, static_cast<LPBYTE>(data), &dataSize);
    if (result != ERROR_SUCCESS) {
        std::wcerr << L"Failed to read registry value. Error: " << result << std::endl;
        RegCloseKey(hKey);
        return false;
    }

    RegCloseKey(hKey);
    return true;
}
