// CONSOLE
#include <sddb>

#include <dbghelp.h>
#include <tlhelp32.h>

#pragma comment( lib, "dbghelp" )
#pragma comment( lib, "user32" )

// TODO To cause a breakpoint exception in another process, use the DebugBreakProcess function.
// TODO Implement fully custom StackWalk
// TODO Implement custom PDB reader OR custom symbol exporting
// TODO Visualise full process\thread memory map with DLLs, stack and heap

// void DebugBreak(); Causes a breakpoint exception to occur in the current process. This allows the calling thread
// to signal the debugger to handle the exception. But why, if I can use RaiseException?!
// REASON: //KERNELBASE.dll!RaiseException+0x69 VS KERNELBASE.dll!DebugBreak+0x3 --> many useless code in RaiseExc!

#define COL_END				p|DS;
	
#define COL_THREAD			p|CS;
#define COL_DLL				p|VS;
#define COL_PROC			p|BS;
#define COL_STR				p|YS;
#define COL_DESPAWN			p|RS;
#define COL_LOAD_SUCC		p|GS;
#define COL_LOAD_FAIL		p|RS;

#define COL_TRACE_PLUS		p|VS;
#define COL_TRACE_OFFSET	p|BS;
#define COL_TRACE_SYMB		p|YS;
#define COL_TRACE_LINE		p|CS;

cstr vc_source = L(
	"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.34.31933\\crt\\src\\vcruntime\\");

typedef struct _DLL_LIST_ITEM
{
	LPWSTR n; // Name
	LPVOID b; // Base address
	DWORD en; // Number of exceptions that happned in this process
	DWORD id;
	struct _DLL_LIST_ITEM *next;
} DLL_LIST_ITEM;

DLL_LIST_ITEM *dlls;
DLL_LIST_ITEM *threads;
DLL_LIST_ITEM *procs;

DWORD decision(DWORD chance);
void dllAttach(DLL_LIST_ITEM **root, LPCWSTR name, LPVOID base, DWORD id);
void dllDestoryList(DLL_LIST_ITEM **root);
HANDLE fndHandle(DWORD id, DLL_LIST_ITEM *what);
LPWSTR fndProcName(DWORD id);
DWORD * getProcExceptStat(DWORD id);
cstr stripPath(LPCSTR path);
void checkSymType(SYM_TYPE t);
BOOL __declspec(nothrow) readProcMemCallback(HANDLE ph, DWORD64 baddr, PVOID buff, DWORD sz, LPDWORD bread);
bool fileExists(const char *fn);
BOOL WINAPI onExitHandler(DWORD ctrl_sig) noexcept;
txt getProcPath(DWORD pid);

int wmain(int argc, wchar_t **argv)
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	static CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(oh, &csbi);
	COORD buf = csbi.dwSize;
	buf.Y = 9000;
	SetConsoleScreenBufferSize(oh, buf);
	SetConsoleCtrlHandler(onExitHandler, TRUE);
	
	SetConsoleTitle(L"SDDBG");
	
