// CredentialStorageTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <assert.h>
#include <iostream>
#include "CredentialStorage.h"


int _tmain(int argc, _TCHAR* argv[])
{
   std::wstring err;
   std::wstring user(L"Long long name of the user");
   std::wstring pass(L"Long long password of the user");

   CStringW testMessage(L"Some test message");
   CLIB_LogMessage(L"Test log message: %s", testMessage.GetString());
   
   std::wcout << L"Encode '" << user << "' len " << user.length() << "\n"; 
   std::wcout << L"Encode '" << pass << "' len " << pass.length() << "\n"; 
   if (!CLIB_StoreCredentials(user, pass, err))
   {
      std::wcout << L"StoreCredentials is failed: " << err; 
   }
   
   std::wstring user2;
   std::wstring pass2;
   if (!CLIB_LoadCredentials(user2, pass2, err))
   {
      std::wcout << L"LoadCredentials is failed: " << err; 
   }

   std::wcout << L"Decode '" << user2 << "' len " << user2.length() << "\n"; 
   std::wcout << L"Decode '" << pass2 << "' len " << pass2.length() << "\n"; 
   
   assert(user2 == user && pass2 == pass);

   if (!CLIB_CreateShortcuts(L"IntraneLogon.exe", err))
   {
      std::wcout << L"CLIB_CreateShortcuts is failed: " << err; 
   }

   if (!CLIB_DeleteShortcuts(L"IntraneLogon.exe", err))
   {
      std::wcout << L"CLIB_DeleteShortcuts is failed: " << err; 
   }

	return 0;
}

