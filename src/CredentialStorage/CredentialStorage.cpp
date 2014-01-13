// CredentialStorage.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <fstream>
#include <KnownFolders.h>
#include "Utils.h"
#include "CNGCrypt.h"
#include "RegistryHelper.h"
#include "CredentialStorage.h"

#define CREDENTIAL_DIRECTORY_PATH CStringW(L"Some Client\\AutoIntranetLogin")
#define USER_NAME_FILE "username.dat"
#define USER_PASS_FILE "password.dat"

#define CREDENTIAL_ROOT_REGISTRY_PATH std::wstring(L"SOFTWARE\\Some Clint")
#define CREDENTIAL_REGISTRY_PATH (CREDENTIAL_ROOT_REGISTRY_PATH + L"\\AutoIntranetLogin")
#define USER_NAME_ATTR L"Username"
#define USER_PASS_ATTR L"Password"
#define RUNONCE_REGISTRY_PATH L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce"
#define DESCRIPTION_ATTR L"AutomaticIntranetLogon"

#define CRYPT_KEY L"34080497d399"


CREDENTIALSTORAGE_API bool CLIB_StoreCredentials(const std::wstring& i_userName, const std::wstring& i_password, std::wstring& o_error)
{
   try
   {
      std::wstring fixedUserName = i_userName;

      // need to fix user name before saving "SOME CLIENT\iano" -> "iano"
      size_t slashPos = std::wstring::npos;
      if ((slashPos = fixedUserName.find_last_of(L'\\')) != std::wstring::npos)
      {
         fixedUserName = fixedUserName.substr(slashPos + 1);
      }
      else if ((slashPos = fixedUserName.find_last_of(L'/')) != std::wstring::npos)
      {
         fixedUserName = fixedUserName.substr(slashPos + 1);
      }

      ByteArray blobUserName = utils::String2ByteArray(CStringW(fixedUserName.c_str()));
      ByteArray blobUserPass = utils::String2ByteArray(CStringW(i_password.c_str()));
      ByteArray blobEncodedUserName;
      ByteArray blobEncodedUserPass;

      // ISSUE: Tried to use utils::GetMACAddressString() call for salt generating, but it looks does not work for my WiFi connection,
      //   this call generates empty string for CP provider that is storing credentials, it looks like this is because. Then this call will return 
      //   good MAC address when net connection is established
      CStringW userNameKey = CStringW(CRYPT_KEY); //utils::GetMACAddressString(); does not work for CP call
      CStringW userPassKey = userNameKey;

      CCNGCrypt crypter;
      crypter.CryptBuffer(true, userNameKey, blobUserName, blobEncodedUserName);
      crypter.CryptBuffer(true, userPassKey.MakeReverse(), blobUserPass, blobEncodedUserPass);

      //CRegistryHelper m_registry;
      //m_registry.open(HKEY_CURRENT_USER, CREDENTIAL_REGISTRY_PATH, true);
      
      //m_registry.write_value(USER_NAME_ATTR, blobEncodedUserName);
      //m_registry.write_value(USER_PASS_ATTR, blobEncodedUserPass);

      std::string path = utils::GetPublicDocumentsPath(CREDENTIAL_DIRECTORY_PATH);
      std::string usernamePath = path + "\\" + USER_NAME_FILE;
      std::string passwordPath = path + "\\" + USER_PASS_FILE;

      std::fstream myFile;
      myFile.open(usernamePath, std::fstream::out | std::fstream::trunc);
      myFile.close();
      myFile.open(usernamePath, std::fstream::out | std::fstream::binary);
      std::copy(blobEncodedUserName.begin(),blobEncodedUserName.end(),std::ostream_iterator<BYTE>(myFile));
      myFile.close();

      myFile.open(passwordPath, std::fstream::out | std::fstream::trunc);
      myFile.close();
      myFile.open(passwordPath, std::fstream::out | std::fstream::binary);
      std::copy(blobEncodedUserPass.begin(),blobEncodedUserPass.end(),std::ostream_iterator<BYTE>(myFile));
      myFile.close();
   }
   catch (const utils::CStorageException& e)
   {
      CStringW msg;
      msg.Format(L"%s(error (%d), message (%s))", 
         e.GetMessage().GetString(), 
         e.GetError(),
         e.GetSystemError().GetString());
      o_error = msg.GetString();
      return false;
   }

   return true;
}


