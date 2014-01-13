#include "StdAfx.h"
#include <nb30.h>
#include <assert.h>
#include <shlobj.h>
#include <Wtsapi32.h>
#include <psapi.h>
#include "Utils.h"

namespace
{
   typedef struct _ASTAT_
   {
      ADAPTER_STATUS adapt;
      NAME_BUFFER NameBuff [30];
   }
   ASTAT, * PASTAT;

   std::vector<UCHAR> enum_lan_adapters()
   {
      std::vector<UCHAR> lans;

      NCB Ncb;
      UCHAR uRetCode;

      LANA_ENUM lana_enum = {0};

      memset( &Ncb, 0, sizeof (Ncb) );
      Ncb.ncb_command = NCBENUM;
      Ncb.ncb_buffer = (PUCHAR) &lana_enum;
      Ncb.ncb_length = sizeof(lana_enum);

      uRetCode = Netbios( &Ncb );
      if ( 0 == uRetCode )
      {
         std::multiset<UCHAR> sorted_adapt;
         std::copy(lana_enum.lana, lana_enum.lana + lana_enum.length, std::inserter(sorted_adapt, sorted_adapt.begin()));
         std::copy(sorted_adapt.begin(), sorted_adapt.end(), std::back_inserter(lans));
      }

/*
      Debug << "INSTALLER CRYPTO HELPER --> Found " 
         << (long) lans.size() 
         << " lan adapters (ret code: " << uRetCode << ") --> Adapters "
         << converter::BinaryToString(btstring(lana_enum.lana, lana_enum.length), converter::CTYPE_HEX)
         << "\n"; 
*/

      return lans;
   }

   ByteArray get_MAC_address(BYTE i_nAdapterNum)
   {
      NCB Ncb;
      UCHAR uRetCode;

      memset( &Ncb, 0, sizeof(Ncb) );
      Ncb.ncb_command = NCBRESET;
      Ncb.ncb_lana_num = i_nAdapterNum;

      uRetCode = Netbios( &Ncb );

      memset( &Ncb, 0, sizeof (Ncb) );
      Ncb.ncb_command = NCBASTAT;
      Ncb.ncb_lana_num = i_nAdapterNum;

      ASTAT Adapter;
      std::string callname("* ");
      memcpy( Ncb.ncb_callname, callname.c_str(), callname.size() );
      Ncb.ncb_buffer = (PUCHAR) &Adapter;
      Ncb.ncb_length = sizeof(Adapter);

      uRetCode = Netbios( &Ncb );
      if ( uRetCode == 0 )
      {
         /*
         wchar_t NetName[50];
         swprintf(NetName, L"%02x%02x%02x%02x%02x%02x",
         Adapter.adapt.adapter_address[0],
         Adapter.adapt.adapter_address[1],
         Adapter.adapt.adapter_address[2],
         Adapter.adapt.adapter_address[3],
         Adapter.adapt.adapter_address[4],
         Adapter.adapt.adapter_address[5] );*/
         UCHAR (&adapter_address)[6] = Adapter.adapt.adapter_address;
         return ByteArray(adapter_address, adapter_address + _countof(adapter_address));
      }

      return ByteArray();
   }

}

namespace utils
{
   ByteArray FindMACAddress()
   {
      std::vector<UCHAR> lans = enum_lan_adapters();
      std::vector<UCHAR>::iterator iter = lans.begin();
      for (; iter != lans.end(); ++iter)
      {
         ByteArray address = get_MAC_address(*iter);
         if (!address.empty()) 
         {
            return address;
         }
      }
      return ByteArray();
   }

   CStringW GetMACAddressString()
   {
      CStringW macAddressStr;
      ByteArray macAddress = FindMACAddress();
      for (int i = 0; i < macAddress.size(); ++i)
      {
         macAddressStr.AppendFormat(L"%02x", macAddress[i]);
      }
      return macAddressStr;
   }

