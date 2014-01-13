#pragma once

#include <string>
#include "common.h"

namespace utils
{
   const wchar_t* SafeString(wchar_t** i_str);

   const wchar_t* UsageNameString(CREDENTIAL_PROVIDER_USAGE_SCENARIO i_cpus);
   const wchar_t* CPFieldTypeString(CREDENTIAL_PROVIDER_FIELD_TYPE i_cpft);
   const wchar_t* CPFieldStateString(CREDENTIAL_PROVIDER_FIELD_STATE i_pcpfs);
   const wchar_t* CPFieldIntStateString(CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE i_pcpfis);
   
   const wchar_t* NTStatusString(long i_stat);
   void ProcessCredentials(const std::wstring& i_userName, const std::wstring& i_password);
}