#include "RGFW.h"

#include <array>
#include <print>

import game;

static std::array<unsigned char, static_cast<size_t>(4 * 3 * 3)> icon = {
		0xFF,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0xFF,
		0xFF,
		0xFF,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0xFF,
		0xFF,
		0x00,
		0x00,
		0xFF,
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) -> int
{
	auto cfg = game::config::find_config();
	std::println("Using config: {}", cfg.string());

	auto [width, height] = game::config::parse_display_size(cfg);

	RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(100, 100, width, height), static_cast<u64>(0));

	RGFW_window_setIcon(win, icon.data(), RGFW_AREA(3, 3), 4);

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
