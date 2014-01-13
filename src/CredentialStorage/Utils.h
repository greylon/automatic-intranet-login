#pragma once

typedef std::vector<BYTE> ByteArray;

namespace utils
{
   ByteArray FindMACAddress();
   CStringW GetMACAddressString();

   ByteArray String2ByteArray( const CStringW& i_str );
   CStringW ByteArray2String( const ByteArray& i_blob );
   
   CStringA GetSystemMessageA( DWORD iCode );
   CStringW GetSystemMessageW( DWORD iCode );

   class CStorageException
   {
   public:
      CStorageException(const CStringW& i_message, DWORD i_errorCode = GetLastError());

      const CStringW& GetMessage() const { return m_message; }
      const CStringW& GetSystemError() const { return m_systemError; }
      const DWORD GetError() const { return m_errorCode; }

   private:
      CStringW m_message;
      CStringW m_systemError;
      DWORD m_errorCode;
   };

   class CAutoImpersonate
   {
   public:
      CAutoImpersonate();
      ~CAutoImpersonate();
      static std::wstring GetCurrentUserName();
   private:
      HANDLE hPToken;
   };

   //////////////////////////////////////////////////////////////////////////
   std::string GetPublicDocumentsPath(const CStringW& i_appName);
   std::wstring GetKnownPath(REFKNOWNFOLDERID rfid);
   void CreateLink(LPCWSTR lpszPathLink, LPCWSTR lpszPathObj, LPCWSTR lpszWorkingDir, LPCWSTR lpszDesc); 

}
