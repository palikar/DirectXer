
#include "IncludeWin.h"
#include "Window.h"
#include "App.h"

#include <wchar.h>
#include <string>


int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdSHow)
{

	App application;

	try {
		application.Go();
        
	}
	catch( const BaseException& e )
	{
		MessageBox( nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION );
	}
	catch( const std::exception& e )
	{
		MessageBox( nullptr,e.what(),"Standard Exception",MB_OK | MB_ICONEXCLAMATION );
	}
	catch( ... )
	{
		MessageBox( nullptr,"No details available","Unknown Exception",MB_OK | MB_ICONEXCLAMATION );
	}

}
