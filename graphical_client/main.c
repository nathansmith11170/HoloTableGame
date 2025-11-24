#include "RGFW.h"
#include <game.h>
#include <stdio.h>

static unsigned char icon[4 * 3 * 3] = {
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

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[argc + 1])
{
  ConfigurationFile cfg = game_find_config("options.toml");
  printf("Using config %s\n", cfg.filename);
  WindowDimensions win_dimensions = game_parse_display_size(cfg);

  RGFW_window* win = RGFW_createWindow("name", RGFW_RECT(100, 100, win_dimensions.width, win_dimensions.height), 0);

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
