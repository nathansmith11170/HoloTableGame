#define RGFW_IMPORT
#include "RGFW.h"

import std;
import game;

static u8 icon[4 * 3 * 3] = {
		0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

int main()
{
	RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(100, 100, 500, 500),
																			 static_cast<u64>(0));

	RGFW_window_setIcon(win, icon, RGFW_AREA(3, 3), 4);

	while (RGFW_window_shouldClose(win) == RGFW_FALSE)
	{
		while (RGFW_window_checkEvent(win))
		{
			if (win->event.type == RGFW_quit || RGFW_isPressed(win, RGFW_escape))
				break;
		}

		RGFW_window_swapBuffers(win);
	}

	RGFW_window_close(win);
}
