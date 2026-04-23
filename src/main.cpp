#include "app/Application.h"
#include <cstdlib>

int main(int argc, char* argv[]) {
#ifdef Q_OS_LINUX
    // Only force framebuffer when no display server is present.
    // If DISPLAY or WAYLAND_DISPLAY is set, Qt will auto-pick xcb/wayland.
    const bool hasDisplayServer =
        std::getenv("DISPLAY") != nullptr ||
        std::getenv("WAYLAND_DISPLAY") != nullptr;

    if (!hasDisplayServer) {
        qputenv("QT_QPA_PLATFORM", "linuxfb:fb=/dev/fb0");
        qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "/dev/input/event0");
    }
#endif

    Application app(argc, argv);
    return app.run();
}
