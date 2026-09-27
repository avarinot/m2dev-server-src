#pragma once

#include <filesystem>
#include <string>

// Resolves "dir/name.ext" to "dir/name_<language>.ext" when that variant exists,
// so the server language can be switched through configuration (LANGUAGE)
// without replacing the base files. Falls back to the base file otherwise.
inline std::string GetLocalizedFileName(const std::string& fileName, const std::string& language)
{
	if (language.empty())
		return fileName;

	std::filesystem::path localized(fileName);
	localized.replace_filename(localized.stem().string() + "_" + language + localized.extension().string());

	return std::filesystem::exists(localized) ? localized.string() : fileName;
}
