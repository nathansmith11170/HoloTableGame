module;
#include <charconv>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <vector>
export module game:config;

namespace game::details
{
auto config_candidates(std::string_view filename = "options.toml") -> std::vector<std::filesystem::path>
{
	std::vector<std::filesystem::path> out;

	if (const char* local = std::getenv("LOCALAPPDATA"); local && *local)
		out.emplace_back(std::filesystem::path(local) / "HoloTable" / filename);
	if (const char* userprofile = std::getenv("USERPROFILE"); userprofile && *userprofile)
		out.emplace_back(std::filesystem::path(userprofile) / "AppData" / "Local" / "HoloTable" / filename);
	if (const char* xdg = std::getenv("XDG_DATA_HOME"); xdg && *xdg)
		out.emplace_back(std::filesystem::path(xdg) / "HoloTable" / filename);
	if (const char* home = std::getenv("HOME"); home && *home)
		out.emplace_back(std::filesystem::path(home) / ".local" / "share" / "HoloTable" / filename);

	out.emplace_back("assets/options.toml");

	return out;
}

auto trim_str(std::string s) -> std::string
{
	while (!s.empty() && std::isspace((unsigned char)s.front()))
		s.erase(s.begin());
	while (!s.empty() && std::isspace((unsigned char)s.back()))
		s.pop_back();
	return s;
}
} // namespace game::details

export namespace game::config
{
auto find_config(std::string_view filename = "options.toml") -> std::filesystem::path
{
	for (auto& p : details::config_candidates(filename))
	{
		std::error_code ec;
		bool exists = std::filesystem::exists(p, ec);
		if (!ec && exists)
			return p;
	}
	return {""};
}

auto parse_display_size(const std::filesystem::path& p) -> std::pair<int, int>
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
		line = details::trim_str(line);
		if (line.empty())
			continue;
		if (line.front() == '[' && line.back() == ']')
		{
			auto sec = details::trim_str(line.substr(1, line.size() - 2));
			in_display = (sec == "Display");
			continue;
		}
		if (!in_display)
			continue;

		auto eq = line.find('=');
		if (eq == std::string::npos)
			continue;

		auto key = details::trim_str(line.substr(0, eq));
		auto val = details::trim_str(line.substr(eq + 1));

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
} // namespace game::config