CREDENTIALSTORAGE_API bool CLIB_LoadCredentials(std::wstring& o_userName, std::wstring& o_password, std::wstring& o_error)
{
   ByteArray blobEncodedUserName;
   ByteArray blobEncodedUserPass;
   ByteArray blobDecodedUserName;
   ByteArray blobDecodedUserPass;

   try
   {
      ///CRegistryHelper m_registry;
      //m_registry.open(HKEY_CURRENT_USER, CREDENTIAL_REGISTRY_PATH, false);
      //m_registry.read_value(USER_NAME_ATTR, blobEncodedUserName);
      //m_registry.read_value(USER_PASS_ATTR, blobEncodedUserPass);

      std::string path = utils::GetPublicDocumentsPath(CREDENTIAL_DIRECTORY_PATH);
      std::string usernamePath = path + "\\" + USER_NAME_FILE;
      std::string passwordPath = path + "\\" + USER_PASS_FILE;

      std::fstream myFile;
      myFile.open(usernamePath, std::fstream::in | std::fstream::binary);
      if (!myFile)
      {
         throw utils::CStorageException(L"Username file does not exist");
      }
      myFile.seekg(0, std::fstream::end);
      blobEncodedUserName.resize(myFile.tellg(), 0);
      myFile.seekg(0, std::fstream::beg);
      myFile.read((char*)&blobEncodedUserName[0], blobEncodedUserName.size());
      myFile.close();

      myFile.open(passwordPath, std::fstream::in | std::fstream::binary);
      if (!myFile)
      {
         throw utils::CStorageException(L"Password file does not exist");
      }
      myFile.seekg(0, std::fstream::end);
      blobEncodedUserPass.resize(myFile.tellg(), 0);
      myFile.seekg(0, std::fstream::beg);
      myFile.read((char*)&blobEncodedUserPass[0], blobEncodedUserPass.size());
      myFile.close();

      // ISSUE: Tried to use utils::GetMACAddressString() call for salt generating, but it looks does not work for my WiFi connection,
      //   this call generates empty string for CP provider that is storing credentials, it looks like this is because. Then this call will return 
      //   good MAC address when net connection is established
      CStringW userNameKey = CStringW(CRYPT_KEY); //utils::GetMACAddressString(); does not work for CP call
      CStringW userPassKey = userNameKey;

      CCNGCrypt crypter;
      crypter.CryptBuffer(false, userNameKey, blobEncodedUserName, blobDecodedUserName);
      crypter.CryptBuffer(false, userPassKey.MakeReverse(), blobEncodedUserPass, blobDecodedUserPass);

      CStringW userName = utils::ByteArray2String(blobDecodedUserName);
      CStringW userPass = utils::ByteArray2String(blobDecodedUserPass);

      o_userName = userName.GetString();
      o_password = userPass.GetString();
   }
   catch (const utils::CStorageException& e)
   {
      CStringW msg;
      msg.Format(L"%s(error (%d), message (%s))", 
         e.GetMessage().GetString(), 
         e.GetError(),
         e.GetSystemError().GetString());
      o_error = msg.GetString();
      return false;
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////
CREDENTIALSTORAGE_API bool CLIB_ClearCredentials(std::wstring& o_error)
{
   CLIB_LogMessage(L"CLIB_ClearCredentials is started ...");
   
   bool res = true;
   try
   {
      //CRegistryHelper::delete_key(HKEY_CURRENT_USER, CREDENTIAL_ROOT_REGISTRY_PATH);
      std::string path = utils::GetPublicDocumentsPath(CREDENTIAL_DIRECTORY_PATH);
      std::string usernamePath = path + "\\" + USER_NAME_FILE;
      std::string passwordPath = path + "\\" + USER_PASS_FILE;
      CLIB_LogMessage(L"CLIB_ClearCredentials: username path \"%S\"", usernamePath.c_str());
      CLIB_LogMessage(L"CLIB_ClearCredentials: password path \"%S\"", passwordPath.c_str());
      CLIB_LogMessage(L"CLIB_ClearCredentials is finished ...");

      std::fstream myFile;
      myFile.open(usernamePath, std::fstream::out | std::fstream::trunc);
      myFile.close();

      myFile.open(passwordPath, std::fstream::out | std::fstream::trunc);
      myFile.close();
   }
   catch (const utils::CStorageException& e)
   {
      CStringW msg;
      msg.Format(L"%s(error (%d), message (%s))", 
         e.GetMessage().GetString(), 
         e.GetError(),
         e.GetSystemError().GetString());
      o_error = msg.GetString();
      res = false;
   }

   CLIB_LogMessage(L"CLIB_ClearCredentials is finished ...");
   return res;
}

//////////////////////////////////////////////////////////////////////////
void doForEachUser(int i_num, bool i_success)
{
   if (i_success)
   {
      CLIB_LogMessage(L"For each user (session %d): username \"%s\"", i_num, utils::CAutoImpersonate::GetCurrentUserName().c_str());
      std::wstring commonDesktopPath = utils::GetKnownPath(FOLDERID_Desktop);
      std::wstring commonStartupPath = utils::GetKnownPath(FOLDERID_Startup);
      CLIB_LogMessage(L"For each user (session %d): desktop path \"%s\"", commonDesktopPath.c_str());
      CLIB_LogMessage(L"For each user (session %d): startup path \"%s\"", commonStartupPath.c_str());
   }
   else
   {
      CLIB_LogMessage(L"For each user (session %d): failed %d (%s)", i_num, GetLastError(), utils::GetSystemMessageW(GetLastError()).GetString());
   }
}

//////////////////////////////////////////////////////////////////////////
CREDENTIALSTORAGE_API bool CLIB_CreateShortcuts( const std::wstring& i_appName, std::wstring& o_error )
{
   CLIB_LogMessage(L"CLIB_CreateShortcuts is started ...");
   CLIB_LogMessage(L"Before impersonation: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());
   CoInitialize(NULL);

   bool res = true;
   try
   {
      utils::CAutoImpersonate imp;
      CLIB_LogMessage(L"After impersonation: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());

      //CRegistryHelper m_registry;
      //m_registry.open(HKEY_CURRENT_USER, RUNONCE_REGISTRY_PATH, true);
      //m_registry.write_value(DESCRIPTION_ATTR, i_appName);

      std::wstring system32Path = utils::GetKnownPath(FOLDERID_System);
      std::wstring appPath = system32Path + L"\\" + i_appName;
      std::wstring commonDesktopPath = utils::GetKnownPath(FOLDERID_Desktop);
      std::wstring commonStartupPath = utils::GetKnownPath(FOLDERID_Startup);

      std::wstring linkPathOnDesktop = commonDesktopPath + L"\\" + i_appName + L".lnk";
      std::wstring linkPathOnStartup = commonStartupPath + L"\\" + i_appName + L".lnk";
      CLIB_LogMessage(L"CLIB_CreateShortcuts: desktop link path \"%s\"", linkPathOnDesktop.c_str());
      CLIB_LogMessage(L"CLIB_CreateShortcuts: startup link path \"%s\"", linkPathOnStartup.c_str());
      utils::CreateLink(linkPathOnDesktop.c_str(), appPath.c_str(), system32Path.c_str(), L"Shortcut for IntranetLogon.exe"); 
      utils::CreateLink(linkPathOnStartup.c_str(), appPath.c_str(), system32Path.c_str(), L"Shortcut for IntranetLogon.exe"); 

   }
   catch (const utils::CStorageException& e)
   {
      CStringW msg;
      msg.Format(L"%s(error (%d), message (%s))", 
         e.GetMessage().GetString(), 
         e.GetError(),
         e.GetSystemError().GetString());
      o_error = msg.GetString();
      res = false;
      CLIB_LogMessage(msg);
   }

   CoUninitialize();
   CLIB_LogMessage(L"After reverting: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());
   CLIB_LogMessage(L"CLIB_CreateShortcuts is finished ...");
   return res;
}

//////////////////////////////////////////////////////////////////////////
CREDENTIALSTORAGE_API bool CLIB_DeleteShortcuts( const std::wstring& i_appName, std::wstring& o_error )
{
   CLIB_LogMessage(L"CLIB_DeleteShortcuts is started ...");
   CLIB_LogMessage(L"Before impersonation: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());

   CoInitialize(NULL);

   bool res = true;
   try
   {
      utils::CAutoImpersonate imp;
      CLIB_LogMessage(L"After impersonation: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());

      std::wstring commonDesktopPath = utils::GetKnownPath(FOLDERID_Desktop);
      std::wstring commonStartupPath = utils::GetKnownPath(FOLDERID_Startup);
      std::wstring linkPathOnDesktop = commonDesktopPath + L"\\" + i_appName + L".lnk";
      std::wstring linkPathOnStartup = commonStartupPath + L"\\" + i_appName + L".lnk";
      DeleteFileW(linkPathOnDesktop.c_str()); 
      DeleteFileW(linkPathOnStartup.c_str()); 
      CLIB_LogMessage(L"CLIB_DeleteShortcuts: desktop link path \"%s\"", linkPathOnDesktop.c_str());
      CLIB_LogMessage(L"CLIB_DeleteShortcuts: startup link path \"%s\"", linkPathOnStartup.c_str());

   }
   catch (const utils::CStorageException& e)
   {
      CStringW msg;
      msg.Format(L"%s(error (%d), message (%s))", 
         e.GetMessage().GetString(), 
         e.GetError(),
         e.GetSystemError().GetString());
      o_error = msg.GetString();
      res = false;
      CLIB_LogMessage(msg);
   }

   CoUninitialize();
   CLIB_LogMessage(L"After reverting: username \"%s\"", utils::CAutoImpersonate::GetCurrentUserName().c_str());
   CLIB_LogMessage(L"CLIB_DeleteShortcuts is finished ...");
   return res;
}

///////////////////////////////////////////////
CREDENTIALSTORAGE_API void CLIB_LogMessage ( const wchar_t* i_fmt, ...)
{
   wchar_t szMessageBuf[512] = {0};	/* Hope this is big enough */

   SYSTEMTIME stm;
   GetLocalTime(&stm);

   va_list pArg;
   va_start(pArg, i_fmt);
   _vsnwprintf_s(szMessageBuf, _TRUNCATE, i_fmt, pArg);
   va_end(pArg);

   try
   {
      std::string path = utils::GetPublicDocumentsPath(CREDENTIAL_DIRECTORY_PATH) + "\\cplog.txt";
      CStringW pathW(path.c_str());
      FILE *filea = NULL; 
      _wfopen_s(&filea, pathW, L"a+");
      fwprintf_s(filea, L"%02d-%02d-%04d %02d:%02d:%02d %s\n", 
         stm.wMonth, stm.wDay, stm.wYear, stm.wHour, stm.wMinute, stm.wSecond, szMessageBuf);
      fclose(filea);
   }
   catch (const utils::CStorageException& e)
   {
   }
}
