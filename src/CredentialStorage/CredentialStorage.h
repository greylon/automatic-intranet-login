// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CREDENTIALSTORAGE_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CREDENTIALSTORAGE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef CREDENTIALSTORAGE_DLL
#ifdef CREDENTIALSTORAGE_EXPORTS
#define CREDENTIALSTORAGE_API __declspec(dllexport)
#else
#define CREDENTIALSTORAGE_API __declspec(dllimport)
#endif
#else
#define CREDENTIALSTORAGE_API
#endif

#include <string>

extern "C"
{
   CREDENTIALSTORAGE_API bool CLIB_StoreCredentials(const std::wstring& i_username, const std::wstring& i_password, std::wstring& o_error);
   CREDENTIALSTORAGE_API bool CLIB_LoadCredentials(std::wstring& o_username, std::wstring& o_password, std::wstring& o_error);
   CREDENTIALSTORAGE_API bool CLIB_ClearCredentials(std::wstring& o_error);
   CREDENTIALSTORAGE_API bool CLIB_CreateShortcuts( const std::wstring& i_appName, std::wstring& o_error );
   CREDENTIALSTORAGE_API bool CLIB_DeleteShortcuts( const std::wstring& i_appName, std::wstring& o_error );
   CREDENTIALSTORAGE_API void CLIB_LogMessage ( const wchar_t* i_fmt, ...);
};
