#pragma once
#include "Utils.h"

class CRegistryHelper
{
public:
    CRegistryHelper();
    virtual ~CRegistryHelper();

    void open(HKEY hKey, const std::wstring& path, bool i_create);
    static void delete_key(HKEY hKey, const std::wstring& path);
    void close();

    void read_value(const std::wstring& value_name, ByteArray &value);
    void write_value(const std::wstring& value_name, const ByteArray &value);
    void write_value(const std::wstring& value_name, const std::wstring &value);

protected:
    HKEY m_hKey;
};

