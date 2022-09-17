namespace ghassanpl::win
{
	/// Base Windows Stuff
	extern "C"
	{
		typedef unsigned short WORD;
		typedef unsigned long DWORD;
		typedef unsigned int UINT;
		typedef void* HWND;
		typedef void* HINSTANCE;
		typedef HINSTANCE HMODULE;
		typedef void* HICON;
		typedef const wchar_t* PCWSTR, * LPCWSTR;
		typedef const char* PCCH, * LPCCH;
		typedef wchar_t* LPWSTR;
		typedef __int64 LONG_PTR;
		typedef unsigned __int64 ULONG_PTR;
		typedef unsigned __int64 UINT_PTR;
		typedef long HRESULT;
		typedef LONG_PTR LPARAM;
		typedef UINT_PTR WPARAM;

		static constexpr auto S_OK = ((HRESULT)0L);
		static constexpr auto S_FALSE = ((HRESULT)1L);

		__declspec(dllimport) HWND __stdcall GetActiveWindow(void);
		__declspec(dllimport) HMODULE __stdcall GetModuleHandleW(LPCWSTR lpModuleName);
	}
}