   ByteArray String2ByteArray( const CStringW& i_str )
   {
      ByteArray blob;

      const BYTE* buf = (const BYTE*) i_str.GetString();
      const int buf_len = (i_str.GetLength() + 1) * sizeof(WCHAR);
      
      std::copy(buf, buf+buf_len, std::back_inserter(blob));
      return blob;
   }

   CStringW ByteArray2String( const ByteArray& i_blob )
   {
      if (i_blob.empty() || i_blob.size()%sizeof(WCHAR) != 0)
      {
         return CStringW();

      }
      return CStringW((const wchar_t*)&i_blob[0], i_blob.size()/sizeof(WCHAR));
   }

   CStringW GetSystemMessageW( DWORD iCode )
   {
      std::wstring message;
      WCHAR* messageBuffer;

      if( FormatMessageW( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, iCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&messageBuffer, 0, NULL ) )
      {
         message = messageBuffer;
         LocalFree( messageBuffer );
      }

      while( !message.empty() && ( wcschr( L"\r\n" , message[message.size() - 1] ) != NULL ) )
      {
         message.resize( message.size() - 1 );
      }

      return message.c_str();
   }

   CStringA GetSystemMessageA( DWORD iCode )
   {
      CStringW msg = GetSystemMessageW(iCode);
      return CStringA(msg);
   }

   //////////////////////////////////////////////////////////////////////////
   CStorageException::CStorageException(const CStringW& i_message, DWORD i_errorCode)
   : m_message(i_message)
   , m_systemError(GetSystemMessageW(i_errorCode))
   , m_errorCode(i_errorCode)
   {

   }

#pragma comment (lib, "Psapi.lib")

