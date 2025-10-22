#pragma once

namespace front {
class IUpdatable {
   public:
    virtual ~IUpdatable() = default;
    virtual void updateUI() = 0;
};
}  // namespace front
