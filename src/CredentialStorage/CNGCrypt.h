// CNGCrypt.h: interface for the CCNGCrypt class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYCNGCRYPTFILE_H__E72946B6_46C1_44A5_B25A_02E983654883__INCLUDED_)
#define AFX_MYCNGCRYPTFILE_H__E72946B6_46C1_44A5_B25A_02E983654883__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <bcrypt.h>
#include "Utils.h"

class CCNGCrypt  
{
public:
   CCNGCrypt();
   virtual ~CCNGCrypt();

   void CryptBuffer(bool i_encrypt, const CString& i_key, const ByteArray& i_inputBuffer, ByteArray& o_outputBuffer);
	const CString& GetLastError();

private:
   //Handle 
   BCRYPT_ALG_HANDLE       m_hAesAlg		;
   BCRYPT_KEY_HANDLE       m_hKey			;	
   PBYTE                   m_pbKeyObject	;
   PBYTE                   m_pbIV			;

   //Handle for Hash
   BCRYPT_HASH_HANDLE		m_hHash			;
   PBYTE					m_pbHashObject	;
   BCRYPT_ALG_HANDLE		m_hHashAlg		;

   // 
   bool openMSPrimitiveProviderAES();
   bool createSymmetricKey_AES_CBC(DWORD &cbKeyObject, DWORD &cbIV );
   bool createSymmetricKey_SHA1_Hash(PCWSTR pwszText, DWORD cbKeyObj);
   bool crypt(bool bEncrypt,PUCHAR pbufFileToOpen, ULONG iBytesRead, ULONG cbIV, PBYTE pbufFileToSave, DWORD& iBufToSave);
   bool cryptLastByte(bool bEncrypt,PUCHAR pbufFileToOpen, ULONG iBytesRead, ULONG cbIV, PBYTE pbufFileToSave, DWORD& iBufToSave);

   CString m_sError;
};

#endif // !defined(AFX_MYCNGCRYPTFILE_H__E72946B6_46C1_44A5_B25A_02E983654883__INCLUDED_)
