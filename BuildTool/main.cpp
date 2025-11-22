#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>
#include <string>
#include <unordered_map>
#include <unordered_set>

enum struct FullImageType : bool
{
	Host = true,
	Installer = false
};

struct Configuration // NOLINT(bugprone-exception-escape)
{
	FullImageType imageType{};
	std::filesystem::path outputDir{};
	std::filesystem::path buildDir{};
	std::unordered_map<std::string, std::vector<std::pair<std::filesystem::path, std::filesystem::path>>> sourcesByCategory;
};

static std::string ToUpper(std::string string)
{
	std::ranges::transform(string, string.begin(), [](unsigned char character) { return std::toupper(character); });
	return string;
}

FullImageType ParseImageType(std::string arg)
{
	arg = ToUpper(arg);
	if (arg == "/INSTALLER") return FullImageType::Installer;
	if (arg == "/SYSTEM") return FullImageType::Host;
	throw std::runtime_error("Invalid image type argument: " + arg);
}

std::filesystem::path DefaultOutputDir(FullImageType type)
{
	auto cwd = std::filesystem::current_path();
	return cwd / "image" / ((type == FullImageType::Installer) ? "Installer" : "System");
}

void ParseSourceLine(const std::string& line, std::string& outSource, std::string& outAlias)
{
	auto pos = line.find(" as ");
	if (pos != std::string::npos)
	{
		outSource = line.substr(0, pos);
		outAlias = line.substr(pos + 4);
	}
	else
	{
		outSource = line;
		outAlias.clear();
	}
}

void CollectSourcesFromDirectory(std::filesystem::path& dir, std::string& filePattern, const std::string& alias,
						   std::filesystem::path& baseDir,
						   std::vector<std::pair<std::filesystem::path, std::filesystem::path>>& outSources,
						   std::unordered_set<std::string>& seenAliases)
{
	for (const auto& entry : std::filesystem::recursive_directory_iterator(dir))
	{
		if (!entry.is_regular_file()) continue;

		std::string filename = ToUpper(entry.path().filename().string());
		auto patternNoStar = filePattern;
		patternNoStar.erase(std::ranges::remove(patternNoStar, '*').begin(), patternNoStar.end());

		if (filename.find(patternNoStar) != std::string::npos)
		{
			std::filesystem::path aliasPath = alias.empty() ? relative(entry.path(), baseDir) : std::filesystem::path(alias);
			if (aliasPath.filename().string().find('*') != std::string::npos)
			{
				auto aliasStr = aliasPath.string();
				auto posStar = aliasStr.find('*');

				aliasStr.replace(posStar, 1, entry.path().filename().string());
				while (aliasStr.starts_with('\\')) aliasStr = aliasStr.substr(1);
				aliasPath = aliasStr;
			}

			outSources.emplace_back(entry.path(), aliasPath);
			seenAliases.insert(ToUpper(aliasPath.string()));
		}
	}
}

void CollectSources(std::filesystem::path baseDir, const std::vector<std::pair<std::string, std::string>>& patternsWithAlias,
				std::vector<std::pair<std::filesystem::path, std::filesystem::path>>& outSources,
				std::unordered_set<std::string>& seenAliases)
{
	for (const auto& [pattern, alias] : patternsWithAlias)
	{
		std::filesystem::path fullPat = baseDir / pattern;
		std::string targetAlias = alias.empty() ? pattern : alias;
		targetAlias = ToUpper(targetAlias); // normalize

		if (seenAliases.contains(targetAlias)) continue;

		if (pattern.find('*') != std::string::npos)
		{
			auto dir = fullPat.parent_path();
			std::string filePattern = ToUpper(fullPat.filename().string());

			if (!std::filesystem::exists(dir)) continue;

			CollectSourcesFromDirectory(dir, filePattern, alias, baseDir, outSources, seenAliases);
		}
		else
		{
			if (std::filesystem::exists(fullPat))
			{
				std::filesystem::path aliasPath = alias.empty() ? fullPat : std::filesystem::path(alias);
				outSources.emplace_back(fullPat, aliasPath);
				seenAliases.insert(ToUpper(aliasPath.string()));
			}
		}
	}
}

