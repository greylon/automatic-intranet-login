// CNGCrypt.cpp: implementation of the CCNGCrypt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CNGCrypt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//#include <NTSTATUS.H>//Problem found cut and paste
//Manaully include in the project settings "C:\Program Files\Microsoft CNG Development Kit\Include"
// "C:\Program Files\Microsoft CNG Development Kit\Lib\X86"

//TODO DA controllare cosa serve veramente

#ifndef NTSTATUS
typedef LONG NTSTATUS, *PNSTATUS;
#endif

#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS                  ((NTSTATUS)0x00000000L)
#define STATUS_NOT_SUPPORTED            ((NTSTATUS)0xC00000BBL)
#define STATUS_UNSUCCESSFUL             ((NTSTATUS)0xC0000001L)
//#define STATUS_INVALID_PARAMETER        ((NTSTATUS)0xC000000DL)
#define STATUS_HMAC_NOT_SUPPORTED       ((NTSTATUS)0xC000A001L)
#define STATUS_BUFFER_TOO_SMALL         ((NTSTATUS)0xC0000023L)
#define STATUS_NOT_IMPLEMENTED          ((NTSTATUS)0xC0000002L)
//#define STATUS_ENTRYPOINT_NOT_FOUND     ((NTSTATUS)0xC0000139l)
#endif

const DWORD CHUNK_SIZE = 16;

#define SYMM_KEY_SIZE_SECRET 16

