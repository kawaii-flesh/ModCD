#pragma once

#include <filesystem>
#include <nlohmann/json_fwd.hpp>
#include <string>
#include <unordered_map>

namespace utils {

class Localization {
   private:
    static nlohmann::json texts;

   public:
    static void load(const std::filesystem::path& filePath);
    static std::string get(const std::string& keyPath);
    static std::string getInterpolated(const std::string& keyPath,
                                       const std::unordered_map<std::string, std::string>& variables);
};

}  // namespace utils
