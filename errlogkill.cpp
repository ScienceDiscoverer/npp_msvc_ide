// CONSOLE
#include <utf>

#pragma comment( lib, "user32" )

#define NPPMSG  (WM_USER + 1000)
#define NPPM_SAVECURRENTFILE (NPPMSG + 38)

const wchar_t *msg_wnd_class = L"ErrLogsMsgWindow";

i32 wmain(i64 argc, wchar_t **argv)
{
	if(argc != 2)
	{
		return 1;
	}
	
	// Find Notepad++ window by Class Name
	HWND nppw = FindWindow(L"Notepad++", NULL);
	SendMessage(nppw, NPPM_SAVECURRENTFILE, 0, 0);

	txt fname = wt2u8(cwstr{ argv[1], strl(argv[1]) });
	
	COPYDATASTRUCT data;
	data.dwData = 0;
	data.cbData = (DWORD)~fname + 1;
	data.lpData = PVOID((const char *)fname);
	
	HWND target = FindWindowEx(HWND_MESSAGE, NULL, msg_wnd_class, NULL);
	
	while(target != NULL)
	{
		SendMessage(target, WM_COPYDATA, 0, (LPARAM)&data);
		target = FindWindowEx(HWND_MESSAGE, target, msg_wnd_class, NULL);
	}
	
	return 0;
}