//Set IV
static const BYTE rgbIV[] =
{
   0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

//Set AES KEY
static const BYTE rgbAES128Key[] =
{
   'D', 'r', '.', 'L', 'u', 'i', 'j', 'i', 
   'C', 'N', 'G', ' ', 'T', 'e', 's', 't'
};
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

/*
To use this class in vc++ 2005 you must add to the project proprieties this:
1. At C++ General in the "Additional Include Directories" this path "C:\Program Files\Microsoft CNG Development Kit\Include" 
2. At "Linker General" in the "Additional Library Directories" this path "C:\Program Files\Microsoft CNG Development Kit\Lib\X86"
3. At "Linker Input" "Additional Dependecies" "bcrypt.lib"
*/


CCNGCrypt::CCNGCrypt()
{
	m_sError.Empty();
	m_hAesAlg			= NULL;
	m_hKey				= NULL;
	m_pbKeyObject		= NULL;	
	m_pbIV				= NULL;

	//Hash
	m_hHash				= NULL;
	m_pbHashObject		= NULL;
	m_hHashAlg			= NULL;
	
}

CCNGCrypt::~CCNGCrypt()
{

    if(m_hAesAlg)
    {
        BCryptCloseAlgorithmProvider(m_hAesAlg,0);
    }

    if (m_hKey)    
    {
        BCryptDestroyKey(m_hKey);
    }

    if(m_pbKeyObject)
    {
        HeapFree(GetProcessHeap(), 0, m_pbKeyObject);
    }

    if(m_pbIV!=NULL)
    {
        HeapFree(GetProcessHeap(), 0, m_pbIV);
    }

	//Hash
	if (m_hHash )
        BCryptDestroyHash(m_hHash);

    if ( m_pbHashObject )
        free(m_pbHashObject);

    if ( m_hHashAlg )
        BCryptCloseAlgorithmProvider(m_hHashAlg,0);
}

//////////////////////////////////////////////////////////////////////////
void CCNGCrypt::CryptBuffer(bool i_encrypt, const CString& i_key, const ByteArray& i_inputBuffer, ByteArray& o_outputBuffer)
{
	NTSTATUS	ntStatus			= STATUS_UNSUCCESSFUL;
	DWORD		cbKeyObject			= 0;
	DWORD		cbIV          = 0;
	BOOL		bFileToOpen			= FALSE;
	BOOL		bFileToCrypt		= FALSE;
	
	UINT		iBytesRead			=0;
	BYTE		pbufToOpen[CHUNK_SIZE];
	BYTE		pbufFileToSave[CHUNK_SIZE*2];
	
	if (!openMSPrimitiveProviderAES())
   {
		throw utils::CStorageException(m_sError);
   }

/*	if(!createSymmetricKey_AES_CBC(cbKeyObject,cbIV))
		return false;
*/
	if(!createSymmetricKey_SHA1_Hash(i_key, cbKeyObject))
   {
      throw utils::CStorageException(m_sError);
   }
	
   ByteArray::const_iterator iter = i_inputBuffer.begin();

	while ( iter != i_inputBuffer.end() )
	{			
      DWORD iBufToSave=0;

      size_t restDistance = std::distance(iter, i_inputBuffer.end());
      if (restDistance >= CHUNK_SIZE)
      {
         iBytesRead = CHUNK_SIZE;
      }
      else
      {
         iBytesRead = restDistance;
      }

      memcpy_s(pbufToOpen, _countof(pbufToOpen), (BYTE*)&*iter, iBytesRead);
      iter += iBytesRead;

		if (restDistance >= CHUNK_SIZE)
		{
			if (!crypt(i_encrypt,pbufToOpen,iBytesRead,cbIV,pbufFileToSave,iBufToSave))
         {
            throw utils::CStorageException(m_sError);
         }

         std::copy(pbufFileToSave, pbufFileToSave + iBytesRead, back_inserter(o_outputBuffer));
		}	
		else //(!LastCryptBuffRead())
		{	
			if(!cryptLastByte(i_encrypt,pbufToOpen,iBytesRead,cbIV,pbufFileToSave,iBufToSave))
         {
            throw utils::CStorageException(m_sError);
         }
         
         std::copy(pbufFileToSave, pbufFileToSave + iBufToSave, back_inserter(o_outputBuffer));
		}
	}
}

//////////////////////////////////////////////////////////////////////////
const CString& CCNGCrypt::GetLastError()
{
	return m_sError;
}

//////////////////////////////////////////////////////////////////////////
bool CCNGCrypt::openMSPrimitiveProviderAES()
{
	NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
	ntStatus  = BCryptOpenAlgorithmProvider( &m_hAesAlg, BCRYPT_AES_ALGORITHM, NULL, 0);
	switch (ntStatus)
	{
	case STATUS_SUCCESS:
		return true;
/*	from msdn but not in NTSTATUS.H
	case STATUS_NOT_FOUND:
		m_sError.Format(_T("Error opening The algorithm handle. BCryptOpenAlgorithmProvider no provider was found for the specified algorithm ID. "));
		break;
*/
	case STATUS_INVALID_PARAMETER:
		m_sError.Format(_T("Error opening The algorithm handle. BCryptOpenAlgorithmProvider one or more parameters are not valid. "));
		break;
	case STATUS_NO_MEMORY:
		m_sError.Format(_T("Error opening The algorithm handle. BCryptOpenAlgorithmProvider a memory allocation failure occurred. "));
		break;
	default:
		m_sError.Format(_T("Error opening The algorithm handle. BCryptOpenAlgorithmProvider return with error 0x%08x"), ntStatus);      
		break;		
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CCNGCrypt::crypt(bool bEncrypt,PUCHAR pbufFileToOpen, ULONG iBytesRead, ULONG cbIV, PBYTE pbufFileToSave, DWORD& iBufToSave)
{
	NTSTATUS ntStatus =STATUS_UNSUCCESSFUL;	
	DWORD		cbCipherText		= 0;
	if ( bEncrypt ) 					
		ntStatus = BCryptEncrypt( m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV,  pbufFileToSave, iBytesRead, &iBufToSave,0);
	else		
		ntStatus = BCryptDecrypt( m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV, pbufFileToSave, iBytesRead, &iBufToSave, 0);

	switch(ntStatus)
	{
	case STATUS_SUCCESS:

		return true;


	case STATUS_BUFFER_TOO_SMALL :
		m_sError.Format(_T("Error %s The size specified by the cbOutput parameter is not large enough to hold the ciphertext. Return with error 0x%08x"), bEncrypt?_T("BCryptEncrypt"):_T("BCryptDecrypt"),  ntStatus);

		break;
		//Line found on msdn but not in bcrypt.h
		//			case STATUS_INVALID_BUFFER_SIZE :
		//				m_sError.Format(_T("Error %s The cbInput parameter is not a multiple of the algorithm's block size and the BCRYPT_BLOCK_PADDING or the BCRYPT_PAD_NONE flag was not specified in the dwFlags parameter. Return with error 0x%08x"), bEncrypt?"BCryptEncrypt":"BCryptDecrypt",  ntStatus);
		//				break;
		//			case STATUS_INVALID_HANDLE :
		//				m_sError.Format(_T("Error %s The key handle in the hKey parameter is not valid. ", bEncrypt?"BCryptEncrypt":"BCryptDecrypt",  ntStatus);
		//					break;
	case STATUS_INVALID_PARAMETER :
		m_sError.Format(_T("Error %s One or more parameters are not valid. Return with error 0x%08x"), bEncrypt?"BCryptEncrypt":"BCryptDecrypt",  ntStatus);
		break;
	case STATUS_NOT_SUPPORTED :
		m_sError.Format(_T("Error %s The algorithm does not support encryption. Return with error 0x%08x"), bEncrypt?"BCryptEncrypt":"BCryptDecrypt",  ntStatus);
		break;
	default:
		m_sError.Format(_T("Error %s return with error 0x%08x"), bEncrypt?"BCryptEncrypt":"BCryptDecrypt",  ntStatus);

	}

	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CCNGCrypt::cryptLastByte(bool bEncrypt,PUCHAR pbufFileToOpen, ULONG iBytesRead, ULONG cbIV, PBYTE pbufFileToSave, DWORD& iBufToSave)
{
	NTSTATUS ntStatus= STATUS_UNSUCCESSFUL;
	DWORD		cbCipherText		= 0;

	if (bEncrypt)
	{
		ntStatus = BCryptEncrypt(m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV, NULL, 0, &cbCipherText, BCRYPT_BLOCK_PADDING);

		if ( ntStatus != STATUS_SUCCESS )
		{
			m_sError.Format(_T("Error receiving the size required for the ciphertext. BCryptEncrypt return with error 0x%08x"), ntStatus);					
			return false;
		}
		ntStatus = BCryptEncrypt( m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV, pbufFileToSave, cbCipherText, &cbCipherText, BCRYPT_BLOCK_PADDING);
		switch(ntStatus)
		{
		case STATUS_SUCCESS:			
			iBufToSave		=	cbCipherText;
			return true;
		default:					
			m_sError.Format(_T("Error receiving ciphertext. BCryptEncrypt return with error 0x%08x"), ntStatus);					
			return false;
		}
	}
	else
	{
		ntStatus = BCryptDecrypt( m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV, NULL, 0, &cbCipherText, BCRYPT_BLOCK_PADDING);

		if ( ntStatus != STATUS_SUCCESS )
		{
			m_sError.Format(_T("Error receiving the size required for the ciphertext. BCryptDecrypt return with error 0x%08x"), ntStatus);					
			return false;
		}	
		ntStatus = BCryptDecrypt( m_hKey, pbufFileToOpen, iBytesRead, NULL, m_pbIV, cbIV, pbufFileToSave, cbCipherText, &cbCipherText, BCRYPT_BLOCK_PADDING);

		switch(ntStatus)
		{
		case STATUS_SUCCESS:			
			iBufToSave = cbCipherText;
			return true;
		default:
			m_sError.Format(_T("Error receiving ciphertext. BCryptDecrypt return with error 0x%08x"), ntStatus);					
			return false;
		}
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
bool CCNGCrypt::createSymmetricKey_SHA1_Hash(PCWSTR pwszText, DWORD cbKeyObject)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	BCRYPT_KEY_HANDLE	hKey = NULL;

	DWORD               cbHashObject, cbResult;
	BYTE                rgbHash[20];
	DWORD				cbData	= 0; 

	ntStatus = BCryptOpenAlgorithmProvider(&m_hHashAlg,BCRYPT_SHA1_ALGORITHM,NULL,0);
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error Open Algoritm for the key. BCryptOpenAlgorithmProvider failed with status: 0x%08x\n"), ntStatus);
		return false;
	}

	ntStatus = BCryptGetProperty(m_hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject,  sizeof(DWORD), &cbData, 0);
	if (ntStatus!=STATUS_SUCCESS)
	{
		m_sError.Format(_T("Error calculating the size of KeyObject. BCryptGetProperty return with error 0x%08x"), ntStatus);
		return false;
	}

	m_pbKeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbKeyObject);
	if(NULL == m_pbKeyObject)
	{
		m_sError.Format(_T("Error memory allocation key object on the heap failed"));
		return false;
	}


	//  Determine the size of the Hash object
	ntStatus = BCryptGetProperty( m_hHashAlg,BCRYPT_OBJECT_LENGTH,(PBYTE) &cbHashObject,sizeof(DWORD),&cbResult,0);
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error determining the size of the Hash object. BCryptGetProperty failed with status: 0x%08x"), ntStatus);
		return false;
	}

	m_pbHashObject = (PBYTE)malloc(cbHashObject);

	if(NULL == m_pbHashObject)
	{
		m_sError.Format(_T("Error allocating Hash object memory"));
		return false;
	}

	//  Create the Hash object
	ntStatus = BCryptCreateHash( m_hHashAlg, &m_hHash, m_pbHashObject, cbHashObject, NULL, 0, 0);
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error Creating the Hash object. BCryptCreateHash failed with status: 0x%08x"), ntStatus);
		return false;
	}

	// Hash the data
	ntStatus = BCryptHashData( m_hHash, (PBYTE)pwszText, (ULONG)wcslen(pwszText), 0 );
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error hashing the data. BCryptHashData failed with status: 0x%08x"), ntStatus);
		return false;
	}

	// Finish the hash
	ntStatus = BCryptFinishHash( m_hHash, rgbHash, sizeof(rgbHash), 0);
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error finish the hash. BCryptFinishHash failed with status: 0x%08x"), ntStatus);
		return false;
	}

	ntStatus = BCryptGenerateSymmetricKey( m_hAesAlg, &hKey, m_pbKeyObject, cbKeyObject, rgbHash, SYMM_KEY_SIZE_SECRET, 0);
	if(STATUS_SUCCESS != ntStatus)
	{
		m_sError.Format(_T("Error creating the key. BCryptGenerateSymmetricKey failed with status: 0x%08x"), ntStatus);
		hKey = NULL;
	}
	m_hKey =hKey ;
	return true;
}