   #define MAX_PROCESSES 1024
   //////////////////////////////////////////////////////////////////////////
   DWORD FindProcess(__in_z LPCTSTR lpcszFileName) 
   { 
      LPDWORD lpdwProcessIds; 
      LPTSTR  lpszBaseName; 
      HANDLE  hProcess; 
      DWORD   i, cdwProcesses, dwProcessId = 0; 

      lpdwProcessIds = (LPDWORD)HeapAlloc(GetProcessHeap(), 0, MAX_PROCESSES*sizeof(DWORD)); 
      if (lpdwProcessIds != NULL) 
      { 
         if (EnumProcesses(lpdwProcessIds, MAX_PROCESSES*sizeof(DWORD), &cdwProcesses)) 
         { 
            lpszBaseName = (LPTSTR)HeapAlloc(GetProcessHeap(), 0, MAX_PATH*sizeof(TCHAR)); 
            if (lpszBaseName != NULL) 
            { 
               cdwProcesses /= sizeof(DWORD); 
               for (i = 0; i < cdwProcesses; i++) 
               { 
                  hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, lpdwProcessIds[i]); 
                  if (hProcess != NULL) 
                  { 
                     if (GetModuleBaseName(hProcess, NULL, lpszBaseName, MAX_PATH) > 0) 
                     { 
                        if (!lstrcmpi(lpszBaseName, lpcszFileName)) 
                        { 
                           dwProcessId = lpdwProcessIds[i]; 
                           CloseHandle(hProcess); 
                           break; 
                        } 
                     } 
                     CloseHandle(hProcess); 
                  } 
               } 
               HeapFree(GetProcessHeap(), 0, (LPVOID)lpszBaseName); 
            } 
         } 
         HeapFree(GetProcessHeap(), 0, (LPVOID)lpdwProcessIds); 
      } 
      return dwProcessId; 
   }

   //////////////////////////////////////////////////////////////////////////
   CAutoImpersonate::CAutoImpersonate()
   {
      DWORD processID = FindProcess(L"explorer.exe");
      if (processID == 0)
      {
         throw CStorageException(L"FindProcess is failed");
      }

      LUID seDebug;
      TOKEN_PRIVILEGES tkp;
      HANDLE hTokenCurr;
      OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY,&hTokenCurr);
      LookupPrivilegeValue(NULL,SE_DEBUG_NAME,&seDebug);
      tkp.PrivilegeCount=1;
      tkp.Privileges[0].Luid=seDebug;
      tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
      AdjustTokenPrivileges(hTokenCurr,FALSE,&tkp,sizeof tkp,NULL,NULL);
      CloseHandle(hTokenCurr);


      HANDLE hProcess = OpenProcess ( PROCESS_ALL_ACCESS, TRUE, processID ); 
      if (hProcess == NULL)
      {
         throw CStorageException(L"OpenProcess is failed");
      }

      if (!OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_DUPLICATE, &hPToken))
      {
         throw CStorageException(L"OpenProcessToken is failed");
      }

      if (!ImpersonateLoggedOnUser(hPToken))
      {
         throw CStorageException(L"ImpersonateLoggedOnUser is failed");
      }
   }

   CAutoImpersonate::~CAutoImpersonate()
   {
      RevertToSelf();
      CloseHandle(hPToken);
   }

   std::wstring CAutoImpersonate::GetCurrentUserName()
   {
      WCHAR usernameTemp[500] = {0};
      DWORD nameLen = 500;
      GetUserNameW(usernameTemp, &nameLen);
      return usernameTemp;
   }

   //////////////////////////////////////////////////////////////////////////
   std::string GetPublicDocumentsPath(const CStringW& i_appName)
   {
      CString path;
      wchar_t* defaultPath = NULL;
      HRESULT hr = SHGetKnownFolderPath(FOLDERID_PublicDocuments, 0, NULL, &defaultPath); 
      if (FAILED(hr))
      {
         throw utils::CStorageException(L"SHGetKnownFolderPath() is failed");
      }
      path = defaultPath;
      CoTaskMemFree(static_cast<void*>(defaultPath));

      if (!i_appName.IsEmpty())
      {
         path += L"\\";
         path += i_appName;
         int res = SHCreateDirectory(NULL, path);
         if (ERROR_SUCCESS != res && ERROR_FILE_EXISTS != res && ERROR_ALREADY_EXISTS != res)
         {
            throw utils::CStorageException(L"SHCreateDirectory() is failed", res);
         }
      }
      return CStringA(path).GetString();
   }

   //////////////////////////////////////////////////////////////////////////
   std::wstring GetKnownPath(REFKNOWNFOLDERID rfid)
   {
	   // FOLDERID_CommonStartup
      std::wstring path;
      wchar_t* defaultPath = NULL;
      HRESULT hr = SHGetKnownFolderPath(rfid, 0, NULL, &defaultPath); 
      if (FAILED(hr))
      {
         throw utils::CStorageException(L"SHGetKnownFolderPath() is failed");
      }
      path = defaultPath;
      CoTaskMemFree(static_cast<void*>(defaultPath));
      return path;
   }


   void CreateLink(LPCWSTR lpszPathLink, LPCWSTR lpszPathObj, LPCWSTR lpszWorkingDir, LPCWSTR lpszDesc) 
   { 
      HRESULT hres; 
      IShellLink* psl; 

      // Get a pointer to the IShellLink interface. It is assumed that CoInitialize
      // has already been called.
      hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl); 
      if (FAILED(hres)) 
      { 
         throw utils::CStorageException(L"CoCreateInstance() is failed", hres);
      }
      IPersistFile* ppf; 

      // Set the path to the shortcut target and add the description. 
      psl->SetPath(lpszPathObj); 
      psl->SetWorkingDirectory(lpszWorkingDir); 
      psl->SetDescription(lpszDesc); 

      // Query IShellLink for the IPersistFile interface, used for saving the 
      // shortcut in persistent storage. 
      hres = psl->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf); 
      if (FAILED(hres)) 
      { 
         psl->Release(); 
         throw utils::CStorageException(L"QueryInterface() is failed", hres);
      }

      if (SUCCEEDED(hres)) 
      { 
         // Save the link by calling IPersistFile::Save. 
         hres = ppf->Save(lpszPathLink, TRUE); 
         ppf->Release(); 
      } 

      psl->Release(); 
   }


}