#include "RGFW.h"

#include <array>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <optional>
#include <print>
#include <vector>

import game;

static std::array<unsigned char, static_cast<size_t>(4 * 3 * 3)> icon = {
		0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF,
		0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xFF};

namespace fs = std::filesystem;

static auto exists_noexcept(const std::optional<fs::path>& p) noexcept -> bool
{
	if (!p.has_value())
		return false;
	std::error_code ec;
	bool exists = fs::exists(p.value(), ec);
	(void)ec; // silence unused in rare builds; we only rely on non-throwing check
	return exists;
}

static auto config_candidates(std::string_view filename = "options.toml")
		-> std::vector<fs::path>
{
	std::vector<fs::path> out;

	if (const char* local = std::getenv("LOCALAPPDATA"); local && *local)
		out.emplace_back(fs::path(local) / "HoloTable" / filename);
	if (const char* userprofile = std::getenv("USERPROFILE");
			userprofile && *userprofile)
		out.emplace_back(fs::path(userprofile) / "AppData" / "Local" / "HoloTable" /
										 filename);
	if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && *xdg)
		out.emplace_back(fs::path(xdg) / "HoloTable" / filename);
	if (const char* home = std::getenv("HOME"); home && *home)
		out.emplace_back(fs::path(home) / ".local" / "share" / "HoloTable" /
										 filename);

	out.emplace_back("assets/options.toml");

	return out;
}

auto find_config(std::string_view filename = "options.toml") noexcept
		-> fs::path
{
	for (auto& p : config_candidates(filename))
	{
		if (exists_noexcept(p))
			return p;
	}
	return {""};
}

// Simple parser for our small options.toml
static auto trim_str(std::string s) -> std::string
{
	while (!s.empty() && std::isspace((unsigned char)s.front()))
		s.erase(s.begin());
	while (!s.empty() && std::isspace((unsigned char)s.back()))
		s.pop_back();
	return s;
}

static auto parse_display_size(const fs::path& p) noexcept
		-> std::pair<int, int>
{
	int width = 800;
	int height = 600;
	if (p.empty())
		return {width, height};

	std::ifstream in(p);
	if (!in)
		return {width, height};

	std::string line;
	bool in_display = false;
	while (std::getline(in, line))
	{
		// strip comments (TOML uses '#')
		auto pos = line.find('#');
		if (pos != std::string::npos)
			line = line.substr(0, pos);
		line = trim_str(line);
		if (line.empty())
			continue;
		if (line.front() == '[' && line.back() == ']')
		{
			auto sec = trim_str(line.substr(1, line.size() - 2));
			in_display = (sec == "Display");
			continue;
		}
		if (!in_display)
			continue;

		auto eq = line.find('=');
		if (eq == std::string::npos)
			continue;

		auto key = trim_str(line.substr(0, eq));
		auto val = trim_str(line.substr(eq + 1));

		if (!val.empty() && (val.front() == '"' || val.front() == '\''))
		{
			if (val.size() >= 2)
				val = val.substr(1, val.size() - 2);
		}

		if (key == "width" || key == "height")
		{
			int parsed_value = 0;
			auto result =
					std::from_chars(val.data(), val.data() + val.size(), parsed_value);

			if (result.ec == std::errc())
			{
				if (key == "width")
					width = parsed_value;
				else
					height = parsed_value;
			}
		}
	}
	return {width, height};
}

auto main() -> int
{
	auto cfg = find_config();
	std::println("Using config: {}", cfg.string());

	auto [width, height] = parse_display_size(cfg);

	RGFW_window* win = RGFW_createWindow(
			"name", RGFW_RECT(100, 100, width, height), static_cast<u64>(0));

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