engage_inject_mode:
	
	BOOL res = FALSE;
	static STARTUPINFOW si;
	static PROCESS_INFORMATION pi;
	
	if(argc != 2)
	{
		p|"Move "|G|"MOUSE POINTER"|" over process "|B|"WINDOW"|" to be attached and press "|Y|"ANY KEY"|"..."|P;
		
		POINT pnt;
		GetCursorPos(&pnt);
		HWND wnd = WindowFromPoint(pnt);
		DWORD pid;
		GetWindowThreadProcessId(wnd, &pid);
		
		if(getProcPath(pid) == L("C:\\Windows\\System32\\cmd.exe"))
		{
			HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);		
			PROCESSENTRY32 lppe;						//	struct tagPROCESSENTRY32
			lppe.dwSize = sizeof(PROCESSENTRY32);		//	DWORD		dwSize;
			if(!Process32First(snap, &lppe))			//	DWORD		cntUsage;
			{											//	DWORD		th32ProcessID;
				p|PE|" in Process32First"|N|P;			//	ULONG_PTR	th32DefaultHeapID;
				return 1;								//	DWORD		th32ModuleID;
			}											//	DWORD		cntThreads;
			do											//	DWORD		th32ParentProcessID
			{											//	LONG		pcPriClassBase;
				if(lppe.th32ParentProcessID == pid)		//	DWORD		dwFlags;
				{										//	CHAR		szExeFile[MAX_PATH];
					if(getProcPath(lppe.th32ProcessID) == L("C:\\Windows\\System32\\conhost.exe"))
					{
						continue;
					}
					
					pid = lppe.th32ProcessID;
					goto proc_attach;
				}
			}
			while(Process32Next(snap, &lppe));
			CloseHandle(snap);
			
			p|R|"Failed to find console process in CMD "|B|pid|R|"!"|N|P;
			return 1;
		}
		
	proc_attach:
		if(!DebugActiveProcess(pid))
		{
			p|PE|N;
			p|R|"Failed to attach to process "|B|pid|R|"!"|N|P;
			return 1;
		}
		
		SetConsoleCursorPosition(oh, { 0, 0 });
		p|S(100);
		SetConsoleCursorPosition(oh, { 0, 0 });
		// To detach from the process being debugged, the debugger should use the DebugActiveProcessStop function.
	}
	else
	{
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		memset(&pi, 0, sizeof(pi));
		
		// Create Process ========================================================================================
		res = CreateProcessW(
			argv[1],				//  [I|O]  Name of the module to be executed, that's it
			NULL,					// [IO|O]  Command line to be exectued, searches PATH, adds extention
			NULL,					//  [I|O]  Sec. Attrib. for inhereting new process by child processes
			NULL,					//  [I|O]  Sec. Attrib. for inhereting new thread by child processes
			FALSE,					//    [I]  New proc. inherits each inheritable handle
			CREATE_NEW_CONSOLE |
			DEBUG_PROCESS,			//    [I]  Process creation flags
			NULL,					//  [I|O]  Ptr to environment block of new process (inherit if NULL)
			NULL,					//  [I|O]  Full path to current directory for the process
			&si,					//    [I]  Ptr to STARTUPINFO struct, if dwFlags = 0, def. values used
			&pi);					//    [O]  Ptr to PROCESS_INFORMATION struct with new proc identification info
		// =======================================================================================================
	}
	
	//DEBUG_ONLY_THIS_PROCESS 0x00000002 ---> To ignore child procs created by debugee
	
	DEBUG_EVENT de;
	DWORD continue_status = DBG_EXCEPTION_NOT_HANDLED;
	//typedef struct _DEBUG_EVENT {
	//    DWORD dwDebugEventCode;
	//    DWORD dwProcessId;
	//    DWORD dwThreadId;
	//    union {
	//        EXCEPTION_DEBUG_INFO Exception;
	//        CREATE_THREAD_DEBUG_INFO CreateThread;
	//        CREATE_PROCESS_DEBUG_INFO CreateProcessInfo;
	//        EXIT_THREAD_DEBUG_INFO ExitThread;
	//        EXIT_PROCESS_DEBUG_INFO ExitProcess;
	//        LOAD_DLL_DEBUG_INFO LoadDll;
	//        UNLOAD_DLL_DEBUG_INFO UnloadDll;
	//        OUTPUT_DEBUG_STRING_INFO DebugString;
	//        RIP_INFO RipInfo;
	//    } u;
	//} DEBUG_EVENT, *LPDEBUG_EVENT;
	
	while(1)
	{
		WaitForDebugEventEx(&de, INFINITE);
		
		switch(de.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:      /* 1 */
		{
			//typedef struct _EXCEPTION_DEBUG_INFO {
			//    EXCEPTION_RECORD ExceptionRecord;
			//    DWORD dwFirstChance;
			//} EXCEPTION_DEBUG_INFO, *LPEXCEPTION_DEBUG_INFO;
			
			HANDLE hp = fndHandle(de.dwProcessId, procs);
			
			pexcept(&de.u.Exception.ExceptionRecord, hp);
			
			HANDLE ht = fndHandle(de.dwThreadId, threads);
			p|"PROCESS: "|fndProcName(de.dwProcessId)|" ("|de.dwProcessId|") THREAD: "|de.dwThreadId|N|N;
			
			// Skip regs/stack listing for first info. exception generated by kernel for each process
			if(de.u.Exception.ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT &&
				++(*getProcExceptStat(de.dwProcessId)) == 1)
			{
				continue_status = DBG_CONTINUE;
				break;
			}
			
			HWND wnd = GetConsoleWindow();
			SetForegroundWindow(wnd);

			// No idea what are this...
			// CONTEXT_SEGMENTS -> SegDs, SegEs, SegFs, and SegGs
			// CONTEXT_FLOATING_POINT -> Xmm0-Xmm15.
			// CONTEXT_DEBUG_REGISTERS -> Dr0-Dr3 and Dr6-Dr7.
			// Full list:
			// CONTEXT_CONTROL         (CONTEXT_AMD64 | 0x00000001L)
			// CONTEXT_INTEGER         (CONTEXT_AMD64 | 0x00000002L)
			// CONTEXT_SEGMENTS        (CONTEXT_AMD64 | 0x00000004L)
			// CONTEXT_FLOATING_POINT  (CONTEXT_AMD64 | 0x00000008L)
			// CONTEXT_DEBUG_REGISTERS (CONTEXT_AMD64 | 0x00000010L)
			// CONTEXT_FULL            (CONTEXT_CONTROL | CONTEXT_INTEGER | CONTEXT_FLOATING_POINT)
			// CONTEXT_ALL             (CONTEXT_CONTROL | CONTEXT_INTEGER | 
            // CONTEXT_SEGMENTS | CONTEXT_FLOATING_POINT | CONTEXT_DEBUG_REGISTERS)
		
			CONTEXT cx;
			memset(&cx, 0x11, sizeof(cx));
			cx.ContextFlags = CONTEXT_ALL;
			res = GetThreadContext(ht, &cx);
			if(!res)
			{
				p|PE|N;
				p|R|"GETTHREADCONTEXT FAILED!"|N;
			}
			
			p|"RIP  0x"|H|cx.Rip|"  RSP  0x"|H|cx.Rsp|N;
			p|"EFL  0x"|H|cx.EFlags|N|N;
			
			p|"RAX  0x"|H|cx.Rax|"  RBX  0x"|H|cx.Rbx|N;
			p|"RCX  0x"|H|cx.Rcx|"  RDX  0x"|H|cx.Rdx|N;
			p|"RSI  0x"|H|cx.Rsi|"  RDI  0x"|H|cx.Rdi|N;
			p|"RBP  0x"|H|cx.Rbp|N|N;
			
			p|"R08  0x"|H|cx.R8|"  R09  0x"|H|cx.R9|N;
			p|"R10  0x"|H|cx.R10|"  R11  0x"|H|cx.R11|N;
			p|"R12  0x"|H|cx.R12|"  R13  0x"|H|cx.R13|N;
			p|"R14  0x"|H|cx.R14|"  R15  0x"|H|cx.R15|N|N;
			
			/* typedef struct _tagSTACKFRAME64 {
				ADDRESS64   AddrPC;               // program counter
				ADDRESS64   AddrReturn;           // return address
				ADDRESS64   AddrFrame;            // frame pointer
				ADDRESS64   AddrStack;            // stack pointer
				ADDRESS64   AddrBStore;           // backing store pointer
				PVOID       FuncTableEntry;       // pointer to pdata/fpo or NULL
				DWORD64     Params[4];            // possible arguments to the function
				BOOL        Far;                  // WOW far call
				BOOL        Virtual;              // is this a virtual frame?
				DWORD64     Reserved[3];
				KDHELP64    KdHelp;
			} STACKFRAME64, *LPSTACKFRAME64;
			
			typedef struct _tagADDRESS64 {
				DWORD64       Offset;
				WORD          Segment;
				ADDRESS_MODE  Mode;
			} ADDRESS64, *LPADDRESS64;
			
			AddrMode1616 0
			AddrMode1632 1
			AddrModeReal 2
			AddrModeFlat 3 */
			
			/* typedef struct _IMAGEHLP_MODULE64 {
			 DWORD    SizeOfStruct;
			 DWORD64  BaseOfImage;
			 DWORD    ImageSize;
			 DWORD    TimeDateStamp;
			 DWORD    CheckSum;
			 DWORD    NumSyms;
			 SYM_TYPE SymType;
			 CHAR     ModuleName[32];
			 CHAR     ImageName[256];
			 CHAR     LoadedImageName[256];
			 CHAR     LoadedPdbName[256];
			 DWORD    CVSig;
			 CHAR     *CVData[MAX_PATH  3];
			 DWORD    PdbSig;
			 GUID     PdbSig70;
			 DWORD    PdbAge;
			 BOOL     PdbUnmatched;
			 BOOL     DbgUnmatched;
			 BOOL     LineNumbers;
			 BOOL     GlobalSymbols;
			 BOOL     TypeInfo;
			 BOOL     SourceIndexed;
			 BOOL     Publics;
			 DWORD    MachineType;
			 DWORD    Reserved;
			} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64; */
			
			/* typedef struct _SYMBOL_INFO {
			 ULONG   SizeOfStruct;
			 ULONG   TypeIndex;
			 ULONG64 Reserved[2];
			 ULONG   Index;
			 ULONG   Size;
			 ULONG64 ModBase;
			 ULONG   Flags;
			 ULONG64 Value;
			 ULONG64 Address;
			 ULONG   Register;
			 ULONG   Scope;
			 ULONG   Tag;
			 ULONG   NameLen;
			 ULONG   MaxNameLen;
			 CHAR    Name[1];
			} SYMBOL_INFO, *PSYMBOL_INFO; */
			
			STACKFRAME64 sf;  // MB COZ MIDDLE MEMBE OF ADDRPC IS NOT SET TO 0??!?! MBMB
			sf.AddrPC.Offset = cx.Rip;
			sf.AddrPC.Mode = AddrModeFlat;
			sf.AddrFrame.Offset = cx.Rbp;
			sf.AddrFrame.Mode = AddrModeFlat;
			sf.AddrStack.Offset = cx.Rsp;
			sf.AddrStack.Mode = AddrModeFlat;
			
			SYMBOL_INFO *psi;
			psi = (SYMBOL_INFO *)HeapAlloc(GetProcessHeap(), 0,
				sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(wchar_t));
			memset(psi, 0x0, sizeof(SYMBOL_INFO) + (MAX_SYM_NAME - 1) * sizeof(wchar_t));
			psi->SizeOfStruct = sizeof(SYMBOL_INFO);
			psi->MaxNameLen = MAX_SYM_NAME;
			DWORD64 displace;
			
			p|"STACK TRACE"|N;
			
			do
			{
				IMAGEHLP_MODULE64 mi;
				mi.SizeOfStruct = sizeof(mi);
				res = SymGetModuleInfo64(
					hp,
					(DWORD64)sf.AddrPC.Offset,
					&mi);
				
				SymFromAddr(hp, sf.AddrPC.Offset, &displace, psi);
				
				
				p|"0x"|H|psi->Address;
				COL_TRACE_PLUS
				p|"+";
				COL_TRACE_OFFSET
				p|"0x"|H|displace;
				COL_END
				p|"|"|stripPath(mi.LoadedImageName)|"|";
				COL_TRACE_SYMB
				p|psi->Name;
				COL_END
				p|"|";
				
				IMAGEHLP_LINE64 ln;
				ln.SizeOfStruct = sizeof(ln);
				DWORD displace0;
				res = SymGetLineFromAddr64(hp, sf.AddrPC.Offset, &displace0, &ln);
				
				if(res)
				{
					p|stripPath(ln.FileName)|"(";
					COL_TRACE_LINE
					p|ln.LineNumber;
					COL_END
					p|")";
					
					txt std_lib_src;
					if(!fileExists(ln.FileName)) // Try to find file in VS sources
					{
						std_lib_src = vc_source;
						std_lib_src += stripPath(ln.FileName);
						ln.FileName = (char *)std_lib_src;
					}
					
					if(fileExists(ln.FileName)) // Jump to line in Notepad++
					{
						_dbgOpenFile(ln.FileName);
						_dbgSelLine(ln.LineNumber);
					}
					p|DC|P;
					//_dbgCleanSelects();
				}
				
				p|N;
				
				res = StackWalk64(
					IMAGE_FILE_MACHINE_AMD64,
					hp,
					ht,
					&sf,
					&cx,
					readProcMemCallback,
					SymFunctionTableAccess64,
					SymGetModuleBase64,
					NULL);
				if(!res)
				{
					break;
				}
			}
			while(sf.AddrReturn.Offset != 0);
				
			HeapFree(GetProcessHeap(), 0, psi);
			p|N;
		}
			
			continue_status = decision(de.u.Exception.dwFirstChance);
			break;
		case CREATE_THREAD_DEBUG_EVENT:  /* 2 */
			//typedef struct _CREATE_THREAD_DEBUG_INFO {
			//  HANDLE                 hThread;
			//  LPVOID                 lpThreadLocalBase;
			//  LPTHREAD_START_ROUTINE lpStartAddress;
			//} CREATE_THREAD_DEBUG_INFO, *LPCREATE_THREAD_DEBUG_INFO;
			dllAttach(&threads, NULL, (LPVOID)de.u.CreateThread.hThread, de.dwThreadId);
			COL_THREAD;
			p|"THREAD SPAWNED";
			COL_END;
			p|" BY "|fndProcName(de.dwProcessId)|": 0x"|H|de.u.CreateThread.lpThreadLocalBase|" ID: "|de.dwThreadId|N;
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case CREATE_PROCESS_DEBUG_EVENT: /* 3 */
			//typedef struct _CREATE_PROCESS_DEBUG_INFO {
			//  HANDLE                 hFile;
			//  HANDLE                 hProcess;
			//  HANDLE                 hThread;
			//  LPVOID                 lpBaseOfImage;
			//  DWORD                  dwDebugInfoFileOffset;
			//  DWORD                  nDebugInfoSize;
			//  LPVOID                 lpThreadLocalBase;
			//  LPTHREAD_START_ROUTINE lpStartAddress;
			//  LPVOID                 lpImageName;
			//  WORD                   fUnicode;
			//} CREATE_PROCESS_DEBUG_INFO, *LPCREATE_PROCESS_DEBUG_INFO;
			
			
			//PROCESS SPAWNED SUCCESSFULLY!
			//PROCESS SPAWNED: 27096 000000000014F140 PDB LOADED! DBG: 0
			
			COL_PROC
			
			if(argc != 2)
			{
				p|"ATTACHED TO PROCESS";
			}
			else
			{
				p|"PROCESS SPAWNED";
			}
			
			COL_END
			p|": ";
		{
			wchar_t nm[MAX_PATH];
			GetFinalPathNameByHandle(de.u.CreateProcessInfo.hFile, nm, MAX_PATH, 0);
			p|nm + 4|" ID: "|de.dwProcessId; // Skip '\\?\' syntax
			
			SymInitialize(de.u.CreateProcessInfo.hProcess, NULL, FALSE);
			SymLoadModuleExW(
				de.u.CreateProcessInfo.hProcess,
				NULL,
				nm,
				NULL,
				(DWORD64)de.u.CreateProcessInfo.lpBaseOfImage,
				0,
				NULL,
				0);
				
			IMAGEHLP_MODULE64 mi;
			mi.SizeOfStruct = sizeof(mi);
			res = SymGetModuleInfo64(
				de.u.CreateProcessInfo.hProcess,
				(DWORD64)de.u.CreateProcessInfo.lpBaseOfImage,
				&mi);
			
			if(res)
			{
				checkSymType(mi.SymType);
			}
		
			// de.u.CreateProcessInfo.nDebugInfoSize -> Useless. Probably legacy info (always 0)
			CloseHandle(de.u.CreateProcessInfo.hFile);
			COL_THREAD
			p|"MAIN THREAD";
			COL_END
			p|": 0x"|H|de.u.CreateProcessInfo.lpThreadLocalBase|" ID: "|de.dwThreadId|N;
			dllAttach(&threads, NULL, (LPVOID)de.u.CreateProcessInfo.hThread, de.dwThreadId);
			
			SIZE_T l = wcslen(nm);
			LPCWSTR lslash = nm+l; // Null wchar aka. Path End
			while(*(--lslash) != L'\\' && lslash >= nm)
			{
				;
			}
			
			dllAttach(&procs, lslash+1, (LPVOID)de.u.CreateProcessInfo.hProcess, de.dwProcessId);
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case EXIT_THREAD_DEBUG_EVENT:    /* 4 */
			//typedef struct _EXIT_THREAD_DEBUG_INFO {
			//  DWORD dwExitCode;
			//} EXIT_THREAD_DEBUG_INFO, *LPEXIT_THREAD_DEBUG_INFO;
			COL_DESPAWN
			p|"THREAD DIED";
			COL_END
			p|" IN " |fndProcName(de.dwProcessId)|": "|de.dwThreadId|" C: "|de.u.ExitThread.dwExitCode|N;
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case EXIT_PROCESS_DEBUG_EVENT:   /* 5 */
			//typedef struct _EXIT_PROCESS_DEBUG_INFO {
			//  DWORD dwExitCode;
			//} EXIT_PROCESS_DEBUG_INFO, *LPEXIT_PROCESS_DEBUG_INFO;
			COL_DESPAWN
			p|"PROCESS DIED";
			COL_END
			p|": "|fndProcName(de.dwProcessId)|" C: "|de.u.ExitProcess.dwExitCode|N;
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			goto stop_debugging;
		case LOAD_DLL_DEBUG_EVENT:       /* 6 */
			//typedef struct _LOAD_DLL_DEBUG_INFO {
			//  HANDLE hFile;
			//  LPVOID lpBaseOfDll;
			//  DWORD  dwDebugInfoFileOffset;
			//  DWORD  nDebugInfoSize;
			//  LPVOID lpImageName;
			//  WORD   fUnicode;
			//} LOAD_DLL_DEBUG_INFO, *LPLOAD_DLL_DEBUG_INFO;
			COL_DLL
			p|"DLL LOADED";
			COL_END
			p|" BY "|fndProcName(de.dwProcessId)|": 0x"|H|de.u.LoadDll.lpBaseOfDll|' ';
		{
			wchar_t name[MAX_PATH];
			GetFinalPathNameByHandle(de.u.LoadDll.hFile, name, MAX_PATH, 0);
			p|name + 4; // Skip '\\?\' syntax
			dllAttach(&dlls, name + 4, de.u.LoadDll.lpBaseOfDll, 0);
			
			HANDLE hp = fndHandle(de.dwProcessId, procs);
			
			SymLoadModuleExW(
				hp,
				NULL,
				name,
				NULL,
				(DWORD64)de.u.LoadDll.lpBaseOfDll,
				0,
				NULL,
				0);
				
			IMAGEHLP_MODULE64 mi;
			mi.SizeOfStruct = sizeof(mi);
			res = SymGetModuleInfo64(
				hp,
				(DWORD64)de.u.LoadDll.lpBaseOfDll,
				&mi);
				
			if(res)
			{
				checkSymType(mi.SymType);
			}
			
			CloseHandle(de.u.CreateProcessInfo.hFile);
			
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case UNLOAD_DLL_DEBUG_EVENT:     /* 7 */
		{
			//typedef struct _UNLOAD_DLL_DEBUG_INFO {
			//  LPVOID lpBaseOfDll;
			//} UNLOAD_DLL_DEBUG_INFO, *LPUNLOAD_DLL_DEBUG_INFO;
			COL_DESPAWN
			p|"DLL UNLOADED";
			COL_END
			p|" BY "|fndProcName(de.dwProcessId)|": ";
			DLL_LIST_ITEM *dll = dlls;
			while(dll != NULL)
			{
				if(dll->b == de.u.UnloadDll.lpBaseOfDll)
				{
					p|dll->n;
					break;
				}
				dll = dll->next;
			}
			p|N;
		}
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		case OUTPUT_DEBUG_STRING_EVENT:  /* 8 */
		{
			//typedef struct _OUTPUT_DEBUG_STRING_INFO {
			//  LPSTR lpDebugStringData;
			//  WORD  fUnicode;
			//  WORD  nDebugStringLength;
			//} OUTPUT_DEBUG_STRING_INFO, *LPOUTPUT_DEBUG_STRING_INFO;
			SIZE_T s = de.u.DebugString.fUnicode ? 
				de.u.DebugString.nDebugStringLength :
				de.u.DebugString.nDebugStringLength * sizeof(wchar_t);
			LPVOID ds = HeapAlloc(GetProcessHeap(), 0, s);
			memset(ds, 0x0, s);
			
			HANDLE ph = fndHandle(de.dwProcessId, procs);
			
			ReadProcessMemory(ph, de.u.DebugString.lpDebugStringData, ds, s, NULL);
			
			if(de.u.DebugString.fUnicode)
			{
				COL_STR
				p|"DEBUG WSTR";
				COL_END
				p|" BY "|fndProcName(de.dwProcessId)|": |"|(LPWSTR)ds|"|"|N;
			}
			else
			{
				COL_STR
				p|"DEBUG WSTR";
				COL_END
				p|" BY "|fndProcName(de.dwProcessId)|": |"|(LPSTR)ds|"|"|N;
			}
			
			HeapFree(GetProcessHeap(), 0, ds);
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
		}	break;
		case RIP_EVENT:                  /* 9 */
			COL_DESPAWN
			p|"RIP_EVENT ---> IGNORING!"|N;
			COL_END
			continue_status = DBG_EXCEPTION_NOT_HANDLED;
			break;
		default:
			break;
		}
		
		ContinueDebugEvent(de.dwProcessId, de.dwThreadId, continue_status);
	}
	
stop_debugging:
	
	if(argc == 2)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	
	dllDestoryList(&dlls);
	dllDestoryList(&threads);
	
	p|TB|"Press any key to go into siphon mode..."|P|CLS;
	
	argc = 1;
	goto engage_inject_mode;
	
	//return 0;
}

DWORD decision(DWORD chance)
{
	static HANDLE ih = GetStdHandle(STD_INPUT_HANDLE);
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	p|DC|C|"H"|"ANDLE | ";
	p|G|"I"|"GNORE [";
	
	if(chance)
	{
		p|G|"FIRST CHANCE";
	}
	else
	{
		p|R|"FINAL CHANCE";
	}
	
	p|']'|N;
	
	while(1)
	{
		DWORD cmod;
		GetConsoleMode(ih, &cmod);
		SetConsoleMode(ih, cmod & 0xFFFFFFF9); // ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT)
		char ch;
		static DWORD bw;
		ReadFile(ih, &ch, 1, &bw, NULL);
		SetConsoleMode(ih, cmod);
		
		switch(ch)
		{
		case 'h':
		case 'H':
			p|RC;
			return DBG_CONTINUE;
		case 'i':
		case 'I':
			p|RC;
			return DBG_EXCEPTION_NOT_HANDLED;
		default:
			break;
		}
	}
}

void dllAttach(DLL_LIST_ITEM **root, LPCWSTR name, LPVOID base, DWORD id)
{
	DLL_LIST_ITEM *head = *root;
	
	if(head != NULL)
	{
		while(head->next != NULL)
		{
			head = head->next;
		}
	}
	
	static HANDLE heap = GetProcessHeap();
	LPVOID nh = NULL;
	if(name != NULL)
	{
		SIZE_T l = wcslen(name) + 1;
		nh = HeapAlloc(heap, 0, l * sizeof(wchar_t));
		CopyMemory(nh, name, l * sizeof(wchar_t));
	}
	
	DLL_LIST_ITEM tmp;
	tmp.n = (LPWSTR)nh;
	tmp.b = base;
	tmp.en = 0;
	tmp.id = id;
	tmp.next = NULL;
	LPVOID ih = HeapAlloc(heap, 0, sizeof(DLL_LIST_ITEM));
	CopyMemory(ih, &tmp, sizeof(DLL_LIST_ITEM));
	
	if(head == NULL)
	{
		*root = (DLL_LIST_ITEM *)ih;
	}
	else
	{
		head->next = (DLL_LIST_ITEM *)ih;
	}
}

void dllDestoryList(DLL_LIST_ITEM **root)
{
	DLL_LIST_ITEM *cur = *root;
	while(cur != NULL)
	{
		DLL_LIST_ITEM *tmp = cur;
		static HANDLE heap = GetProcessHeap();
		HeapFree(heap, 0, cur->n);
		HeapFree(heap, 0, cur);
		cur = tmp->next;
	}
	*root = NULL;
}

HANDLE fndHandle(DWORD id, DLL_LIST_ITEM *what)
{
	DLL_LIST_ITEM *itm = what;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return (HANDLE)itm->b;
		}
		
		itm = itm->next;
	}
	
	return NULL;
}

