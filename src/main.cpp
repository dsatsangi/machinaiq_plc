#include "app/Application.h"

int main(int argc, char* argv[]) {
#ifdef Q_OS_LINUX
    // On Pi: render directly to framebuffer, no X11 required
    qputenv("QT_QPA_PLATFORM", "linuxfb:fb=/dev/fb0");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "/dev/input/event0");
#endif

    Application app(argc, argv);
    return app.run();
}
