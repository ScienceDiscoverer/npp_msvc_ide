// NLAUNCH
#include <txta>

#pragma comment( lib, "user32" )

#define FILE_READ_CHUNK 4096

#define SWAP_KW KeyWord tmp;tmp = keywords[i];keywords[i] = keywords[j];keywords[j] = tmp

#define NPPMSG  (WM_USER + 1000)
#define NPPM_SAVECURRENTFILE (NPPMSG + 38)

const char *fin = "D:\\P\\NppCustomFuncs\\npp_autos.h";
const char *fout = "C:\\Program Files\\Notepad++\\autoCompletion\\cpp.xml";
const char *ftime = "D:\\P\\NppCustomFuncs\\last_mod_time.dat";
//const char *fout = "D:\\P\\NppCustomFuncs\\xml_out.xml";

FILETIME last_mt;

struct KeyWord
{
	KeyWord() = default;
	KeyWord(const KeyWord &o) = delete;
	KeyWord(KeyWord &&o) = delete;
	KeyWord & operator=(const KeyWord &o) = default;
	KeyWord & operator=(KeyWord &&o) = delete;
	
	txt name;
	txt rett;
	txta args;
};

class KeyWordDarr
{
public:
	KeyWordDarr() : ts(0), s(0), d(nullptr) {}
	KeyWordDarr(const KeyWordDarr &o) = delete;
	KeyWordDarr(KeyWordDarr &&o) = delete;
	
	~KeyWordDarr()
	{
		for(ui64 i = 0; i < s; ++i)
		{
			txtfree(d[i].name);
			txtfree(d[i].rett);
			d[i].args.~txta();
		}
		
		VirtualFree(d, 0, MEM_RELEASE);
	}
	
	KeyWordDarr & operator=(const KeyWordDarr &o) = delete;
	KeyWordDarr & operator=(KeyWordDarr &&o) = delete;
	
	KeyWord & operator[](ui64 i)
	{
#ifdef DEBUG
		if(i >= s) RaiseException(0xE0000011, 0, 0, 0);
#endif
		return d[i];
	}
	const KeyWord & operator[](ui64 i) const
	{
#ifdef DEBUG
		if(i >= s) RaiseException(0xE0000011, 0, 0, 0);
#endif
		return d[i];
	}
	
	ui64 operator~() const { return s; }  // Size operator
	ui64 operator!() const { return ts; } // True Size Operator
	