LPWSTR fndProcName(DWORD id)
{
	DLL_LIST_ITEM *itm = procs;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return itm->n;
		}
		
		itm = itm->next;
	}
	
	return NULL;
}

DWORD * getProcExceptStat(DWORD id)
{
	DLL_LIST_ITEM *itm = procs;
	while(itm != 0)
	{
		if(itm->id == id)
		{
			return &itm->en;
		}
		
		itm = itm->next;
	}
	
	static DWORD dummy;
	dummy = 0;
	return &dummy;
}

cstr stripPath(LPCSTR path)
{
	SIZE_T l = strl(path);
	LPCSTR lslash = path+l; // Null wchar aka. Path End
	LPCSTR end = lslash;
	while(*(--lslash) != '\\' && lslash >= path)
	{
		// Do nothing...
	}
	
	return cstr({ lslash + 1, (ui64)(end - lslash - 1) });
}

void checkSymType(SYM_TYPE t)
{
	static HANDLE oh = GetStdHandle(STD_OUTPUT_HANDLE);
	
	COL_LOAD_SUCC
	switch(t)
	{
	case SymCoff: /* = 0 */
		p|" COFF LOADED!";
		break;
	case SymCv:
		p|" CODEVIEW LOADED!";
		break;
	case SymDeferred:
		p|" LOADING DEFERRED!";
		break;
	case SymDia:
		p|" DIA LOADED!";
		break;
	case SymExport:
		p|" DLL EXPORT LOADED!";
		break;
	case SymPdb:
		p|" PDB LOADED!";
		break;
	case SymSym:
		p|" SYM LOADED!";
		break;
	case SymVirtual:
		p|" SYM VIRTUAL BS!";
		break;
	case NumSymTypes:
		p|" NUMSYMTYPES BS!";
		break;
	case SymNone:
		COL_LOAD_FAIL
		p|" NO SYMBOLS FOUND...";
		break;
	default:
		break;
	}
	COL_END
	p|N;
}

