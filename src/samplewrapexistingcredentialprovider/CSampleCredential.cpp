//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//


#ifndef WIN32_NO_STATUS
#include <ntstatus.h>
#define WIN32_NO_STATUS
#endif
#include <unknwn.h>
#include <string>

#include "CredentialStorage.h"
#include "CSampleCredential.h"
#include "CWrappedCredentialEvents.h"
#include "guid.h"
#include "utils.h"

#define PASSWORD_FIELD_ID 3


// CSampleCredential ////////////////////////////////////////////////////////

// NOTE: Please read the readme.txt file to understand when it's appropriate to
// wrap an another credential provider and when it's not.  If you have questions
// about whether your scenario is an appropriate use of wrapping another credprov,
// please contact credprov@microsoft.com
CSampleCredential::CSampleCredential():
_cRef(1)
{
   DllAddRef();

   _pWrappedCredential = NULL;
   _pWrappedCredentialEvents = NULL;
   _pCredProvCredentialEvents = NULL;

   _dwWrappedDescriptorCount = 0;
   CLIB_LogMessage(L"CSampleCredential::CSampleCredential()");
}

CSampleCredential::~CSampleCredential()
{
   _CleanupEvents();

   if (_pWrappedCredential)
   {
      _pWrappedCredential->Release();
   }

   DllRelease();
   CLIB_LogMessage(L"CSampleCredential::~CSampleCredential()");
}

// Initializes one credential with the field information passed in. We also keep track
// of our wrapped credential and how many fields it has.
HRESULT CSampleCredential::Initialize(
   __in ICredentialProviderCredential *pWrappedCredential,
   __in DWORD dwWrappedDescriptorCount
   )
{
   HRESULT hr = S_OK;

   // Grab the credential we're wrapping for future reference.
   if (_pWrappedCredential != NULL)
   {
      _pWrappedCredential->Release();
   }
   _pWrappedCredential = pWrappedCredential;
   _pWrappedCredential->AddRef();

   // We also need to remember how many fields the inner credential has.
   _dwWrappedDescriptorCount = dwWrappedDescriptorCount;

   CLIB_LogMessage(L"CSampleCredential::Initialize()");
   return hr;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of 
// anything. We'll also provide it to the wrapped credential.
HRESULT CSampleCredential::Advise(
   __in ICredentialProviderCredentialEvents* pcpce
   )
{
   HRESULT hr = S_OK;

   _CleanupEvents();

   // We keep a strong reference on the real ICredentialProviderCredentialEvents
   // to ensure that the weak reference held by the CWrappedCredentialEvents is valid.
   _pCredProvCredentialEvents = pcpce;
   _pCredProvCredentialEvents->AddRef();

   _pWrappedCredentialEvents = new CWrappedCredentialEvents();

   if (_pWrappedCredentialEvents != NULL)
   {
      _pWrappedCredentialEvents->Initialize(this, pcpce);

      if (_pWrappedCredential != NULL)
      {
         hr = _pWrappedCredential->Advise(_pWrappedCredentialEvents);
      }
   }
   else
   {
      hr = E_OUTOFMEMORY;
   }

   CLIB_LogMessage(L"CSampleCredential::Advise()");
   return hr;
}

// LogonUI calls this to tell us to release the callback. 
// We'll also provide it to the wrapped credential.
HRESULT CSampleCredential::UnAdvise()
{
   HRESULT hr = S_OK;

   if (_pWrappedCredential != NULL)
   {
      _pWrappedCredential->UnAdvise();
   }

   _CleanupEvents();
   CLIB_LogMessage(L"CSampleCredential::UnAdvise()");

   return hr;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the 
// field definitions. In fact, we're just going to hand it off to the
// wrapped credential in case it wants to do something.
HRESULT CSampleCredential::SetSelected(__out BOOL* pbAutoLogon)  
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->SetSelected(pbAutoLogon);
   }

   CLIB_LogMessage(L"CSampleCredential::SetSelected(autoLogon: %d)", *pbAutoLogon);

   return hr;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. We'll let the wrapped credential do anything it needs.
HRESULT CSampleCredential::SetDeselected()
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->SetDeselected();
      CLIB_LogMessage(L"CSampleCredential::SetDeselected()");
   }

   return hr;
}

