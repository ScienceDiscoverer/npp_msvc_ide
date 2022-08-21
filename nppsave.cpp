#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#pragma comment( lib, "user32")

int wmain()
{
	// Find Notepad++ window by Class Name
	HWND nppw = FindWindow(L"Notepad++", NULL);
	
	//cout << "BROADCASTING NPP MESSAGE!!!" << endl;
	// PostMessage is smart! It knows to not broadcast WM_USER MSG!!!
	PostMessage(nppw, WM_USER + 1000 + 38, 0, 0); // 2062 -> 0x80E
	
	return 0;
}