#pragma once

#include <app/config.hpp>
#include <app/modCD.hpp>
#include <app/repositoryProvider.hpp>

namespace app {
class RepositoryProviderOffline : public RepositoryProvider {
   private:
    app::ModCD &modCD;

   public:
    RepositoryProviderOffline(app::ModCD &aModCD) noexcept;

    virtual ~RepositoryProviderOffline() = default;

    std::unique_ptr<core::Repository> getRepository() override;
};
};  // namespace app
