// WINAPI
BOOL ShowWindow(
  [in] HWND hWnd,
  [in] int  nCmdShow
);
LONG_PTR GetWindowLongPtrA(
  [in] HWND hWnd,
  [in] int  nIndex
);
BOOL SetFileTime(
  [in]           HANDLE         hFile,
  [in, optional] const FILETIME *lpCreationTime,
  [in, optional] const FILETIME *lpLastAccessTime,
  [in, optional] const FILETIME *lpLastWriteTime
);
BOOLEAN CreateSymbolicLinkA(
  [in] LPCSTR lpSymlinkFileName,
  [in] LPCSTR lpTargetFileName,
  [in] DWORD  dwFlags
);
BOOLEAN CreateSymbolicLinkW(
  [in] LPCWSTR lpSymlinkFileName,
  [in] LPCWSTR lpTargetFileName,
  [in] DWORD   dwFlags
);
BOOL CreateHardLinkA(
  [in] LPCSTR                lpFileName,
  [in] LPCSTR                lpExistingFileName,
  [in] LPSECURITY_ATTRIBUTES lpSecurityAttributes
);
BOOL CreateHardLinkW(
  [in] LPCWSTR               lpFileName,
  [in] LPCWSTR               lpExistingFileName,
  [in] LPSECURITY_ATTRIBUTES lpSecurityAttributes
);
BOOL GetFileInformationByHandle(
  [in]  HANDLE                       hFile,
  [out] LPBY_HANDLE_FILE_INFORMATION lpFileInformation
);
DWORD GetLogicalDrives();
BOOL Rectangle(
  [in] HDC hdc,
  [in] int left,
  [in] int top,
  [in] int right,
  [in] int bottom
);
BOOL MoveToEx(
  [in]  HDC     hdc,
  [in]  int     x,
  [in]  int     y,
  [out] LPPOINT lppt
);
BOOL LineTo(
  [in] HDC hdc,
  [in] int x,
  [in] int y
);
LPVOID HeapAlloc(
  [in] HANDLE hHeap,
  [in] DWORD  dwFlags,
  [in] SIZE_T dwBytes
);
LPVOID HeapReAlloc(
  [in] HANDLE  hHeap,
  [in] DWORD   dwFlags,
  [in] LPVOID  lpMem,
  [in] SIZE_T  dwBytes
);
BOOL HeapFree(
  [in] HANDLE hHeap,
  [in] DWORD  dwFlags,
  [in] LPVOID lpMem
);
HANDLE HeapCreate(
  [in] DWORD  flOptions,
  [in] SIZE_T dwInitialSize,
  [in] SIZE_T dwMaximumSize
);
SIZE_T HeapCompact(
  [in] HANDLE hHeap,
  [in] DWORD  dwFlags
);
HANDLE GetProcessHeap();
DWORD GetProcessHeaps(
  [in]  DWORD   NumberOfHeaps,
  [out] PHANDLE ProcessHeaps
);
BOOL HeapWalk(
  [in]      HANDLE               hHeap,
  [in, out] LPPROCESS_HEAP_ENTRY lpEntry
);
DWORD WaitForSingleObject(
  [in] HANDLE hHandle,
  [in] DWORD  dwMilliseconds
);
HANDLE OpenProcess(
  [in] DWORD dwDesiredAccess,
  [in] BOOL  bInheritHandle,
  [in] DWORD dwProcessId
);
DWORD GetWindowThreadProcessId(
  [in]            HWND    hWnd,
  [out, optional] LPDWORD lpdwProcessId
);
void GetLocalTime(
  [out] LPSYSTEMTIME lpSystemTime
);
DWORD GetFileAttributesA(
  [in] LPCSTR lpFileName
);
DWORD GetFileAttributesW(
  [in] LPCWSTR lpFileName
);
BOOL GetFileTime(
  [in]            HANDLE     hFile,
  [out, optional] LPFILETIME lpCreationTime,
  [out, optional] LPFILETIME lpLastAccessTime,
  [out, optional] LPFILETIME lpLastWriteTime
);
BOOL WriteFile(
  [in]                HANDLE       hFile,
  [in]                LPCVOID      lpBuffer,
  [in]                DWORD        nNumberOfBytesToWrite,
  [out, optional]     LPDWORD      lpNumberOfBytesWritten,
  [in, out, optional] LPOVERLAPPED lpOverlapped
);
BOOL ReadFile(
  [in]                HANDLE       hFile,
  [out]               LPVOID       lpBuffer,
  [in]                DWORD        nNumberOfBytesToRead,
  [out, optional]     LPDWORD      lpNumberOfBytesRead,
  [in, out, optional] LPOVERLAPPED lpOverlapped
);
HANDLE FindFirstFileW(
  [in]  LPCWSTR            lpFileName,
  [out] LPWIN32_FIND_DATAW lpFindFileData
);
int MessageBoxA(
  [in, optional] HWND   hWnd,
  [in, optional] LPCSTR lpText,
  [in, optional] LPCSTR lpCaption,
  [in]           UINT   uType
);
int MessageBoxW(
  [in, optional] HWND    hWnd,
  [in, optional] LPCWSTR lpText,
  [in, optional] LPCWSTR lpCaption,
  [in]           UINT    uType
);
void RaiseException(
  [in] DWORD           dwExceptionCode,
  [in] DWORD           dwExceptionFlags,
  [in] DWORD           nNumberOfArguments,
  [in] const ULONG_PTR *lpArguments
);
DWORD GetModuleFileNameA(
  [in, optional] HMODULE hModule,
  [out]          LPSTR   lpFilename,
  [in]           DWORD   nSize
);
DWORD GetModuleFileNameW(
  [in, optional] HMODULE hModule,
  [out]          LPWSTR  lpFilename,
  [in]           DWORD   nSize
);
BOOL GetFileSizeEx(
  [in]  HANDLE         hFile,
  [out] PLARGE_INTEGER lpFileSize
);
BOOL InvalidateRect(
  [in] HWND       hWnd,
  [in] const RECT *lpRect,
  [in] BOOL       bErase
);
HANDLE CreateThread(
  [in, optional]  LPSEC_ATTR   lpThAttrs,
  [in]            SIZE_T       dwStackSz,
  [in]            LPTH_ST_FUNC lpStAddr,
  [in, optional]  LPVOID       lpParam,
  [in]            DWORD        dwCrFlags,
  [out, optional] LPDWORD      lpThreadId
);
BOOL CloseHandle(
  [in] HANDLE hObject
);
BOOL SetWindowPos(
  [in]           HWND hWnd,
  [in, optional] HWND hWndInsertAfter,
  [in]           int  X,
  [in]           int  Y,
  [in]           int  cx,
  [in]           int  cy,
  [in]           UINT uFlags
);
LRESULT SendMessage(
  [in] HWND   hWnd,
  [in] UINT   Msg,
  [in] WPARAM wParam,
  [in] LPARAM lParam
);
BOOL PostMessage(
  [in, optional] HWND   hWnd,
  [in]           UINT   Msg,
  [in]           WPARAM wParam,
  [in]           LPARAM lParam
);
HANDLE CreateEvent(
  [in, optional] LPSEC_ATTRS           lpEventAttributes,
  [in]           BOOL                  bManualReset,
  [in]           BOOL                  bInitialState,
  [in, optional] LPCSTR                lpName
);
HANDLE CreateMutex(
  [in, optional] LPSEC_ATTRS           lpMutexAttributes,
  [in]           BOOL                  bInitialOwner,
  [in, optional] LPCSTR                lpName
);
HANDLE CreateSemaphore(
  [in, optional] LPSEC_ATTRS           lpSemaphoreAttributes,
  [in]           LONG                  lInitialCount,
  [in]           LONG                  lMaximumCount,
  [in, optional] LPCSTR                lpName
);
BOOL SetEvent(
  [in] HANDLE hEvent
);
BOOL ResetEvent(
  [in] HANDLE hEvent
);
BOOL SetFilePointerEx(
  [in]            HANDLE         hFile,
  [in]            LARGE_INTEGER  liDistanceToMove,
  [out, optional] PLARGE_INTEGER lpNewFilePointer,
  [in]            DWORD          dwMoveMethod
);
BOOL SetEndOfFile(
  [in] HANDLE hFile
);
BOOL CreateDirectoryA(
  [in]           LPCSTR                lpPathName,
  [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
);
BOOL CreateDirectoryW(
  [in]           LPCWSTR               lpPathName,
  [in, optional] LPSECURITY_ATTRIBUTES lpSecurityAttributes
);
BOOL FileTimeToSystemTime(
  [in]  const FILETIME *lpFileTime,
  [out] LPSYSTEMTIME   lpSystemTime
);
BOOL ScreenToClient(
  [in]      HWND    hWnd,
  [in, out] LPPOINT lpPoint
);
BOOL ClientToScreen(
  [in]      HWND    hWnd,
  [in, out] LPPOINT lpPoint
);
BOOL MoveFile(
  [in] LPCTSTR lpExistingFileName,
  [in] LPCTSTR lpNewFileName
);
BOOL MoveFileExA(
  [in]           LPCSTR lpExistingFileName,
  [in, optional] LPCSTR lpNewFileName,
  [in]           DWORD  dwFlags
);
BOOL CopyFile(
  [in] LPCTSTR lpExistingFileName,
  [in] LPCTSTR lpNewFileName,
  [in] BOOL    bFailIfExists
);
BOOL CopyFileExA(
  [in]           LPCSTR             lpExistingFileName,
  [in]           LPCSTR             lpNewFileName,
  [in, optional] LPPROGRESS_ROUTINE lpProgressRoutine,
  [in, optional] LPVOID             lpData,
  [in, optional] LPBOOL             pbCancel,
  [in]           DWORD              dwCopyFlags
);
BOOL DeleteFileA(
  [in] LPCSTR lpFileName
);
BOOL DeleteFileW(
  [in] LPCWSTR lpFileName
);
BOOL QueryPerformanceCounter(
  [out] LARGE_INTEGER *lpPerformanceCount
);
void Sleep(
  [in] DWORD dwMilliseconds
);
LPWSTR GetCommandLineW();
LPWSTR * CommandLineToArgvW(
  [in]  LPCWSTR lpCmdLine,
  [out] int     *pNumArgs
);
BOOL UpdateWindow(
  [in] HWND hWnd
);
DWORD GetTickCount();
DWORD GetLastError();
void DisableProcessWindowsGhosting();
BOOL TerminateProcess(
  [in] HANDLE hProcess,
  [in] UINT   uExitCode
);
BOOL TerminateThread(
  [in, out] HANDLE hThread,
  [in]      DWORD  dwExitCode
);
void ExitProcess(
  [in] UINT uExitCode
);
BOOL TextOutW(
  [in] HDC     hdc,
  [in] int     x,
  [in] int     y,
  [in] LPCWSTR lpString,
  [in] int     c
);
BOOL TextOutA(
  [in] HDC    hdc,
  [in] int    x,
  [in] int    y,
  [in] LPCSTR lpString,
  [in] int    c
);
COLORREF SetBkColor(
  [in] HDC      hdc,
  [in] COLORREF color
);
COLORREF SetBkColor(
  [in] HDC      hdc,
  [in] COLORREF color
);
int SetBkMode(
  [in] HDC hdc,
  [in] int mode
);
COLORREF SetTextColor(
  [in] HDC      hdc,
  [in] COLORREF color
);
HGDIOBJ SelectObject(
  [in] HDC     hdc,
  [in] HGDIOBJ h
);
HANDLE LoadImageA(
  [in, optional] HINSTANCE hInst,
  [in]           LPCSTR    name,
  [in]           UINT      type,
  [in]           int       cx,
  [in]           int       cy,
  [in]           UINT      fuLoad
);
int ShowCursor(
  [in] BOOL bShow
);
HCURSOR SetCursor(
  [in, optional] HCURSOR hCursor
);
HGDIOBJ GetStockObject(
  [in] int i
);
HANDLE WINAPI GetStdHandle(
  _In_ DWORD nStdHandle
);
STD_INPUT_HANDLE
STD_OUTPUT_HANDLE
STD_ERROR_HANDLE
PROCESS_ALL_ACCESS
PROCESS_CREATE_PROCESS
PROCESS_CREATE_THREAD
PROCESS_DUP_HANDLE
PROCESS_QUERY_INFORMATION
PROCESS_QUERY_LIMITED_INFORMATION
PROCESS_SET_INFORMATION
PROCESS_SET_QUOTA
PROCESS_SUSPEND_RESUME
PROCESS_TERMINATE
PROCESS_VM_OPERATION
PROCESS_VM_READ
PROCESS_VM_WRITE
SYNCHRONIZE
SYSTEMTIME
DELETE
READ_CONTROL
WRITE_DAC
WRITE_OWNER
INFINITE
LARGE_INTEGER
FILE_ATTRIBUTE_READONLY
FILE_ATTRIBUTE_HIDDEN
FILE_ATTRIBUTE_SYSTEM
FILE_ATTRIBUTE_DIRECTORY
FILE_ATTRIBUTE_ARCHIVE
FILE_ATTRIBUTE_DEVICE
FILE_ATTRIBUTE_NORMAL
FILE_ATTRIBUTE_TEMPORARY
FILE_ATTRIBUTE_SPARSE_FILE
FILE_ATTRIBUTE_REPARSE_POINT
FILE_ATTRIBUTE_COMPRESSED
FILE_ATTRIBUTE_OFFLINE
FILE_ATTRIBUTE_NOT_CONTENT_INDEXED
FILE_ATTRIBUTE_ENCRYPTED
FILE_ATTRIBUTE_INTEGRITY_STREAM
FILE_ATTRIBUTE_VIRTUAL
FILE_ATTRIBUTE_NO_SCRUB_DATA
FILE_ATTRIBUTE_EA
FILE_ATTRIBUTE_PINNED
FILE_ATTRIBUTE_UNPINNED
FILE_ATTRIBUTE_RECALL_ON_OPEN
FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS
FILE_BEGIN
FILE_CURRENT
FILE_END
FILE_SHARE_DELETE
FILE_SHARE_READ
FILE_SHARE_WRITE
FILETIME
CREATE_ALWAYS
CREATE_NEW
OPEN_ALWAYS
OPEN_EXISTING
TRUNCATE_EXISTING
WIN32_FIND_DATA
WIN32_LEAN_AND_MEAN
HEAP_CREATE_ENABLE_EXECUTE
HEAP_GENERATE_EXCEPTIONS
HEAP_NO_SERIALIZE
VK_LBUTTON
VK_RBUTTON
VK_MBUTTON
VK_XBUTTON1
VK_XBUTTON2
VK_BACK
VK_TAB
VK_RETURN
VK_SHIFT
VK_CONTROL
VK_MENU
VK_CAPITAL
VK_SPACE
VK_LEFT
VK_UP
VK_RIGHT
VK_DOWN
VK_SNAPSHOT
VK_DELETE
VK_LSHIFT
VK_RSHIFT
VK_LCONTROL
VK_RCONTROL
VK_LMENU
VK_RMENU
VK_ESCAPE
VK_END
VK_HOME
MK_CONTROL
MK_LBUTTON
MK_MBUTTON
MK_RBUTTON
MK_XBUTTON1
MK_XBUTTON2
HWND_BOTTOM
HWND_NOTOPMOST
HWND_TOP
HWND_TOPMOST
SWP_ASYNCWINDOWPOS
SWP_DEFERERASE
SWP_DRAWFRAME
SWP_FRAMECHANGED
SWP_HIDEWINDOW
SWP_NOACTIVATE
SWP_NOCOPYBITS
SWP_NOMOVE
SWP_NOOWNERZORDER
SWP_NOREDRAW
SWP_NOREPOSITION
SWP_NOSENDCHANGING
SWP_NOSIZE
SWP_NOZORDER
SWP_SHOWWINDOW
FILE_BEGIN
FILE_CURRENT
FILE_END
GWL_EXSTYLE
GWLP_HINSTANCE
GWLP_HWNDPARENT
GWLP_ID
GWL_STYLE
GWLP_USERDATA
GWLP_WNDPROC
DWLP_DLGPROC
DWLP_MSGRESULT
DWLP_USER
WS_BORDER
WS_CAPTION
WS_CHILD
WS_CHILDWINDOW
WS_CLIPCHILDREN
WS_CLIPSIBLINGS
WS_DISABLED
WS_DLGFRAME
WS_GROUP
WS_HSCROLL
WS_ICONIC
WS_MAXIMIZE
WS_MAXIMIZEBOX
WS_MINIMIZE
WS_MINIMIZEBOX
WS_OVERLAPPED
WS_OVERLAPPEDWINDOW
WS_POPUP
WS_POPUPWINDOW
WS_SIZEBOX
WS_SYSMENU
WS_TABSTOP
WS_THICKFRAME
WS_TILED
WS_TILEDWINDOW
WS_VISIBLE
WS_VSCROLL
WS_EX_ACCEPTFILES
WS_EX_APPWINDOW
WS_EX_CLIENTEDGE
WS_EX_COMPOSITED
WS_EX_CONTEXTHELP
WS_EX_CONTROLPARENT
WS_EX_DLGMODALFRAME
WS_EX_LAYERED
WS_EX_LAYOUTRTL
WS_EX_LEFT
WS_EX_LEFTSCROLLBAR
WS_EX_LTRREADING
WS_EX_MDICHILD
WS_EX_NOACTIVATE
WS_EX_NOINHERITLAYOUT
WS_EX_NOPARENTNOTIFY
WS_EX_NOREDIRECTIONBITMAP
WS_EX_OVERLAPPEDWINDOW
WS_EX_PALETTEWINDOW
WS_EX_RIGHT
WS_EX_RIGHTSCROLLBAR
WS_EX_RTLREADING
WS_EX_STATICEDGE
WS_EX_TOOLWINDOW
WS_EX_TOPMOST
WS_EX_TRANSPARENT
WS_EX_WINDOWEDGE
SW_HIDE
SW_SHOWNORMAL
SW_NORMAL
SW_SHOWMINIMIZED
SW_SHOWMAXIMIZED
SW_MAXIMIZE
SW_SHOWNOACTIVATE
SW_SHOW
SW_MINIMIZE
SW_SHOWMINNOACTIVE
SW_SHOWNA
SW_RESTORE
SW_SHOWDEFAULT
SW_FORCEMINIMIZE
// std c
void * memset(void *ptr, int value, size_t num);
void * memcpy(void *destination, const void *source, size_t num);
int memcmp(const void *ptr1, const void *ptr2, size_t num);
// mmanip
bool64 memeq(const void *a, const void *b, ui64 s);
bool64 memneq(const void *a, const void *b, ui64 s);
i64 mcmp(const void *a, const void *b, ui64 s);
void mcpy(void *d, const void *s, ui64 n);
ui64 strl(const char *cs);
ui64 strl(const wchar_t *cs);
void mand(void *m, ui64 v, ui64 s);
void mor(void *m, ui64 v, ui64 s);
// utf
wtxt t2u16(const txt &t8);
txt wt2u8(const wtxt &t16);
wtxt & t2u16(wtxt &t16, const txt &t8);
txt & wt2u8(txt &t8, const wtxt &t16);
ui64 u8ts(const txt &t8);
ui64 u16ts(const wtxt &t16);
void txtMovToWtxt(txt &t, wtxt &wt);
void wtxtMovToTxt(wtxt &wt, txt &t);
// utfcase
wchar_t utfCharToUp(wchar_t c);
wchar_t utfCharToLow(wchar_t c);
ui64 utfTxtFindCaseIns(const wtxt &t, ui64 pos, const wtxt &fnd);
wtxt & utfTxtToLow(wtxt &t);
wtxt & utfTxtToUp(wtxt &t);
// txt + wtxt
txt & txtd(txt &t, ui64 pos, ui64 n);
wtxt & txtd(wtxt &t, ui64 pos, ui64 n);
txt & txti(txt &t, ui64 pos, char ins);
txt & txti(txt &t, ui64 pos, const char *ins);
txt & txti(txt &t, ui64 pos, const txt &ins);
wtxt & txti(wtxt &t, ui64 pos, const wchar_t *ins);
wtxt & txti(wtxt &t, ui64 pos, const wtxt &ins);
wtxt & txti(wtxt &t, ui64 pos, wchar_t ins);
txt & txto(txt &t, ui64 pos, char ovr);
txt & txto(txt &t, ui64 pos, const char *ovr);
txt & txto(txt &t, ui64 pos, const txt &ovr);
wtxt & txto(wtxt &t, ui64 pos, const wchar_t *ovr);
wtxt & txto(wtxt &t, ui64 pos, const wtxt &ovr);
wtxt & txto(wtxt &t, ui64 pos, wchar_t ovr);
txt & txtr(txt &t, ui64 pos, ui64 n, char rep);
txt & txtr(txt &t, ui64 pos, ui64 n, const char *rep);
txt & txtr(txt &t, ui64 pos, ui64 n, const txt &rep);
wtxt & txtr(wtxt &t, ui64 pos, ui64 n, const wchar_t *rep);
wtxt & txtr(wtxt &t, ui64 pos, ui64 n, const wtxt &rep);
wtxt & txtr(wtxt &t, ui64 pos, ui64 n, wchar_t rep);
txt i2h(ui16 i);
txt i2h(ui32 i);
txt i2h(ui64 i);
txt i2h(ui8 i);
txt i2t(i32 i);
txt i2t(i64 i);
txt i2t(ui32 i);
txt i2t(ui64 i);
txt b2t(ui8 b);
txt b2t(i8 b);
txt b2h(ui8 b);
txt i2b(ui64 i);
txt i2b(ui32 i);
txt i2b(ui16 i);
txt b2b(ui8 b);
txt txts(const txt &t, ui64 pos, ui64 n);
wtxt txts(const wtxt &t, ui64 pos, ui64 n);
txt txtsp(const txt &t, ui64 p0, ui64 p1);
wtxt txtsp(const wtxt &t, ui64 p0, ui64 p1);
ui64 h2i(const txt &t);
ui64 h2i(const wtxt &t);
ui64 t2i(const txt &t);
ui64 t2i(const wtxt &t);
ui64 txtf(const txt &t, ui64 pos, char fnd);
ui64 txtf(const txt &t, ui64 pos, const char *fnd);
ui64 txtf(const txt &t, ui64 pos, const txt &fnd);
ui64 txtf(const wtxt &t, ui64 pos, const wchar_t *fnd);
ui64 txtf(const wtxt &t, ui64 pos, const wtxt &fnd);
ui64 txtf(const wtxt &t, ui64 pos, wchar_t fnd);
ui64 txtfe(const txt &t, ui64 pos, char fnd);
ui64 txtfe(const txt &t, ui64 pos, const char *fnd);
ui64 txtfe(const txt &t, ui64 pos, const txt &fnd);
ui64 txtfe(const wtxt &t, ui64 pos, const wchar_t *fnd);
ui64 txtfe(const wtxt &t, ui64 pos, const wtxt &fnd);
ui64 txtfe(const wtxt &t, ui64 pos, wchar_t fnd);
ui64 txtfr(const txt &t, ui64 p0, ui64 p1, char fnd);
ui64 txtfr(const txt &t, ui64 p0, ui64 p1, const char *fnd);
ui64 txtfr(const txt &t, ui64 p0, ui64 p1, const txt &fnd);
ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, const wchar_t *fnd);
ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, const wtxt &fnd);
ui64 txtfr(const wtxt &t, ui64 p0, ui64 p1, wchar_t fnd);
wtxt i2wh(ui16 i);
wtxt i2wh(ui32 i);
wtxt i2wh(ui64 i);
wtxt i2wh(ui8 i);
wtxt i2wt(i32 i);
wtxt i2wt(i64 i);
wtxt i2wt(ui32 i);
wtxt i2wt(ui64 i);
wtxt b2wt(ui8 b);
wtxt b2wt(i8 b);
wtxt b2wh(ui8 b);
wtxt i2wb(ui64 i);
wtxt i2wb(ui32 i);
wtxt i2wb(ui16 i);
wtxt b2wb(ui8 b);
txt & txtssz(txt &t, ui64 sz);
txt & txtsdt(txt &t, ui64 ts, ui64 s, char *td);
txt & txtclr(txt &t);
txt & txtz(txt &t);
txt & txtinv(txt &t);
txt & txtfree(txt &t);
txt & txtadd(txt &t, const char *d, ui64 sz);
wtxt & txtadd(wtxt &t, const wchar_t *d, ui64 sz);
wtxt & txtssz(wtxt &t, ui64 sz);
wtxt & txtsdt(wtxt &t, ui64 ts, ui64 s, wchar_t *td);
wtxt & txtclr(wtxt &t);
wtxt & txtz(wtxt &t);
wtxt & txtinv(wtxt &t);
wtxt & txtfree(wtxt &t);
txt & t2low(txt &t);
txt & t2lowfst(txt &t);
txt & t2up(txt &t);
wtxt & t2low(wtxt &t);
wtxt & t2lowfst(wtxt &t);
wtxt & t2up(wtxt &t);
char c2low(char c);
char c2up(char c);
wchar_t c2low(wchar_t c);
wchar_t c2up(wchar_t c);
ui64 txtfci(const txt &t, ui64 pos, const txt &fnd);
ui64 txtfci(const txt &t, ui64 pos, const char *fnd);
ui64 txtfci(const txt &t, ui64 pos, char fnd);
ui64 txtfci(const wtxt &t, ui64 pos, const wtxt &fnd);
ui64 txtfci(const wtxt &t, ui64 pos, const wchar_t *fnd);
ui64 txtfci(const wtxt &t, ui64 pos, wchar_t fnd);
ui64 txtfa(const txt &t, ui64 pos, const txt &chars);
ui64 txtfa(const txt &t, ui64 pos, const char *chars, ui64 charss);
ui64 txtfa(const wtxt &t, ui64 pos, const wtxt &chars);
ui64 txtfa(const wtxt &t, ui64 pos, const wchar_t *chars, ui64 charss);
bool64 txtseq(const txt &t, ui64 pos, const txt &sub);
bool64 txtsneq(const txt &t, ui64 pos, const txt &sub);
bool64 txtseq(const txt &t, ui64 pos, cstr sub);
bool64 txtsneq(const txt &t, ui64 pos, cstr sub);
bool64 txtseq(const wtxt &t, ui64 pos, const wtxt &sub);
bool64 txtsneq(const wtxt &t, ui64 pos, const wtxt &sub);
bool64 txtseq(const wtxt &t, ui64 pos, cwstr sub);
bool64 txtsneq(const wtxt &t, ui64 pos, cwstr sub);
txt & txtszu(txt &t);
txt & txttrm(txt &t);
wtxt & txtszu(wtxt &t);
wtxt & txttrm(wtxt &t);
txt & txtrp(txt &t, ui64 p0, ui64 p1, const txt &rep);
txt & txtrp(txt &t, ui64 p0, ui64 p1, cstr rep);
txt & txtrp(txt &t, ui64 p0, ui64 p1, char rep);
wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, const wtxt &rep);
wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, cwstr rep);
wtxt & txtrp(wtxt &t, ui64 p0, ui64 p1, char rep);
txt & txts(txt &t, const txt &src, ui64 pos, ui64 n);
txt & txtsp(txt &t, const txt &src, ui64 p0, ui64 p1);
wtxt & txts(wtxt &t, const wtxt &src, ui64 pos, ui64 n);
wtxt & txtsp(wtxt &t, const wtxt &src, ui64 p0, ui64 p1);
txt & txtdp(txt &t, ui64 p0, ui64 p1);
wtxt & txtdp(wtxt &t, ui64 p0, ui64 p1);
bool64 txtnui(const txt &t);
bool64 txtnsi(const txt &t);
bool64 txtnhi(const txt &t);
bool64 txtnui(const wtxt &t);
bool64 txtnsi(const wtxt &t);
bool64 txtnhi(const wtxt &t);
txt & txtoa(txt &t, char old, char ovr);
wtxt & txtoa(wtxt &t, wchar_t old, wchar_t ovr);
txt & txtdl(txt &t);
wtxt & txtdl(wtxt &t);
// clip
bool clip(const txt &t);
bool clip(const txt &t, HWND wnd);
bool clip(const wtxt &t);
bool clip(const wtxt &t, HWND wnd);
bool64 paste(txt &t);
bool64 paste(txt &t, HWND wnd);
bool64 paste(wtxt &t);
bool64 paste(wtxt &t, HWND wnd);
// sddb
void prect(RECT r);
void prect(SMALL_RECT r);
txt txt2hex(const txt &t);
txt txt2hex(const wtxt &t);
txt str2hex(const char *s);
txt str2hex(const wchar_t *s);
txt truncStr(const char *s, const char *se, ui64 up_to);
wtxt truncStr(const wchar_t *s, const wchar_t *se, ui64 up_to);
void dbgmb(const txt &m);
ui64 dbgmbChoice(const txt &m);
void dbgFileLog(const txt &msg);
void dbgFileLog(const wtxt &msg);
txt err2txt(DWORD err);
// txta + wtxta + ui64a
txta & AddUnique(const txt &data);
wtxta & AddUnique(const wtxt &data);
ui64a & AddUnique(const ui64 &data);
txta & Clear();
txt & Last();
ui64 Find(ui64 val);
ui64a & DelVal(ui64 val);
ui64a & DelIdx(ui64 i);
ui64a & DelLast();
wtxta & MoveInto(wtxta &o);
ui64a & Reserve(ui64 n);
ui64a & Fill(ui64 val);
// math
ui64 abs(ui64 i);
ui32 abs(ui32 i);
ui16 abs(ui16 i);
ui8 abs(ui8 i);
i64 abs(i64 i);
i32 abs(i32 i);
i16 abs(i16 i);
i8 abs(i8 i);
ui64 mpow(ui64 b, ui64 e, ui64 m);
ui64 sqrt(ui64 x);
bool isPrime(ui64 x);
// wndint
ATOM initGui(WNDPROC wnd_proc);
ATOM initGui(WNDPROC wnd_proc, ui64 opts);
HWND spawnMainWnd(ui64 style, const wchar_t* title, i32 w, i32 h, i32 x, i32 y);
HWND spawnMainWnd(ui64 style, const wchar_t* title, i32 w, i32 h, ui64 pos);
HWND spawnWnd(ui64 style, const wchar_t* title, i32 w, i32 h, i32 x, i32 y);
void enterMsgLoop();
void enterTranslateMsgLoop();
LRESULT toggleTopmost();
LRESULT toggleTopmost(HWND wnd);
// conint
void printProgBar(ui64 max, ui64 cur, ui64 x, ui64 y, ui64 len);
void printProgBarMod(ui64 mod, ui64 max, ui64 cur, ui64 x, ui64 y, ui64 len);
// time
txt ticks2txt(ui64 timing);
wtxt ticks2wtxt(ui64 timing);
void swSet(ui64 i);
void swStop(ui64 i);
txt swTime(ui64 i);
wtxt swTimeW(ui64 i);
// fnmanip.h
txt fishOutFileExt(const txt &fn);
txt & fishOutFileExt(const txt &fn, txt &ext);
wtxt fishOutFileExt(const wtxt &fn);
wtxt & fishOutFileExt(const wtxt &fn, wtxt &ext);
txt & removeLastDir(txt &dir);
wtxt & removeLastDir(wtxt &dir);
txt & nameFromPath(const txt &path, txt &n);
wtxt & nameFromPath(const wtxt &path, wtxt &n);
// txta + wtxta + ui64a
wtxta & Print();
// ftools
ui64 getFileList(const cwstr *dirs, wtxta &list);
ui64 getFileList(const cwstr *dirs, const cwstr *idirs, wtxta &list);
ui64 getFileList(const cwstr *dirs, const cwstr *ext, ui64 exts, wtxta &list);
ui64 getFileList(const cwstr *dirs, const cwstr *idirs, const cwstr *ext, ui64 exts, wtxta &list);
bool64 fileExists(const char *fn);
bool64 fileExists(const wchar_t *fn);
void getFileHandleInfo(const wchar_t *fn, BY_HANDLE_FILE_INFORMATION &bhfi);
ui64 getFileSize(const wchar_t *fn);
Time getFileCreationTime(const wchar_t *fn);
Time getFileAccessTime(const wchar_t *fn);
Time getFileWriteTime(const wchar_t *fn);
void setFileTimeFromLocal(const wchar_t *fn, Time t);
void setFileTimeFromUtc(const wchar_t *fn, Time t);
void setCrtModFileTimeFromLocal(const wchar_t *fn, Time t);
void setCrtModFileTimeFromUtc(const wchar_t *fn, Time t);;
// exec
bool64 execProc(wchar_t *cmd);
bool64 execProcBlock(wchar_t *cmd);
// txt + wtxt
NFND
TEND
HEAP_MULTI_THREAD_MODE
// sddb
SWSET
SWSTOP
SWPRINT
SWP
SWPDIFF
SWIDRESET
SWRESET
// stdp
PADL
PADR
PADC
// math
MAX
MIN
// types
UI16_MAX
UI32_MAX
UI64_MAX
UI8
I64_MAX
I32_MAX
I16_MAX
I8_MAX
I64_MIN
I32_MIN
I16_MIN
I8_MIN
NOTHROW