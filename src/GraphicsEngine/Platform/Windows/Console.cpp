

#ifdef PLATFORM_WIN
#include "GraphicsEngine/Console.h"
//
//

void *GEng::Console::cHandle = nullptr;

namespace GEng
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
