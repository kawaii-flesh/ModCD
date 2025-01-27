#include "localization.hpp"

#include <nlohmann/json.hpp>
#include <regex>
#include <sstream>
#include <string>
#include <utils/utils.hpp>

namespace {
constexpr std::string_view keyError = "Key not found: ";
constexpr std::string_view valueTypeError = "Value at ";
}  // namespace

namespace utils {

nlohmann::json Localization::texts;

void Localization::load(const std::filesystem::path& filePath) {
    try {
        texts = nlohmann::json::parse(readFileOldWay(filePath));
    } catch (const std::exception& e) {
        /// Log
    }
}

std::string Localization::get(const std::string& keyPath) {
    const nlohmann::json* current = &texts;
    std::istringstream keyStream(keyPath);
    std::string key;

    while (std::getline(keyStream, key, '.')) {
        if (!current->contains(key)) {
            return keyError.data() + keyPath;
        }

        current = &(*current)[key];
    }

    if (current->is_string()) {
        return current->get<std::string>();
    }

    return valueTypeError.data() + keyPath + " is not a string";
}

std::string Localization::getInterpolated(const std::string& keyPath,
                                          const std::unordered_map<std::string, std::string>& variables) {
    std::string templateString = get(keyPath);

    if (utils::startsWith(templateString, keyError) || utils::startsWith(templateString, valueTypeError)) {
        return templateString;
    }

    std::regex placeholderRegex("\\{(\\w+)\\}");
    std::smatch match;

    while (std::regex_search(templateString, match, placeholderRegex)) {
        auto it = variables.find(match[1]);
        templateString.replace(match.position(0), match.length(0), (it != variables.end()) ? it->second : "");
    }

    return templateString;
}

}  // namespace utils
