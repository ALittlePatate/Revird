#include "Registry.hpp"
#include "Output.hpp"

typedef unsigned __int64 QWORD;

NTSTATUS InitRegistry()
{
    UNICODE_STRING keyPath;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;
    UNICODE_STRING valueName;
    ULONG valueData;

    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    status = ZwCreateKey(&keyHandle, KEY_ALL_ACCESS, &objectAttributes, 0, NULL, REG_OPTION_NON_VOLATILE, NULL);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to create or open registry key: 0x%X\n", status);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Address");
    valueData = 0x000000000000000000000;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_QWORD, &valueData, sizeof(QWORD));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'address' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

	RtlInitUnicodeString(&valueName, L"Data");
    valueData = 0x0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'Data' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }


    RtlInitUnicodeString(&valueName, L"Size");
    valueData = 4;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_DWORD, &valueData, sizeof(valueData));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'size' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Go");
    valueData = 0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'go' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Operation"); //0 for RPM, 1 for WPM
    valueData = 0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'Operation' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Out");
    valueData = 0x00;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'out' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Unload");
    valueData = 0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'unload' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Process");
    valueData = 0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_SZ, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'Process' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    RtlInitUnicodeString(&valueName, L"Module");
    valueData = 0;
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_SZ, &valueData, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[INIT_REG] Failed to set 'Module' value: 0x%X\n", status);
        ZwClose(keyHandle);
        return status;
    }

    ZwClose(keyHandle);

    return STATUS_SUCCESS;
}

NTSTATUS QueryRegistryValue(const char* valueNameStr, PKEY_VALUE_PARTIAL_INFORMATION* kvpi, ULONG* resultLength)
{
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;
    static UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + 256] = { 0 };

    valueName = CharToUnicode(valueNameStr);
    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&keyHandle, KEY_READ, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[QUERY_REG] Failed to open registry key: 0x%X\n", status);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    status = ZwQueryValueKey(keyHandle, &valueName, KeyValuePartialInformation, buffer, sizeof(buffer), resultLength);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[QUERY_REG] Failed to query registry value: 0x%X\n", status);
        ZwClose(keyHandle);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    *kvpi = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;

    ZwClose(keyHandle);
    RtlFreeUnicodeString(&valueName);

    return STATUS_SUCCESS;
}

NTSTATUS GetValue(const char* valueNameStr, PVOID buffer, ULONG bufferSize)
{
    PKEY_VALUE_PARTIAL_INFORMATION kvpi;
    ULONG resultLength;
    NTSTATUS status = QueryRegistryValue(valueNameStr, &kvpi, &resultLength);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (kvpi->Type == REG_BINARY)
    {
        if (kvpi->DataLength <= bufferSize)
        {
            RtlCopyMemory(buffer, kvpi->Data, kvpi->DataLength);
        }
        else
        {
            status = STATUS_BUFFER_TOO_SMALL;
            DbgPrint_("[GET_BINARY_VALUE] Buffer too small: 0x%X\n", status);
        }
    }
    else
    {
        status = STATUS_OBJECT_TYPE_MISMATCH;
        DbgPrint_("[GET_BINARY_VALUE] Registry value type mismatch: 0x%X\n", status);
    }

    return status;
}

NTSTATUS GetValue(const char* valueNameStr, PWCHAR value, ULONG valueBufferSize)
{
    PKEY_VALUE_PARTIAL_INFORMATION kvpi;
    ULONG resultLength;
    NTSTATUS status = QueryRegistryValue(valueNameStr, &kvpi, &resultLength);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (kvpi->Type == REG_SZ)
    {
        if (kvpi->DataLength <= valueBufferSize)
        {
            RtlCopyMemory(value, kvpi->Data, kvpi->DataLength);
            value[kvpi->DataLength / sizeof(WCHAR)] = L'\0';
        }
        else
        {
            status = STATUS_BUFFER_TOO_SMALL;
            DbgPrint_("[GET_STRING_VALUE][SZ] Buffer too small: 0x%X\n", status);
        }
    }
    else
    {
        status = STATUS_OBJECT_TYPE_MISMATCH;
        DbgPrint_("[GET_STRING_VALUE][SZ] Registry value type mismatch: 0x%X\n", status);
    }

    return status;
}

