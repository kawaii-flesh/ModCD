#pragma once

#include <app/config.hpp>
#include <app/repositoryProvider.hpp>
#include <utils/http.hpp>

namespace app {

class RepositoryAccessException : public std::exception {
   public:
    const char *what() const noexcept override { return "Repository access error"; }
};

class RepositoryProviderOnline : public RepositoryProvider {
   private:
    Config &config;
    utils::HttpRequester &httpRequester;

   public:
    RepositoryProviderOnline(Config &aConfig, utils::HttpRequester &aHttpRequester) noexcept;

    virtual ~RepositoryProviderOnline() = default;

    std::unique_ptr<core::Repository> getRepository() override;
};
};  // namespace app
