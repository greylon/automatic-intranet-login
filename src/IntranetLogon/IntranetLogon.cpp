// IntranetLogon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "CredentialStorage.h"

void runLogonScript(const std::wstring& i_username, const std::wstring& i_password)
{
   std::wstring cmdLine(L"IntranetLogonScript.vbs");
   cmdLine += L" \"";
   cmdLine += i_username;
   cmdLine += L"\" \"";
   cmdLine += i_password;
   cmdLine += L"\"";

   if(!ShellExecuteW(NULL, L"open", L"wscript.exe", const_cast<wchar_t*>(cmdLine.c_str()), NULL,SW_SHOW ) ) 
   {
      MessageBoxW(NULL, L"Automatic logon is failed (see cplog.text)", L"Logon error", MB_ICONERROR);
      CLIB_LogMessage(L"Fail to execute command: %s", cmdLine.c_str());
   }
}

int _tmain(int argc, _TCHAR* argv[])
{
   CLIB_LogMessage(L"***************************");
   std::wstring err;

   if (argc == 2 && std::wstring(argv[1]) == L"-uninstall")
   {
      CLIB_LogMessage(L"Run intranet logon tool (uninstall mode) ...");
#ifdef _DEBUG
      MessageBoxW(NULL, L"Run intranet logon tool (uninstall mode)", L"Info", MB_ICONERROR);
#endif      
      if (!CLIB_ClearCredentials(err))
      {
         CLIB_LogMessage(L"CLIB_ClearCredentials is failed: %s", err.c_str());
      }
      if (!CLIB_DeleteShortcuts(L"IntranetLogon.exe", err))
      {
         CLIB_LogMessage(L"CLIB_DeleteShortcuts is failed: %s", err.c_str());
      }
   }
   else if (argc == 2 && std::wstring(argv[1]) == L"-install")
   {
      CLIB_LogMessage(L"Run intranet logon tool (install mode) ...");
#ifdef _DEBUG
      MessageBoxW(NULL, L"Run intranet logon tool (install mode)", L"Info", MB_ICONERROR);
#endif      
      if (!CLIB_CreateShortcuts(L"IntranetLogon.exe", err))
      {
         CLIB_LogMessage(L"CLIB_CreateShortcuts is failed: %s", err.c_str());
      }
   }
   else
   {
      CLIB_LogMessage(L"Run intranet logon tool (script launch mode) ...");

      std::wstring user;
      std::wstring pass;
      if (!CLIB_LoadCredentials(user, pass, err))
      {
         MessageBoxW(NULL, L"Automatic logon is failed, try to relogon (see cplog.text)", L"Logon error", MB_ICONERROR);
         CLIB_LogMessage(L"CLIB_LoadCredentials is failed: %s", err.c_str());
      }
      else
      {
         runLogonScript(user, pass);
      }
   }


   CLIB_LogMessage(L"Finish intranet logon tool ...");
   CLIB_LogMessage(L"***************************");
	return 0;
}

