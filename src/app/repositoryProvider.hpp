#pragma once

#include <core/repository.hpp>
namespace app {

class RepositoryProvider {
   public:
    virtual std::unique_ptr<core::Repository> getRepository() = 0;
    virtual ~RepositoryProvider() = default;
};

};  // namespace app
