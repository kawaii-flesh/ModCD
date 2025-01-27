#include "gamesListActivity.hpp"

#include <utils/utils.hpp>

namespace front {
GamesListActivity::GamesListActivity(app::ModCD &aModCD) noexcept
    : modCD(aModCD), gamesListView(new GamesListView(this->modCD)) {
    MODCD_LOG_DEBUG("[{}]: created", __PRETTY_FUNCTION__);
}

GamesListActivity::~GamesListActivity() { MODCD_LOG_DEBUG("[{}]: deleted", __PRETTY_FUNCTION__); }

brls::View *GamesListActivity::createContentView() { return this->gamesListView; }
}  // namespace front
