#pragma once

#include <borealis.hpp>
#include <filesystem>
#include <front/view/baView.hpp>

namespace front {
class BigPictureActivity : public brls::Activity {
   private:
    const std::list<std::filesystem::path> &imagesPath;
    brls::Box *root;
    std::list<std::filesystem::path>::const_iterator baseIndex;

    void prepare(const std::filesystem::path &path);

   public:
    BigPictureActivity(const std::list<std::filesystem::path> &aImagesPath, int index);

    ~BigPictureActivity();

    brls::View *createContentView() override;
    void onContentAvailable() override;
};
}  // namespace front
