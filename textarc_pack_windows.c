#define _UNICODE
#define UNICODE

#include <windows.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "textarc.h"

#define PROGNAME L"textarc-pack"

#define COUNTOF(x) (sizeof(x) / sizeof(x[0]))

static struct textarc_entry e;

static void
diesys(const wchar_t *msg)
{
	static wchar_t errbuf[128];

	FormatMessageW(
	        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0,
	        GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
	        errbuf, COUNTOF(errbuf), 0);
	errbuf[COUNTOF(errbuf) - 1] = 0;
	fwprintf(stderr, L"%s: %s: %s", PROGNAME, msg, errbuf);
	exit(1);
}

static char *
utf8_from_wstring(const wchar_t *wstring)
{
	char *utf8;
	int nbyte; /* including null terminator */

	if (!(nbyte = WideCharToMultiByte(
	              CP_UTF8, 0, wstring, -1, 0, 0, 0, 0))) {
		diesys(L"charset conversion error");
	}
	if (!(utf8 = calloc(nbyte, 1))) {
		diesys(L"out of memory");
	}
	if (WideCharToMultiByte(CP_UTF8, 0, wstring, -1, utf8, nbyte, 0, 0) !=
	        nbyte) {
		free(utf8);
		diesys(L"charset conversion error");
	}
	return utf8;
}

static void
unpack_windows_filetime(FILETIME *filetime)
{
	SYSTEMTIME st;

	if (!FileTimeToSystemTime(filetime, &st))
		diesys(L"FileTimeToSystemTime");
	e.year = st.wYear;
	e.month = st.wMonth;
	e.day = st.wDay;
	e.hour = st.wHour;
	e.minute = st.wMinute;
	e.second = st.wSecond;
}

static void
write_windows_entry(const wchar_t *filename)
{
	WIN32_FIND_DATA wfd;
	HANDLE handle;

	memset(&e, 0, sizeof(e));
	if ((handle = FindFirstFile(filename, &wfd)) ==
	        INVALID_HANDLE_VALUE) {
		diesys(L"FindFirstFile");
	}
	e.filename = utf8_from_wstring(filename);
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		e.type = "dir";
	} else {
		e.type = "file";
		unpack_windows_filetime(&wfd.ftLastWriteTime);
	}
	FindClose(handle);
	e.mode = -1;
	e.uid = -1;
	e.gid = -1;
	write_entry(&e);
	free((void *)e.filename);
}

int
wmain(int argc, wchar_t **argv)
{
	size_t i;

	for (i = 1; i < argc; i++) {
		write_windows_entry(argv[i]);
	}
	return 0;
}