bool CCNGCrypt::createSymmetricKey_AES_CBC(DWORD &cbKeyObject, DWORD &cbIV )
{
	NTSTATUS	ntStatus = STATUS_UNSUCCESSFUL;
	DWORD		cbData	= 0; 

	cbKeyObject	= 0;
	cbIV  = 0;

	ntStatus = BCryptGetProperty(m_hAesAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbKeyObject,  sizeof(DWORD), &cbData, 0);
	if (ntStatus!=STATUS_SUCCESS)
	{
		m_sError.Format(_T("Error calculating the size of KeyObject. BCryptGetProperty return with error 0x%08x"), ntStatus);
		return false;
	}
	
	m_pbKeyObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbKeyObject);
	if(NULL == m_pbKeyObject)
	{
		m_sError.Format(_T("Error memory allocation key object on the heap failed"));
		return false;
	}

	ntStatus = BCryptGetProperty( m_hAesAlg, BCRYPT_BLOCK_LENGTH, (PBYTE)&cbIV, sizeof(DWORD), &cbData, 0);
	if (ntStatus!=STATUS_SUCCESS)
	{
		m_sError.Format(_T("Error calculating the block length for the IV. BCryptGetProperty return with error 0x%08x"), ntStatus);
		return false;
	}

	if (cbIV > sizeof (rgbIV))
	{
		m_sError.Format(_T("Block length is longer than the provided IV length\n"));
		return false;
	}

	m_pbIV= (PBYTE) HeapAlloc (GetProcessHeap (), 0, cbIV);
	if(NULL == m_pbIV)
	{
		m_sError.Format(_T("Error memory allocation buffer for the IV on the heap failed"));
		return false;
	}

	memcpy(m_pbIV, rgbIV, cbIV);

	ntStatus = BCryptSetProperty(m_hAesAlg, BCRYPT_CHAINING_MODE, (PBYTE)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0);
	if (ntStatus!=STATUS_SUCCESS)
	{
		m_sError.Format(_T("Error setting the ChainingMode CBC. BCryptSetProperty return with error 0x%08x"), ntStatus);
		return false;
	}

	// generate the key from supplied input key bytes
	ntStatus = BCryptGenerateSymmetricKey(m_hAesAlg, &m_hKey, m_pbKeyObject, cbKeyObject, (PBYTE)rgbAES128Key, sizeof(rgbAES128Key), 0);

/*	PBYTE pKey;
	memcpy(pKey, sKey, sKey.GetLength());
	ntStatus = BCryptGenerateSymmetricKey(m_hAesAlg, &m_hKey, m_pbKeyObject, cbKeyObject, (PBYTE) pKey, sKey.GetLength(), 0);
*/	
	if (ntStatus!=STATUS_SUCCESS)
	{
		m_sError.Format(_T("Error generate the key. BCryptGenerateSymmetricKey return with error 0x%08x"), ntStatus);
		return false;
	}

	return true;

}