Configuration ParseArguments(int argc, char** argv)
{
	if (argc < 2) throw std::runtime_error("Missing required image type argument (/INSTALLER or /SYSTEM)");

	Configuration config{};
	config.imageType = ParseImageType(argv[1]);
	config.buildDir = std::filesystem::current_path();

	for (int i = 2; i < argc; ++i)
	{
		std::string arg = argv[i];
		if (arg.starts_with("/BUILD:")) config.buildDir = arg.substr(sizeof("/BUILD:") - 1);
		else
			config.outputDir = arg;
	}

	if (config.outputDir.empty()) config.outputDir = DefaultOutputDir(config.imageType);

	std::filesystem::path sourcesList = config.buildDir / "sources.list";
	if (!std::filesystem::exists(sourcesList)) return config;

	std::ifstream file(sourcesList);
	std::string currentCategory;
	std::vector<std::pair<std::string, std::string>> patternsWithAlias;
	std::unordered_set<std::string> seenAliases;

	for (std::string line; std::getline(file, line);)
	{
		if (line.empty() || line[0] == '#') continue;

		if (line.starts_with("- "))
		{
			if (!currentCategory.empty() && !patternsWithAlias.empty())
			{
				CollectSources(config.buildDir, patternsWithAlias, config.sourcesByCategory[currentCategory], seenAliases);
				patternsWithAlias.clear();
			}
			currentCategory = line.substr(2);
		}
		else
		{
			std::string src;
			std::string alias;
			ParseSourceLine(line, src, alias);
			patternsWithAlias.emplace_back(src, alias);
		}
	}

	if (!currentCategory.empty() && !patternsWithAlias.empty())
	{
		CollectSources(config.buildDir, patternsWithAlias, config.sourcesByCategory[currentCategory], seenAliases);
	}

	return config;
}

int main(int argc, char** argv) // NOLINT(bugprone-exception-escape)
{
	try
	{
		auto config = ParseArguments(argc, argv);

		std::println("Building the system image...");
		std::println("Image type: {}", (config.imageType == FullImageType::Installer ? "Installer" : "System"));
		std::println("Output directory: {}", config.outputDir.string());
		std::println("Build directory: {}", config.buildDir.string());

		for (const auto& entry : std::filesystem::directory_iterator(config.outputDir)) std::filesystem::remove_all(entry);

		std::filesystem::create_directories(config.outputDir);
		std::println("Collected sources:");
		for (const auto& [category, sources] : config.sourcesByCategory)
		{
			std::println("  {}: {} {}:", category, sources.size(), sources.size() == 1 ? "source" : "sources");
			for (const auto& [real, alias] : sources)
			{
				auto relReal = std::filesystem::relative(real, config.buildDir);
				auto relAlias = alias;
				std::println("    \x1b[31m\\{}\x1b[0m => \x1b[35m\\{}\x1b[0m", relReal.string(), relAlias.string());
				std::filesystem::create_directories((config.outputDir / alias).parent_path());
				std::filesystem::copy_file(real, config.outputDir / alias);
			}
		}

		std::system("\
qemu-system-x86_64 \
-bios OVMF.fd \
-drive file=fat:rw:G:,format=raw,if=none,id=drive0 \
-device virtio-blk-pci,drive=drive0,serial=purr \
-boot d \
-net none \
-no-reboot \
-no-shutdown \
-m 2G \
-machine q35,hpet=on \
-cpu max,migratable=no,monitor=on \
-overcommit cpu-pm=on \
-smp 1 \
-rtc base=utc,clock=rt \
-serial stdio\
");
	}
	catch (const std::exception& e)
	{
		std::println("Error: {}", e.what());
		return 1;
	}

	return 0;
}
