#pragma once

#include <app/modCD.hpp>

namespace front {
class App {
   private:
    app::ModCD modCD;

   public:
    App(int argc, char *argv[]);
    ~App() {}
    void start();
};
}  // namespace front
