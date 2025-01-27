#include <switch.h>

#include <front/app.hpp>
#include <fslib.hpp>

int main(int argc, char *argv[]) {
    nsInitialize();
    nifmInitialize(NifmServiceType_User);
    fslib::initialize();
    try {
        front::App app(argc, argv);
        app.start();
    } catch (...) {
    }

    nsExit();
    nifmExit();
    fslib::exit();

    return 0;
}
