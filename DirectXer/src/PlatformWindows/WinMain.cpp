#include "Platform.hpp"
#include "Memory.hpp"
#include "Resources.hpp"
#include "App.hpp"

#include <shellapi.h>

extern App* InitMain(char** argv, int argc);

static LPSTR* CommandLineToArgvA(LPSTR lpCmdLine, INT* pNumArgs)
{
	int retval;
	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, NULL, 0);
	Assert(SUCCEEDED(retval), "Can't convert arguments to char*");


	LPWSTR lpWideCharStr = (LPWSTR)malloc(retval * sizeof(WCHAR));
	Assert(lpWideCharStr != NULL, "Can't convert arguments to char*");
		

	retval = MultiByteToWideChar(CP_ACP, MB_ERR_INVALID_CHARS, lpCmdLine, -1, lpWideCharStr, retval);
	if (!SUCCEEDED(retval))
	{
		free(lpWideCharStr);
		return NULL;
	}

	int numArgs;
	LPWSTR* args;
	args = CommandLineToArgvW(lpWideCharStr, &numArgs);
	free(lpWideCharStr);
	Assert(args != NULL, "Can't convert arguments to char*");

	int storage = numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, NULL, 0, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(args);
			return NULL;
		}

		storage += retval;
	}

	LPSTR* result = (LPSTR*)LocalAlloc(LMEM_FIXED, storage);
	if (result == NULL)
	{
		LocalFree(args);
		return NULL;
	}

	int bufLen = storage - numArgs * sizeof(LPSTR);
	LPSTR buffer = ((LPSTR)result) + numArgs * sizeof(LPSTR);
	for (int i = 0; i < numArgs; ++i)
	{
		Assert(bufLen > 0, "Can't convert arguments to char*");
		BOOL lpUsedDefaultChar = FALSE;
		retval = WideCharToMultiByte(CP_ACP, 0, args[i], -1, buffer, bufLen, NULL, &lpUsedDefaultChar);
		if (!SUCCEEDED(retval))
		{
			LocalFree(result);
			LocalFree(args);
			return NULL;
		}

		result[i] = buffer;
		buffer += retval;
		bufLen -= retval;
	}

	LocalFree(args);

	*pNumArgs = numArgs;
	return result;

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{
	SetupConsole();
	gDxgiManager.Init();

	int argc;
	char** argv;
	argv = CommandLineToArgvA(GetCommandLine(), &argc);

	WindowsSettings settings{0};
	settings.ClassName = "DirectXClass";
	settings.WindowTitle = "DirectXer";
	settings.InitialWidth = 1080;
	settings.InitialHeight = 720;
	settings.Fullscreen = 0;

	WindowsWindow window;
	window.Application = InitMain(argv, argc);

	size_t InitialStackMemory{0};
	InitialStackMemory += sizeof(WindowsWindow);
	InitialStackMemory += sizeof(WindowsSettings);
	InitialStackMemory += sizeof(int);
	InitialStackMemory += sizeof(char**);
	DXLOG("[Init] Initial Stack Memory: {:.3} kB ", InitialStackMemory/1024.0f);
	
	window.Init(settings);
	return window.Run();
}
