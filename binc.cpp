// NLAUNCH
#include <fstream>
#include <string>
#include <sstream>
#include <stdp>
#include <txtp>

// Automaton Build Incrementer (binc) program

using namespace std;

size_t fsize;

string getExeDate();
void setFsize();

string fname;
string exe_name;

int wmain(int argc, wchar_t** argv)
{
	if(argc < 3)
	{
		return 1;
	}
	
	wchar_t *fn = argv[1] - 1;
	while(*(++fn) != 0)
	{
		fname += (char)*fn;
	}
	
	exe_name = fname + ".exe";
	fname += "_ver.h";
	
	setFsize();
	
	ifstream ifs(fname, ios::in | ios::binary);
	char* b = new char[fsize+1];
	ifs.read(b, (streamsize)fsize);
	b[fsize] = '\0'; // Very important to add null char at end of file!
	string file = b;
	delete[] b;
	ifs.close();

	if(!wcscmp(argv[2], L"-inc"))
	{
		size_t bp = file.find("const int build = ") + 18;
		size_t sc = file.find_first_of(';', bp);
		
		int ver = stoi(file.substr(bp, sc-bp));
		file.replace(bp, sc-bp, to_string(++ver));
		
		bp = file.find("// last_success_build ") + 22;
		size_t nl = file.find_first_of('\r', bp);
		file.replace(bp, nl-bp, getExeDate());
	}
	else if(!wcscmp(argv[2], L"-dec"))
	{
		size_t bp = file.find("const int build = ") + 18;
		size_t sc = file.find_first_of(';', bp);
		
		int ver = stoi(file.substr(bp, sc-bp));
		file.replace(bp, sc-bp, to_string(--ver));
	}
	
	ofstream ofs(fname, ios::out | ios::trunc | ios::binary);
	ofs << file;
	ofs.close();
	
	return 0;
}

string getExeDate()
{
	WIN32_FILE_ATTRIBUTE_DATA attr;
	SYSTEMTIME ct;
	
	GetFileAttributesExA(exe_name.c_str(), GetFileExInfoStandard, &attr);
	FileTimeToSystemTime(&attr.ftLastWriteTime, &ct);
	
	TIME_ZONE_INFORMATION tz;
	GetTimeZoneInformation(&tz);
	
	
	txtp tp;
	tp|ct.wYear|'.'|
		SP(2,'0')|ct.wMonth|DP|'.'|
		SP(2,'0')|ct.wDay|DP|'_'|
		SP(2,'0')|ct.wHour - tz.Bias/60|DP|':'|
		SP(2,'0')|ct.wMinute|DP|':'|
		SP(2,'0')|ct.wSecond|DP|"_GMT";
	
	if(tz.Bias < 0)
	{
		tp|'+'|tz.Bias/60 * -1;
	}
	else
	{
		tp|'-'|tz.Bias/60;
	}
	
	return string(!tp); 
}

void setFsize()
{
	ifstream ifs(fname, ios::in | ios::ate | ios::binary);
	fsize = (size_t)ifs.tellg();
	ifs.close();
}