#include "repositoryProviderOnline.hpp"

#include <nlohmann/json.hpp>
#include <utils/utils.hpp>

namespace app {
RepositoryProviderOnline::RepositoryProviderOnline(Config &aConfig, utils::HttpRequester &aHttpRequester) noexcept
    : config(aConfig), httpRequester(aHttpRequester) {}

std::unique_ptr<core::Repository> RepositoryProviderOnline::getRepository() {
    try {
        MODCD_LOG_DEBUG("[{}]: start", __PRETTY_FUNCTION__);
        const std::string repositoryJSON = this->httpRequester.getText(this->config.repositoryUrl);
        MODCD_LOG_DEBUG("[{}]: the repository has been received - url: {}", __PRETTY_FUNCTION__,
                        this->config.repositoryUrl);
        return core::Repository::fromJson(nlohmann::json::parse(repositoryJSON));
    } catch (const utils::CURLException &curlException) {
        throw RepositoryAccessException();
    }
}
};  // namespace app