// Get info for a particular field of a tile. Called by logonUI to get information to 
// display the tile. We'll check to see if it's for us or the wrapped credential, and then
// handle or route it as appropriate.
HRESULT CSampleCredential::GetFieldState(
   __in DWORD dwFieldID,
   __out CREDENTIAL_PROVIDER_FIELD_STATE* pcpfs,
   __out CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE* pcpfis
   )
{
   HRESULT hr = E_UNEXPECTED;


   // Make sure we have a wrapped credential.
   if (_pWrappedCredential != NULL)
   {
      // Validate parameters.
      if ((pcpfs != NULL) && (pcpfis != NULL))
      {
         hr = _pWrappedCredential->GetFieldState(dwFieldID, pcpfs, pcpfis);
         CLIB_LogMessage(L"CSampleCredential::GetFieldState(id: %d): state \'%s\', interactive state \'%s\'", 
            dwFieldID, utils::CPFieldStateString(*pcpfs), utils::CPFieldIntStateString(*pcpfis));
      }
      else
      {
         hr = E_INVALIDARG;
      }
   }
   return hr;
}

// Sets ppwsz to the string value of the field at the index dwFieldID. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CSampleCredential::GetStringValue(
   __in DWORD dwFieldID, 
   __deref_out PWSTR* ppwsz
   )
{
   HRESULT hr = E_UNEXPECTED;

   // Make sure we have a wrapped credential.
   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetStringValue(dwFieldID, ppwsz);

      CLIB_LogMessage(L"CSampleCredential::GetStringValue(id: %d): %s", dwFieldID, *ppwsz);
   }
   return hr;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the 
// currently selected item (pdwSelectedItem). We'll check to see if it's for us or the 
// wrapped credential, and then handle or route it as appropriate.
HRESULT CSampleCredential::GetComboBoxValueCount(
   __in DWORD dwFieldID, 
   __out DWORD* pcItems, 
   __out_range(<,*pcItems) DWORD* pdwSelectedItem
   )
{
   HRESULT hr = E_UNEXPECTED;

   // Make sure we have a wrapped credential.
   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetComboBoxValueCount(dwFieldID, pcItems, pdwSelectedItem);
   }

   return hr;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
// We'll check to see if it's for us or the wrapped credential, and then handle or route 
// it as appropriate.
HRESULT CSampleCredential::GetComboBoxValueAt(
   __in DWORD dwFieldID, 
   __in DWORD dwItem,
   __deref_out PWSTR* ppwszItem
   )
{
   HRESULT hr = E_UNEXPECTED;

   // Make sure we have a wrapped credential.
   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetComboBoxValueAt(dwFieldID, dwItem, ppwszItem);
   }

   return hr;
}

// Called when the user changes the selected item in the combobox. We'll check to see if 
// it's for us or the wrapped credential, and then handle or route it as appropriate.
HRESULT CSampleCredential::SetComboBoxSelectedValue(
   __in DWORD dwFieldID,
   __in DWORD dwSelectedItem
   )
{
   HRESULT hr = E_UNEXPECTED;

   // Make sure we have a wrapped credential.
   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->SetComboBoxSelectedValue(dwFieldID, dwSelectedItem);
   }

   return hr;
}

//------------- 
// The following methods are for logonUI to get the values of various UI elements and 
// then communicate to the credential about what the user did in that field. Even though
// we don't offer these field types ourselves, we need to pass along the request to the
// wrapped credential.

HRESULT CSampleCredential::GetBitmapValue(
   __in DWORD dwFieldID, 
   __out HBITMAP* phbmp
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetBitmapValue(dwFieldID, phbmp);
   }

   return hr;
}

HRESULT CSampleCredential::GetSubmitButtonValue(
   __in DWORD dwFieldID,
   __out DWORD* pdwAdjacentTo
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetSubmitButtonValue(dwFieldID, pdwAdjacentTo);
   }
   CLIB_LogMessage(L"CSampleCredential::GetSubmitButtonValue(id: %d): value %d\n", dwFieldID, *pdwAdjacentTo);

   return hr;
}