NTSTATUS GetValue(const char* valueNameStr, ULONG* value)
{
    PKEY_VALUE_PARTIAL_INFORMATION kvpi;
    ULONG resultLength;
    NTSTATUS status = QueryRegistryValue(valueNameStr, &kvpi, &resultLength);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (kvpi->Type == REG_DWORD)
    {
        *value = *(ULONG*)(kvpi->Data);
    }
    else
    {
        status = STATUS_OBJECT_TYPE_MISMATCH;
        DbgPrint_("[GET_VALUE][DWORD] Registry value type mismatch: 0x%X\n", status);
    }

    return status;
}

NTSTATUS GetValue(const char* valueNameStr, ULONGLONG* value)
{
    PKEY_VALUE_PARTIAL_INFORMATION kvpi;
    ULONG resultLength;
    NTSTATUS status = QueryRegistryValue(valueNameStr, &kvpi, &resultLength);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (kvpi->Type == REG_QWORD)
    {
        *value = *(ULONGLONG*)(kvpi->Data);
    }
    else
    {
        status = STATUS_OBJECT_TYPE_MISMATCH;
        DbgPrint_("[GET_VALUE][QWORD] Registry value type mismatch: 0x%X\n", status);
    }

    return status;
}

NTSTATUS GetValue(const char* valueNameStr, PUCHAR value)
{
    PKEY_VALUE_PARTIAL_INFORMATION kvpi;
    ULONG resultLength;
    NTSTATUS status = QueryRegistryValue(valueNameStr, &kvpi, &resultLength);

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    if (kvpi->Type == REG_BINARY)
    {
        RtlCopyMemory(value, kvpi->Data, kvpi->DataLength);
    }
    else
    {
        status = STATUS_OBJECT_TYPE_MISMATCH;
        DbgPrint_("[GET_VALUE][BINARY] Registry value type mismatch or buffer too small: 0x%X\n", status);
    }

    return status;
}

NTSTATUS SetValue(const char* valueNameStr, PVOID buffer, ULONG bufferSize)
{
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;

    // Convert the ANSI string to a UNICODE string
    valueName = CharToUnicode(valueNameStr);
    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    // Open the registry key for writing
    status = ZwOpenKey(&keyHandle, KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SetValue][BINARY] Failed to open registry key for writing: 0x%X\n", status);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    // Write the buffer as a REG_BINARY value
    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, buffer, bufferSize);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SetValue][BINARY] Failed to set binary value: 0x%X\n", status);
    }

    // Clean up resources
    ZwClose(keyHandle);
    RtlFreeUnicodeString(&valueName);

    return status;
}

NTSTATUS SetValue(const char* valueNameStr, ULONG value)
{
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;

    valueName = CharToUnicode(valueNameStr);
    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&keyHandle, KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][DWORD] Failed to open registry key for writing: 0x%X\n", status);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_DWORD, &value, sizeof(value));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][DWORD] Failed to set DWORD value: 0x%X\n", status);
    }

    ZwClose(keyHandle);
    RtlFreeUnicodeString(&valueName);

    return status;
}

NTSTATUS SetValue(const char* valueNameStr, ULONGLONG value)
{
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;

    valueName = CharToUnicode(valueNameStr);
    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&keyHandle, KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][QWORD] Failed to open registry key for writing: 0x%X\n", status);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_QWORD, &value, sizeof(value));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][QWORD] Failed to set QWORD value: 0x%X\n", status);
    }

    ZwClose(keyHandle);
    RtlFreeUnicodeString(&valueName);

    return status;
}

NTSTATUS SetValue(const char* valueNameStr, UCHAR value)
{
    UNICODE_STRING keyPath;
    UNICODE_STRING valueName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE keyHandle;
    NTSTATUS status;

    valueName = CharToUnicode(valueNameStr);
    RtlInitUnicodeString(&keyPath, L"\\Registry\\Machine\\SOFTWARE\\Revird");
    InitializeObjectAttributes(&objectAttributes, &keyPath, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);

    status = ZwOpenKey(&keyHandle, KEY_WRITE, &objectAttributes);
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][BINARY] Failed to open registry key for writing: 0x%X\n", status);
        RtlFreeUnicodeString(&valueName);
        return status;
    }

    status = ZwSetValueKey(keyHandle, &valueName, 0, REG_BINARY, &value, sizeof(UCHAR));
    if (!NT_SUCCESS(status))
    {
        DbgPrint_("[SET_VALUE][BINARY] Failed to set binary value: 0x%X\n", status);
    }

    ZwClose(keyHandle);
    RtlFreeUnicodeString(&valueName);

    return status;
}