BOOL __declspec(nothrow) readProcMemCallback(HANDLE ph, DWORD64 baddr, PVOID buff, DWORD sz, LPDWORD bread)
{
	return ReadProcessMemory(ph, (LPCVOID)baddr, buff, sz, (SIZE_T *)bread);
}

bool fileExists(const char *fn)
{
	DWORD attr = GetFileAttributesA(fn);
	if(attr == INVALID_FILE_ATTRIBUTES)
	{
		return false;
	}
	
	return true;
}

BOOL WINAPI onExitHandler(DWORD ctrl_sig) noexcept
{
	if(ctrl_sig != CTRL_CLOSE_EVENT)
	{
		return FALSE;
	}
	
	_dbgDeselectAll();
	
	return FALSE; // Handlers are added in a list FILO. Def. first. Returning FALSE calls next in a list
}

txt getProcPath(DWORD pid)
{
	txt name = MAX_PATH;
	txtssz(name, MAX_PATH);
	HANDLE proc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if(proc != NULL)
	{
		QueryFullProcessImageNameA(proc, 0, name, *name);
		CloseHandle(proc);
	}
	
	return name;
}

//DBG_CONTINUE 0x00010002L

// If the thread specified by the dwThreadId parameter previously reported an EXCEPTION_DEBUG_EVENT debugging event, the function stops all exception processing and continues the thread and the exception is marked as handled. For any other debugging event, this flag simply continues the thread.

