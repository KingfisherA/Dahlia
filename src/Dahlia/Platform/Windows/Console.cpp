

#ifdef PLATFORM_WIN
#include "Dahlia/Console.h"
//
//

void *DH::Console::cHandle = nullptr;

namespace DH
{
	//
	void Console::SetHandle(void *handle)
	{
		cHandle = handle;
	}
	//
	void Console::SetConsoleColour(ConsoleColour col)
	{
		SetConsoleTextAttribute(cHandle, WORD(col));
	}
	//
}
#endif
