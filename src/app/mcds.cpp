#include "mcds.hpp"

#include <app/modCD.hpp>
#include <borealis.hpp>
#include <fstream>
#include <sstream>
#include <utils/constants.hpp>
#include <utils/utils.hpp>

namespace app {

void MCDS::setWorkingDirectory(std::filesystem::path &&aWorkingDirectory) noexcept {
    this->workingDirectory = std::move(aWorkingDirectory);
}

std::filesystem::path MCDS::getRelativePath(const std::filesystem::path &path) const {
    if (path.is_absolute()) {
        return path;
    } else {
        return this->workingDirectory / path;
    }
}

void MCDS::executeRule(const std::string &rule) const {
    MODCD_LOG_DEBUG("[{}]: start rule execution - {}", __PRETTY_FUNCTION__, rule);
    const std::filesystem::path filePath = this->workingDirectory / rulesFileName;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        MODCD_LOG_ERROR("[{}]: there is no rule file - {}", __PRETTY_FUNCTION__, filePath);
        return;
    }
    MODCD_LOG_DEBUG("[{}]: the rules file is open - {}", __PRETTY_FUNCTION__, filePath);

    std::string line;
    while (std::getline(file, line)) {
        if (line == '[' + rule + ']') {
            while (std::getline(file, line) && line != "end") {
                if (line.empty()) {
                    continue;
                }
                if (line[0] == '!') {
                    this->executeRule(line.substr(1));
                } else {
                    this->executeCommand(line);
                }
            }
            break;
        }
    }
    MODCD_LOG_DEBUG("[{}]: the rule has been completed - {}", __PRETTY_FUNCTION__, rule);

    file.close();
    return;
}

void MCDS::executeCommand(const std::string &line) const {
    MODCD_LOG_DEBUG("[{}]: line - {}", __PRETTY_FUNCTION__, line);
    std::istringstream iss(line);
    std::string command;
    std::vector<std::filesystem::path> arguments;
    std::string word;

    if (std::getline(iss, command, modcd_constants::DELIMITER)) {
        while (std::getline(iss, word, modcd_constants::DELIMITER)) {
            arguments.push_back(std::move(word));
        }
    }

    const auto it = this->processors.find(command);
    if (it != this->processors.end()) {
        (this->*(it->second))(arguments);
    } else {
        MODCD_LOG_ERROR("[{}]: the wrong command - {}", __PRETTY_FUNCTION__, line);
    }
}

std::list<std::filesystem::path> MCDS::getLastArgs(const std::string &rule) const {
    std::list<std::filesystem::path> lastArgs;
    const std::filesystem::path filePath = this->workingDirectory / rulesFileName;
    std::ifstream file(filePath);
    if (!file.is_open()) {
        MODCD_LOG_ERROR("[{}]: there is no rule file - {}", __PRETTY_FUNCTION__, filePath);
        return lastArgs;
    }

    std::string line;
    bool isInRule = false;
    while (std::getline(file, line)) {
        if (line == '[' + rule + ']') {
            isInRule = true;
            continue;
        }

        if (isInRule) {
            if (line == "end") {
                break;
            }

            if (line.empty() || line[0] == '!') {
                continue;
            }

            auto pos = line.rfind(modcd_constants::DELIMITER);
            if ((pos != std::string::npos) && (pos + 1 < line.size())) {
                lastArgs.emplace_back(line.substr(pos + 1));
            }
        }
    }

    file.close();
    return lastArgs;
}

bool MCDS::isModInstalledByUninstallPaths() const {
    std::list args = this->getLastArgs("uninstall");
    for (const std::filesystem::path &arg : args) {
        if (!utils::exists(arg)) {
            return false;
        }
    }
    return true;
}

bool MCDS::unzip(const std::vector<std::filesystem::path> &params) const {
    if (params.size() != 2) {
        return false;
    }
    const std::filesystem::path &archPath = params[0];
    const std::filesystem::path dst = this->getRelativePath(params[1]);
    MODCD_LOG_DEBUG("[{}]: archPath: {} | dst: {}", __PRETTY_FUNCTION__, archPath, dst);
    return utils::unzipFile(this->workingDirectory, archPath, dst);
}

bool MCDS::srm(const std::vector<std::filesystem::path> &params) const {
    if (params.size() != 1) {
        return false;
    }
    const std::filesystem::path &path = params[0];
    MODCD_LOG_DEBUG("[{}]: path: {}", __PRETTY_FUNCTION__, path);
    return utils::remove(this->getRelativePath(path));
}

bool MCDS::rm(const std::vector<std::filesystem::path> &params) const {
    if (params.size() != 1) {
        return false;
    }
    const std::filesystem::path &path = params[0];
    MODCD_LOG_DEBUG("[{}]: path: {}", __PRETTY_FUNCTION__, path);
    return utils::removeAndEmpty(this->getRelativePath(path));
}

bool MCDS::cp(const std::vector<std::filesystem::path> &params) const {
    if (params.size() != 2) {
        return false;
    }
    const std::filesystem::path &src = params[0];
    const std::filesystem::path &dst = params[1];
    MODCD_LOG_DEBUG("[{}]: src: {} | dst: {}", __PRETTY_FUNCTION__, src, dst);
    return utils::copy(this->getRelativePath(src), this->getRelativePath(dst));
}

bool MCDS::mv(const std::vector<std::filesystem::path> &params) const {
    if (params.size() != 2) {
        return false;
    }
    const std::filesystem::path &src = params[0];
    const std::filesystem::path &dst = params[1];
    MODCD_LOG_DEBUG("[{}]: src: {} | dst: {}", __PRETTY_FUNCTION__, src, dst);
    return utils::move(this->getRelativePath(src), this->getRelativePath(dst));
}
}  // namespace app
