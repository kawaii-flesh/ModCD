#pragma once

#include <filesystem>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace app {
const std::string rulesFileName = "mcds";
class MCDS {
   private:
    std::map<std::string, bool (MCDS::*)(const std::vector<std::filesystem::path> &) const> processors = {
        {"unzip", &MCDS::unzip}, {"cp", &MCDS::cp}, {"mv", &MCDS::mv}, {"srm", &MCDS::srm}, {"rm", &MCDS::rm}};

    bool unzip(const std::vector<std::filesystem::path> &params) const;
    bool cp(const std::vector<std::filesystem::path> &params) const;
    bool mv(const std::vector<std::filesystem::path> &params) const;
    bool srm(const std::vector<std::filesystem::path> &params) const;
    bool rm(const std::vector<std::filesystem::path> &params) const;

    std::filesystem::path workingDirectory;

    void executeCommand(const std::string &line) const;
    std::filesystem::path getRelativePath(const std::filesystem::path &path) const;
    std::list<std::filesystem::path> getLastArgs(const std::string &rule) const;

   public:

    void executeRule(const std::string &rule) const;
    bool isModInstalledByUninstallPaths() const;
    void setWorkingDirectory(std::filesystem::path &&aWorkingDirectory) noexcept;
};

}  // namespace app
