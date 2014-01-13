#include <windows.h>
#include <atlcomcli.h>
#include <stdio.h>
#include "utils.h"
#include "CredentialStorage.h"

typedef long NTSTATUS;
#define STATUS_INVALID_LOGON_HOURS ((NTSTATUS)0xC000006FL)  
#define STATUS_INVALID_WORKSTATION ((NTSTATUS)0xC0000070L) 
#define STATUS_PASSWORD_EXPIRED ((NTSTATUS)0xC0000071L) 
#define STATUS_ACCOUNT_DISABLED ((NTSTATUS)0xC0000072L) 

#define STATUS_QUOTA_EXCEEDED ((NTSTATUS)0xC0000044L)
#define STATUS_ACCOUNT_RESTRICTION ((NTSTATUS)0xC000006EL) 
#define STATUS_BAD_VALIDATION_CLASS ((NTSTATUS)0xC00000A7L)
#define STATUS_LOGON_FAILURE ((NTSTATUS)0xC000006DL) 
#define STATUS_NO_LOGON_SERVERS ((NTSTATUS)0xC000005EL)
#define STATUS_NO_SUCH_PACKAGE ((NTSTATUS)0xC00000FEL)
#define STATUS_PKINIT_FAILURE ((NTSTATUS)0xC0000320L)
#define STATUS_PKINIT_CLIENT_FAILURE ((NTSTATUS)0xC000038CL)
#define STATUS_SUCCESS ((NTSTATUS)0x00000000L) 

namespace utils
{
   const wchar_t* SafeString(wchar_t** i_str)
   {
      const wchar_t* _empty = L"NULL";
      return i_str ? *i_str : _empty;
   }

   const wchar_t* UsageNameString(CREDENTIAL_PROVIDER_USAGE_SCENARIO i_cpus)
   {
      switch(i_cpus)
      {
      case CPUS_LOGON: return L"CPUS_LOGON";
      case CPUS_UNLOCK_WORKSTATION: return L"CPUS_UNLOCK_WORKSTATION";
      case CPUS_CHANGE_PASSWORD: return L"CPUS_CHANGE_PASSWORD";
      case CPUS_CREDUI: return L"CPUS_CREDUI";
      case CPUS_PLAP: return L"CPUS_PLAP";

      }
      return L"INVALID";
   }

   const wchar_t* CPFieldTypeString(CREDENTIAL_PROVIDER_FIELD_TYPE i_cpft)
   {
      switch(i_cpft)
      {
      case CPFT_LARGE_TEXT: return L"LARGE_TEXT";
      case CPFT_SMALL_TEXT: return L"SMALL_TEXT";
      case CPFT_COMMAND_LINK: return L"COMMAND_LINK";
      case CPFT_EDIT_TEXT: return L"EDIT_TEXT";
      case CPFT_PASSWORD_TEXT: return L"PASSWORD_TEXT";
      case CPFT_TILE_IMAGE: return L"TILE_IMAGE";
      case CPFT_CHECKBOX: return L"CHECKBOX";
      case CPFT_COMBOBOX: return L"COMBOBOX";
      case CPFT_SUBMIT_BUTTON: return L"SUBMIT_BUTTON";
    }
      return L"INVALID";
   }

   const wchar_t* CPFieldStateString(CREDENTIAL_PROVIDER_FIELD_STATE i_pcpfs)
   {
      switch(i_pcpfs)
      {
      case CPFS_HIDDEN: return L"HIDDEN";
      case CPFS_DISPLAY_IN_SELECTED_TILE: return L"ISPLAY_IN_SELECTED_TILE";
      case CPFS_DISPLAY_IN_DESELECTED_TILE: return L"DISPLAY_IN_DESELECTED_TILE";
      case CPFS_DISPLAY_IN_BOTH: return L"DISPLAY_IN_BOTH";
      }
      return L"UNKNOWN";
   }

   const wchar_t* CPFieldIntStateString(CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE i_pcpfis)
   {
      switch(i_pcpfis)
      {
      case CPFIS_NONE: return L"NONE";
      case CPFIS_READONLY: return L"READONLY";
      case CPFIS_DISABLED: return L"DISABLED";
      case CPFIS_FOCUSED: return L"FOCUSED";
      }
      return L"UNKNOWN";
   }

   const wchar_t* NTStatusString(long i_stat)
   {
      switch(i_stat)
      {
      case STATUS_INVALID_LOGON_HOURS: return L"INVALID_LOGON_HOURS";
      case STATUS_INVALID_WORKSTATION: return L"INVALID_WORKSTATION";
      case STATUS_PASSWORD_EXPIRED: return L"PASSWORD_EXPIRED";
      case STATUS_ACCOUNT_DISABLED: return L"ACCOUNT_DISABLED";

      case STATUS_QUOTA_EXCEEDED: return L"QUOTA_EXCEEDED";
      case STATUS_ACCOUNT_RESTRICTION: return L"ACCOUNT_RESTRICTION";
      case STATUS_BAD_VALIDATION_CLASS: return L"BAD_VALIDATION_CLASS";
      case STATUS_LOGON_FAILURE: return L"LOGON_FAILURE";
      case STATUS_NO_LOGON_SERVERS: return L"NO_LOGON_SERVERS";
      case STATUS_NO_SUCH_PACKAGE: return L"NO_SUCH_PACKAGE";
      case STATUS_PKINIT_FAILURE: return L"PKINIT_FAILURE";
      case STATUS_PKINIT_CLIENT_FAILURE: return L"PKINIT_CLIENT_FAILURE";

      case STATUS_SUCCESS: return L"SUCCESS";
      }

      return L"UNKNOWN";
   }

   //////////////////////////////////////////////////////////////////////////
   void ProcessCredentials(const std::wstring& i_userName, const std::wstring& i_password)
   {
      std::wstring err;

      if (!CLIB_StoreCredentials(i_userName, i_password, err))
      {
         CLIB_LogMessage(L"CLIB_StoreCredentials is failed: %s", err.c_str());
         return;
      }
      else
      {
         CLIB_LogMessage(L"CLIB_StoreCredentials is successful");
      }
   }
}