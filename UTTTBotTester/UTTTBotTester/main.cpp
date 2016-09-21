#include <windows.h> 
#include <string>
#include <iostream>
#include <tchar.h>
#include <stdio.h> 
#include <strsafe.h>

using namespace std;

#define BUFSIZE 4096 

HANDLE g_hChildStd_IN_Rd = NULL;
HANDLE g_hChildStd_IN_Wr = NULL;
HANDLE g_hChildStd_OUT_Rd = NULL;
HANDLE g_hChildStd_OUT_Wr = NULL;

//HANDLE g_hInputFile = NULL;

void CreateChildProcess(string dir);
void WriteToPipe(string str);
string ReadFromPipe(int size);
void ErrorExit(PTSTR);

int _tmain(int argc, TCHAR *argv[])
{
	SECURITY_ATTRIBUTES saAttr;

	printf("\n->Start of parent execution.\n");

	// Set the bInheritHandle flag so pipe handles are inherited. 

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	// Create a pipe for the child process's STDOUT. 

	if (!CreatePipe(&g_hChildStd_OUT_Rd, &g_hChildStd_OUT_Wr, &saAttr, 0))
		ErrorExit(TEXT("StdoutRd CreatePipe"));

	// Ensure the read handle to the pipe for STDOUT is not inherited.

	if (!SetHandleInformation(g_hChildStd_OUT_Rd, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdout SetHandleInformation"));

	// Create a pipe for the child process's STDIN. 

	if (!CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0))
		ErrorExit(TEXT("Stdin CreatePipe"));

	// Ensure the write handle to the pipe for STDIN is not inherited. 

	if (!SetHandleInformation(g_hChildStd_IN_Wr, HANDLE_FLAG_INHERIT, 0))
		ErrorExit(TEXT("Stdin SetHandleInformation"));

	// Create the child process. 
	string dir1;
	cout << "Directory 1: ";
	getline(cin, dir1);

	CreateChildProcess(dir1);

	// Get a handle to an input file for the parent. 
	// This example assumes a plain text file and uses string output to verify data flow. 
	/*
	if (argc == 1)
		ErrorExit(TEXT("Please specify an input file.\n"));

	g_hInputFile = CreateFile(
		argv[1],
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_READONLY,
		NULL);

	if (g_hInputFile == INVALID_HANDLE_VALUE)
		ErrorExit(TEXT("CreateFile"));
		*/

	// Write to the pipe that is the standard input for a child process. 
	// Data is written to the pipe's buffers, so it is not necessary to wait
	// until the child process is running before writing data.
	int i = 0;
	/*
	WriteToPipe("settings timebank 10000\n");
	WriteToPipe("settings time_per_move 500\n");
	WriteToPipe("settings player_names player1,player2\n");
	WriteToPipe("settings your_bot player1\n");
	WriteToPipe("settings your_botid 1\n");

	WriteToPipe("update game round 1\n");
	WriteToPipe("update game move 1\n");
	WriteToPipe("update game field 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0\n");
	WriteToPipe("update game macroboard -1,-1,-1,-1,-1,-1,-1,-1,-1\n");
	*/
	WriteToPipe("action move 10000\n");
	while (1)
	{
		//string str = "cba\n";
		//WriteToPipe(str);
		//CreatePipe(&g_hChildStd_IN_Rd, &g_hChildStd_IN_Wr, &saAttr, 0);
		string result = ReadFromPipe(1);
		printf("Program returned: %s\n", result.c_str());
	}
	
	

	// Read from pipe that is the standard output for child process. 

	

	

	// The remaining open handles are cleaned up when this process terminates. 
	// To avoid resource leaks in a larger application, close handles explicitly. 
	while (1);
	return 0;
}

void CreateChildProcess(string dir)
// Create a child process that uses the previously created pipes for STDIN and STDOUT.
{
	TCHAR* szCmdline = new TCHAR[dir.length() + 1];
	copy(dir.begin(), dir.end(), szCmdline);
	szCmdline[dir.length()] = 0;
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	BOOL bSuccess = FALSE;

	// Set up members of the PROCESS_INFORMATION structure. 

	ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

	// Set up members of the STARTUPINFO structure. 
	// This structure specifies the STDIN and STDOUT handles for redirection.

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.hStdError = g_hChildStd_OUT_Wr;
	siStartInfo.hStdOutput = g_hChildStd_OUT_Wr;
	siStartInfo.hStdInput = g_hChildStd_IN_Rd;
	siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

	// Create the child process. 

	bSuccess = CreateProcess(NULL,
		szCmdline,     // command line 
		NULL,          // process security attributes 
		NULL,          // primary thread security attributes 
		TRUE,          // handles are inherited 
		0,             // creation flags 
		NULL,          // use parent's environment 
		NULL,          // use parent's current directory 
		&siStartInfo,  // STARTUPINFO pointer 
		&piProcInfo);  // receives PROCESS_INFORMATION 

					   // If an error occurs, exit the application. 
	if (!bSuccess)
		ErrorExit(TEXT("CreateProcess"));
	else
	{
		// Close handles to the child process and its primary thread.
		// Some applications might keep these handles to monitor the status
		// of the child process, for example. 

		CloseHandle(piProcInfo.hProcess);
		CloseHandle(piProcInfo.hThread);
	}
	delete szCmdline;
}

void WriteToPipe(string str)

// Read from a file and write its contents to the pipe for the child's STDIN.
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;

	strcpy(chBuf, str.c_str());

	WriteFile(g_hChildStd_IN_Wr, chBuf, sizeof(str)+1, &dwWritten, NULL);

	// Close the pipe handle so the child process stops reading. 
	//CloseHandle(g_hChildStd_IN_Wr);

//	if (!CloseHandle(g_hChildStd_IN_Wr))
//		ErrorExit(TEXT("StdInWr CloseHandle"));
}

string ReadFromPipe(int size)

// Read output from the child process's pipe for STDOUT
// and write to the parent process's pipe for STDOUT. 
// Stop when there is no more data. 
{
	DWORD dwRead, dwWritten;
	CHAR chBuf[BUFSIZE];
	BOOL bSuccess = FALSE;
	HANDLE hParentStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

	//ReadFile(g_hChildStd_OUT_Rd, chBuf, size, &dwRead, NULL);
	int index = 0;
	for (;;)
	{
		char buf[1];
		bSuccess = ReadFile(g_hChildStd_OUT_Rd, buf, 1, &dwRead, NULL);
		chBuf[index++] = *buf;
		if (!bSuccess || dwRead == 0 || (*buf) == '\n' || (*buf) == '\r')
		{
			ReadFile(g_hChildStd_OUT_Rd, buf, 1, &dwRead, NULL);
			break;
		}
	}
	chBuf[index] = 0;
	string str(chBuf);
	return str;
}

void ErrorExit(PTSTR lpszFunction)

// Format a readable error message, display a message box, 
// and exit from the application.
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40)*sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(1);
}