//DBG_EXCEPTION_NOT_HANDLED 0x80010001L

// If the thread specified by dwThreadId previously reported an EXCEPTION_DEBUG_EVENT debugging event, the function continues exception processing. If this is a first-chance exception event, the search and dispatch logic of the structured exception handler is used; otherwise, the process is terminated. For any other debugging event, this flag simply continues the thread.

//DBG_REPLY_LATER 0x40010001L

// Supported in Windows 10, version 1507 or above, this flag causes dwThreadId to replay the existing breaking event after the target continues. By calling the SuspendThread API against dwThreadId, a debugger can resume other threads in the process and later return to the breaking.

// Context Frame
//
//  This frame has a several purposes: 1) it is used as an argument to
//  NtContinue, 2) it is used to constuct a call frame for APC delivery,
//  and 3) it is used in the user level thread creation routines.
//
//
// The flags field within this record controls the contents of a CONTEXT
// record.
//
// If the context record is used as an input parameter, then for each
// portion of the context record controlled by a flag whose value is
// set, it is assumed that that portion of the context record contains
// valid context. If the context record is being used to modify a threads
// context, then only that portion of the threads context is modified.
//
// If the context record is used as an output parameter to capture the
// context of a thread, then only those portions of the thread's context
// corresponding to set flags will be returned.
//
// CONTEXT_CONTROL specifies SegSs, Rsp, SegCs, Rip, and EFlags.
//
// CONTEXT_INTEGER specifies Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, and R8-R15.
//
// CONTEXT_SEGMENTS specifies SegDs, SegEs, SegFs, and SegGs.
//
// CONTEXT_FLOATING_POINT specifies Xmm0-Xmm15.
//
// CONTEXT_DEBUG_REGISTERS specifies Dr0-Dr3 and Dr6-Dr7.
//

