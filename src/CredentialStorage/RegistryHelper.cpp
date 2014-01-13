#include "StdAfx.h"
#include "RegistryHelper.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
// class CRegistrySettings

CRegistryHelper::CRegistryHelper()
{
}

CRegistryHelper::~CRegistryHelper()
{
    close();
}

void CRegistryHelper::open(HKEY hKey, const std::wstring& path, bool i_create)
{
   bool opened = RegOpenKeyExW( hKey, path.c_str(), 0, KEY_READ|KEY_WRITE, &m_hKey ) == ERROR_SUCCESS;
   if (!opened && !i_create)
   {
      throw utils::CStorageException(L"CRegistryHelper::open: RegOpenKeyExW is failed");
   }

   if (!opened && RegCreateKeyW(hKey, path.c_str(), &m_hKey) != ERROR_SUCCESS)
   {
      throw utils::CStorageException(L"CRegistryHelper::open: RegCreateKeyW is failed");
   }
}

void CRegistryHelper::delete_key(HKEY hKey, const std::wstring& path)
{
   if (ERROR_SUCCESS != SHDeleteKeyW(hKey, path.c_str()))
   {
      throw utils::CStorageException(L"CRegistryHelper::delete_key: SHDeleteKey is failed");
   }
}

void CRegistryHelper::close()
{
    RegCloseKey( m_hKey );
}

void CRegistryHelper::read_value(const std::wstring& value_name, ByteArray &value)
{
    DWORD dwValueSize = 1000;
    auto_ptr<BYTE> pBuffer(new BYTE[dwValueSize]); 

    LONG lRetStatus = ERROR_SUCCESS;
    if (ERROR_SUCCESS != (lRetStatus = RegQueryValueExW(
        m_hKey,
        value_name.c_str(),
        NULL,
        NULL,
        (LPBYTE) pBuffer.get(),
        &dwValueSize)))
    {
        if (ERROR_MORE_DATA != lRetStatus)
        {
           throw utils::CStorageException(L"CRegistryHelper::read_value is failed");
        }

        // realloc buffer
        pBuffer = auto_ptr<BYTE>(new BYTE[dwValueSize]); 

        if (ERROR_SUCCESS != (lRetStatus = RegQueryValueExW(
            m_hKey,
            value_name.c_str(),
            NULL,
            NULL,
            (LPBYTE) pBuffer.get(),
            &dwValueSize)))
        {
           throw utils::CStorageException(L"CRegistryHelper::read_value: RegQueryValueExW is failed");
        }
    }

    if (dwValueSize > 0)
    {
       std::copy(pBuffer.get(), pBuffer.get() + dwValueSize, std::back_inserter(value));
    }
    else
    {
       CString msg;
       msg.Format(L"CRegistryHelper::read_value: %s value is empty", value_name.c_str());
       throw utils::CStorageException(msg);
    }
}

//////////////////////////////////////////////////////////////////////////
void CRegistryHelper::write_value(const std::wstring& value_name, const ByteArray &value)
{
   LONG lRetStatus = ERROR_SUCCESS;
   if (ERROR_SUCCESS != (lRetStatus = RegSetValueExW(
      m_hKey,
      value_name.c_str(),
      NULL,
      REG_NONE,
      (const BYTE *) &value[0],
      (DWORD)value.size())))
   {
      throw utils::CStorageException(L"CRegistryHelper::write_value: RegSetValueExW(REG_NONE) is failed");
   }
}

//////////////////////////////////////////////////////////////////////////
void CRegistryHelper::write_value(const std::wstring& value_name, const std::wstring &value)
{
   LONG lRetStatus = ERROR_SUCCESS;
   if (ERROR_SUCCESS != (lRetStatus = RegSetValueExW(
      m_hKey,
      value_name.c_str(),
      NULL,
      REG_SZ,
      (const BYTE *) value.c_str(),
      (DWORD)(value.size()*sizeof(wchar_t)))))
   {
      throw utils::CStorageException(L"CRegistryHelper::write_value: RegSetValueExW(REG_SZ) is failed");
   }
}