HRESULT CSampleCredential::SetStringValue(
   __in DWORD dwFieldID,
   __in PCWSTR pwz
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      if (dwFieldID != 3)
      {
         CLIB_LogMessage(L"CSampleCredential::SetStringValue(id: %d): \'%s\'", dwFieldID, pwz);
      }
      else
      {
         // here is password
         std::wstring pass( wcslen(pwz), L'*');
         CLIB_LogMessage(L"CSampleCredential::SetStringValue(id: %d): \'%s\'", dwFieldID, pass.c_str());
      }

      hr = _pWrappedCredential->SetStringValue(dwFieldID, pwz);

      if (SUCCEEDED(hr) && dwFieldID == PASSWORD_FIELD_ID)
      {
         _password = pwz;
         //CLIB_LogMessage(L"CSampleCredential::Saved password: \'%s\'", _password.c_str());
      }
   }

   return hr;
}

HRESULT CSampleCredential::GetCheckboxValue(
   __in DWORD dwFieldID, 
   __out BOOL* pbChecked,
   __deref_out PWSTR* ppwszLabel
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetCheckboxValue(dwFieldID, pbChecked, ppwszLabel);
   }
   CLIB_LogMessage(L"CSampleCredential::GetCheckboxValue(id: %d): checked %d", dwFieldID, *pbChecked);

   return hr;
}

HRESULT CSampleCredential::SetCheckboxValue(
   __in DWORD dwFieldID, 
   __in BOOL bChecked
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->SetCheckboxValue(dwFieldID, bChecked);
   }

   CLIB_LogMessage(L"CSampleCredential::SetCheckboxValue(id: %d): checked %d", dwFieldID, bChecked);

   return hr;
}

HRESULT CSampleCredential::CommandLinkClicked(__in DWORD dwFieldID)
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->CommandLinkClicked(dwFieldID);
   }
   CLIB_LogMessage(L"CSampleCredential::CommandLinkClicked(id: %d)", dwFieldID);

   return hr;
}
//------ end of methods for controls we don't have ourselves ----//


//
// Collect the username and password into a serialized credential for the correct usage scenario 
// (logon/unlock is what's demonstrated in this sample).  LogonUI then passes these credentials 
// back to the system to log on.
//
HRESULT CSampleCredential::GetSerialization(
   __out CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE* pcpgsr,
   __out CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION* pcpcs, 
   __deref_out_opt PWSTR* ppwszOptionalStatusText, 
   __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->GetSerialization(pcpgsr, pcpcs, ppwszOptionalStatusText, pcpsiOptionalStatusIcon);

   }

   return hr;
}

// ReportResult is completely optional. However, we will hand it off to the wrapped
// credential in case they want to handle it.
HRESULT CSampleCredential::ReportResult(
   __in NTSTATUS ntsStatus, 
   __in NTSTATUS ntsSubstatus,
   __deref_out_opt PWSTR* ppwszOptionalStatusText, 
   __out CREDENTIAL_PROVIDER_STATUS_ICON* pcpsiOptionalStatusIcon
   )
{
   HRESULT hr = E_UNEXPECTED;

   if (_pWrappedCredential != NULL)
   {
      hr = _pWrappedCredential->ReportResult(ntsStatus, ntsSubstatus, ppwszOptionalStatusText, pcpsiOptionalStatusIcon);
      CLIB_LogMessage(L"CSampleCredential::ReportResult(status: %s)", utils::NTStatusString(ntsStatus));
      
      CLIB_LogMessage(L"*****************************");
      CLIB_LogMessage(L"Store user credentials into registry ...");

      if ( SUCCEEDED(hr) )
      {
         PWSTR userName = NULL;
 
         bool bLoadedCredentials = true;
         if (FAILED(_pWrappedCredential->GetStringValue(0, &userName)))
         {
            CLIB_LogMessage(L"Failed to get user name or password from default credential provider");
            bLoadedCredentials = false;
         }

         if (bLoadedCredentials)
         {
            utils::ProcessCredentials(userName, _password.c_str());
         }
      }

      CLIB_LogMessage(L"*****************************");
   }


   return hr;
}

void CSampleCredential::_CleanupEvents()
{
   // Call Uninitialize before releasing our reference on the real 
   // ICredentialProviderCredentialEvents to avoid having an
   // invalid reference.
   if (_pWrappedCredentialEvents != NULL)
   {
      _pWrappedCredentialEvents->Uninitialize();
      _pWrappedCredentialEvents->Release();
      _pWrappedCredentialEvents = NULL;
   }

   if (_pCredProvCredentialEvents != NULL)
   {
      _pCredProvCredentialEvents->Release();
      _pCredProvCredentialEvents = NULL;
   }
}