//typedef struct DECLSPEC_ALIGN(16) DECLSPEC_NOINITALL _CONTEXT {
//
//    //
//    // Register parameter home addresses.
//    //
//    // N.B. These fields are for convience - they could be used to extend the
//    //      context record in the future.
//    //
//
//    DWORD64 P1Home;
//    DWORD64 P2Home;
//    DWORD64 P3Home;
//    DWORD64 P4Home;
//    DWORD64 P5Home;
//    DWORD64 P6Home;
//
//    //
//    // Control flags.
//    //
//
//    DWORD ContextFlags;
//    DWORD MxCsr;
//
//    //
//    // Segment Registers and processor flags.
//    //
//
//    WORD   SegCs;
//    WORD   SegDs;
//    WORD   SegEs;
//    WORD   SegFs;
//    WORD   SegGs;
//    WORD   SegSs;
//    DWORD EFlags;
//
//    //
//    // Debug registers
//    //
//
//    DWORD64 Dr0;
//    DWORD64 Dr1;
//    DWORD64 Dr2;
//    DWORD64 Dr3;
//    DWORD64 Dr6;
//    DWORD64 Dr7;
//
//    //
//    // Integer registers.
//    //
//
//    DWORD64 Rax;
//    DWORD64 Rcx;
//    DWORD64 Rdx;
//    DWORD64 Rbx;
//    DWORD64 Rsp;
//    DWORD64 Rbp;
//    DWORD64 Rsi;
//    DWORD64 Rdi;
//    DWORD64 R8;
//    DWORD64 R9;
//    DWORD64 R10;
//    DWORD64 R11;
//    DWORD64 R12;
//    DWORD64 R13;
//    DWORD64 R14;
//    DWORD64 R15;
//
//    //
//    // Program counter.
//    //
//
//    DWORD64 Rip;
//
//    //
//    // Floating point state.
//    //
//
//    union {
//        XMM_SAVE_AREA32 FltSave;
//        struct {
//            M128A Header[2];
//            M128A Legacy[8];
//            M128A Xmm0;
//            M128A Xmm1;
//            M128A Xmm2;
//            M128A Xmm3;
//            M128A Xmm4;
//            M128A Xmm5;
//            M128A Xmm6;
//            M128A Xmm7;
//            M128A Xmm8;
//            M128A Xmm9;
//            M128A Xmm10;
//            M128A Xmm11;
//            M128A Xmm12;
//            M128A Xmm13;
//            M128A Xmm14;
//            M128A Xmm15;
//        } DUMMYSTRUCTNAME;
//    } DUMMYUNIONNAME;
//
//    //
//    // Vector registers.
//    //
//
//    M128A VectorRegister[26];
//    DWORD64 VectorControl;
//
//    //
//    // Special debug control registers.
//    //
//
//    DWORD64 DebugControl;
//    DWORD64 LastBranchToRip;
//    DWORD64 LastBranchFromRip;
//    DWORD64 LastExceptionToRip;
//    DWORD64 LastExceptionFromRip;
//} CONTEXT, *PCONTEXT;