	KeyWordDarr & operator<<(const KeyWord &data)
	{
		ui64 ns = s + 1;
		if(ts < ns)
		{
			ts = ns < 8 ? 8 : ns * 2; // REALLOC_MULT
			KeyWord *tmp = (KeyWord *)VirtualAlloc(NULL, ts * sizeof(KeyWord),
				MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
			if(d != nullptr)
			{
				memcpy(tmp, d, s * sizeof(KeyWord));
				VirtualFree(d, 0, MEM_RELEASE);
			}
			d = tmp;
		}

		d[s] = data;
		s = ns;
		
		return *this;
	}
	
private:
	ui64 ts;
	ui64 s;
	KeyWord *d;
};

void replaceXmlEnts(txt &t)
{
	static txt amp_ent = L("&amp;");
	for(ui64 i = 0; i < ~t; ++i)
	{
		if(t[i] == '&') // Change to switch if more needed
		{
			txtr(t, i, 1, L("&amp;"));
		}
	}
}

bool addParam(txta &pa, txt &param)
{
	if(param == empty)
	{
		return false;
	}
	
	ui64 dir_s = txtf(param, 0, L("[in"));
	if(dir_s == NFND)
	{
		dir_s = txtf(param, 0, L("[out"));
	}
	if(dir_s == NFND)
	{
		dir_s = txtf(param, 0, L("[optional"));
	}
	
	txt fin_param;
	if(dir_s != NFND) // Windows API func
	{
		ui64 dir_e = txtf(param, dir_s, ']');
		
		if(dir_e == NFND) // This is not full param, comma -> [in, out]
		{
			return false;
		}
		
		txt dir = txtsp(param, dir_s, dir_e);
		if(txtf(dir, 0, L("in")) != NFND)
		{
			fin_param += 'I';
		}
		if(txtf(dir, 0, L("out")) != NFND)
		{
			fin_param += 'O';
		}
		if(txtf(dir, 0, L("optional")) != NFND)
		{
			fin_param += 'o';
		}
		fin_param += ' ';
		
		ui64 n = txtfe(param, TEND, ' ') + 1;		// Name
		ui64 tb = txtf(param, dir_e+1, 'A', 'z');	// Types begin
		ui64 te = txtfe(param, n-1, 'A', 'z');		// Types end
		
		fin_param += txtsp(param, tb, te) + ' ' + txtsp(param, n, TEND);
	}
	
	if(~fin_param != 0)
	{
		replaceXmlEnts(fin_param);
		pa << fin_param;
	}
	else
	{
		replaceXmlEnts(param);
		pa << param;
	}
	
	return true;
}

void execCmd(const char *cmd)
{
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	memset(&pi, 0, sizeof(pi));
	
	// Create Process ============================================================================
	BOOL res = CreateProcessA(
		NULL,				//  [I|O]  Name of the module to be executed, that's it
		(char *)cmd,		// [IO|O]  Command line to be exectued, searches PATH, adds extention
		NULL,				//  [I|O]  Sec. Attrib. for inhereting new process by child processes
		NULL,				//  [I|O]  Sec. Attrib. for inhereting new thread by child processes
		FALSE,				//    [I]  New proc. inherits each inheritable handle
		NULL,				//    [I]  Process creation flags
		NULL,				//  [I|O]  Ptr to environment block of new process (inherit if NULL)
		NULL,				//  [I|O]  Full path to current directory for the process
		&si,				//    [I]  Ptr to STARTUPINFO struct, if dwFlags = 0, def. values used
		&pi);				//    [O]  Ptr to PROCESS_INFORMATION struct with new proc identification info
	// ===========================================================================================
	
	if(res)
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
}

void closeNpp()
{
	// Find Notepad++ window by Class Name
	HWND nppw = FindWindow(L"Notepad++", NULL);
	SendMessage(nppw, NPPM_SAVECURRENTFILE, 0, 0);
	DWORD npp_pid = 0;
	GetWindowThreadProcessId(nppw, &npp_pid);
	HANDLE nppp = OpenProcess(SYNCHRONIZE, FALSE, npp_pid);
	SendMessage(nppw, WM_CLOSE, 0, 0);
	WaitForSingleObject(nppp, 1000);
	CloseHandle(nppp);
}

int wmain()
{
	// Create or open File or Device =================================================================
	HANDLE tf = CreateFileA(
		ftime,							//   [I]  Name of the file or device to create/open
		GENERIC_READ | GENERIC_WRITE,	//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		0,								//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,							// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_ALWAYS,					//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,		//   [I]  Attributes and flags for file/device
		NULL);							// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================
	
	if(tf == INVALID_HANDLE_VALUE)
	{
	#ifdef STD_PRINTER
		p|PE|" Line "|__LINE__|'!'|N|P;
	#endif
		return 1;
	}
	
	txt ftime_txt = sizeof(FILETIME);
	while(ReadFile(tf, ftime_txt, sizeof(FILETIME), *ftime_txt, NULL) && ~ftime_txt)
	{
		// Do nothing...
	}
	
	last_mt = *(FILETIME *)(const char *)ftime_txt;
	
	// Create or open File or Device =================================================================
	HANDLE f = CreateFileA(
		fin,						//   [I]  Name of the file or device to create/open
		GENERIC_READ,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_READ,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		OPEN_EXISTING,				//   [I]  Action to take if file/device exist or not
		FILE_FLAG_SEQUENTIAL_SCAN,	//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================
	
	if(f == INVALID_HANDLE_VALUE)
	{
	#ifdef STD_PRINTER
		p|PE|" Line "|__LINE__|'!'|N|P;
	#endif
		return 1;
	}
	
	FILETIME cur_mt;
	GetFileTime(f, NULL, NULL, &cur_mt);
	
	if(*(ui64 *)&cur_mt == *(ui64 *)&last_mt) // No changes to the input file was found
	{
		execCmd("notepad++ D:\\P\\NppCustomFuncs\\npp_autos.h");
		CloseHandle(f);
		CloseHandle(tf);
		return 0;
	}
	
	DWORD bw;
	WriteFile(tf, &cur_mt, sizeof(FILETIME), &bw, NULL);
	CloseHandle(tf);
	
	txt input, ibuff = FILE_READ_CHUNK;
	while(ReadFile(f, ibuff, FILE_READ_CHUNK, *ibuff, NULL) && ~ibuff)
	{
		input += ibuff;
	}
	
	CloseHandle(f);
	
	bool comment_line = false;
	bool ret_name_done = false;
	bool inside_func = false;
	txta rets;
	txt name;
	txta params;
	txt buf;
	KeyWordDarr keywords;
	
	for(ui64 i = 0; i < ~input; ++i)
	{
		if(comment_line && input[i] != '\n')
		{
			continue;
		}
		
		if(input[i] == '/' && (i == 0 || input[i-1] == '\n'))
		{
			comment_line = true;
			continue;
		}
		
		if(input[i] == '\n')
		{
			if(comment_line)
			{
				comment_line = false;
				continue;
			}
			
			if(!inside_func && input[i-2] != ';')
			{
				KeyWord kw;
				kw.name = buf;
				keywords << kw;
				txtclr(buf);
			}
			
			continue;
		}
		
		if(input[i] == '\r' || input[i] == ';')
		{
			continue;
		}
		
		if(!ret_name_done && input[i] == ' ')
		{
			inside_func = true;
			rets << buf;
			txtclr(buf);
			continue;
		}
		else if(!ret_name_done && input[i] == '(')
		{
			name = buf;
			txtclr(buf);
			ret_name_done = true;
			continue;
		}
		
		if(input[i] == ',')
		{
			if(!addParam(params, buf))
			{
				continue;
			}
			txtclr(buf);
			++i;
			continue;
		}
		else if(input[i] == ')' && i != ~input-1 && input[i+1] == ';')
		{
			addParam(params, buf);
			txt ret;
			for(ui64 j = 0; j < ~rets; ++j)
			{
				ret += rets[j] + L(" ");
			}
			txtd(ret, ~ret-1, 1);
			replaceXmlEnts(ret);
			
			KeyWord out;
			out.name = name;
			out.rett = ret;
			
			for(ui64 j = 0; j < ~params; ++j)
			{
				out.args << params[j];
			}
			
			keywords << out;
			
			ret_name_done = false;
			inside_func = false;
			rets.Clear();
			txtclr(name);
			params.Clear();
			txtclr(buf);
			continue;
		}
		
		buf += input[i];
	}
	
	for(ui64 i = 0; i < ~keywords; ++i) // Insertion sort
	{
		for(ui64 j = i+1; j < ~keywords; ++j)
		{
			if(keywords[j].name < keywords[i].name)
			{
				SWAP_KW;
			}
			else if(keywords[j].name == keywords[i].name)
			{
				if(~keywords[j].args < ~keywords[i].args)
				{
					SWAP_KW;
				}
				else if(~keywords[j].args == ~keywords[i].args)
				{
					for(ui64 k = 0; k < ~keywords[j].args; ++k)
					{
						if(keywords[j].args[k] < keywords[i].args[k])
						{
							SWAP_KW;
							break;
						}
					}
				}
			}
		}
	}
	
	// Create or open File or Device =================================================================
	f = CreateFileA(
		fout,						//   [I]  Name of the file or device to create/open
		GENERIC_WRITE,				//   [I]  Requested access GENERIC_READ|GENERIC_WRITE|0
		FILE_SHARE_WRITE,			//   [I]  Sharing mode FILE_SHARE_READ|WRITE|DELETE|0
		NULL,						// [I|O]  SECURITY_ATTRIBUTES for file, handle inheritability
		CREATE_ALWAYS,				//   [I]  Action to take if file/device exist or not
		0,							//   [I]  Attributes and flags for file/device
		NULL);						// [I|O]  Handle to template file to copy attributes from
	// ===============================================================================================
	
	if(f == INVALID_HANDLE_VALUE)
	{
	#ifdef STD_PRINTER
		p|PE|" Line "|__LINE__|'!'|N|P;
	#endif
		return 1;
	}
	
	const char *head =
		"<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\r\n"
		"<NotepadPlus>\r\n"
		"\t<AutoComplete language=\"C++\">\r\n"
		"\t<Environment ignoreCase=\"no\" startFunc=\"(\" stopFunc=\")\" paramSeparator=\",\" terminal=\";\" additionalWordChar=\"\"/>\r\n";
	WriteFile(f, head, (DWORD)strlen(head), &bw, NULL);
	
	
	for(ui64 i = 0; i < ~keywords; ++i)
	{
		if(keywords[i].rett == empty) // Non-function keyword
		{
			WriteFile(f, "\t\t<KeyWord name=\"", 17, &bw, NULL);
			WriteFile(f, keywords[i].name, (DWORD)~keywords[i].name, &bw, NULL);
			WriteFile(f, "\" />\r\n", 6, &bw, NULL);
			continue;
		}
		
		WriteFile(f, "\t\t<KeyWord name=\"", 17, &bw, NULL);
		WriteFile(f, keywords[i].name, (DWORD)~keywords[i].name, &bw, NULL);
		WriteFile(f, "\" func=\"yes\">\r\n", 15, &bw, NULL);
		
		static txt prev_name;
		do
		{
			WriteFile(f, "\t\t\t<Overload retVal=\"", 21, &bw, NULL);
			WriteFile(f, keywords[i].rett, (DWORD)~keywords[i].rett, &bw, NULL);
			WriteFile(f, "\" >\r\n", 5, &bw, NULL);
			
			for(ui64 j = 0; j < ~keywords[i].args; ++j)
			{
				WriteFile(f, "\t\t\t\t<Param name=\"", 17, &bw, NULL);
				WriteFile(f, keywords[i].args[j], (DWORD)~keywords[i].args[j], &bw, NULL);
				WriteFile(f, "\" />\r\n", 6, &bw, NULL);
			}
			
			WriteFile(f, "\t\t\t</Overload>\r\n", 16, &bw, NULL);
			
			prev_name = keywords[i].name;
			++i;
		}
		while(i < ~keywords && keywords[i].name == prev_name); // Output all the overloads
		
		WriteFile(f, "\t\t</KeyWord>\r\n", 14, &bw, NULL);
		--i;
	}
	
	const char *foot =
		"\t</AutoComplete>\r\n"
		"</NotepadPlus>";
	WriteFile(f, foot, (DWORD)strlen(foot), &bw, NULL);
	
	CloseHandle(f);
	
	closeNpp();
	execCmd("notepad++");
	
	return 0;
}

//<?xml version="1.0" encoding="UTF-8" ?>
//<NotepadPlus>
//	<!-- language doesnt really mean anything, its more of a comment -->
//	<AutoComplete language="C++">
//		<!--
//		Environment specifies how the language should be interpreted. ignoreCase makes autocomplete
//		ignore any casing, start and stopFunc specify what chars a function starts and stops with.
//		param specifies parameter separator and terminal can be used to specify a character that stops
//		any function. Using the same character for different functions results in undefined behaviour.
//		
//		The basic word character are : A-Z a-z 0-9 and '_' 
//		If your function name contains other characters,
//		add your characters in "additionalWordChar" attribute (without separator)
//		in order to make calltip hint work
//		-->
//
//		<Environment ignoreCase="no" startFunc="(" stopFunc=")" paramSeparator="," terminal=";" additionalWordChar=""/>
//		
//		<!--
//		The following items should be alphabetically ordered.
//		func="yes" means the keyword should be treated as a fuction, and thus can be used in the parameter
//		calltip system. If this is the case, the retVal attribute specifies the return value/type. Any
//		following Param tag specifies a parameter, they must be in order. The name attributes specifies
//		the parameter name.
//		-->
//
//		<KeyWord name="bool" />
//
//		<KeyWord name="txtd" func="yes">
//			<Overload retVal="txt &" >
//				<Param name="txt &t" />
//				<Param name="ui64 pos" />
//				<Param name="ui64 n" />
//			</Overload>
//			<Overload retVal="wtxt &" >
//				<Param name="wtxt &t" />
//				<Param name="ui64 pos" />
//				<Param name="ui64 n" />
//			</Overload>
//		</KeyWord>
//
//	</AutoComplete>
//</NotepadPlus>