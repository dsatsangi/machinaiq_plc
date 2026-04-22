#include "app/Application.h"

int main(int argc, char* argv[]) {
    // Needed for Qt Widgets on framebuffer (Pi without X11)
    qputenv("QT_QPA_PLATFORM", "linuxfb:fb=/dev/fb0");
    qputenv("QT_QPA_EVDEV_TOUCHSCREEN_PARAMETERS", "/dev/input/event0");

    Application app(argc, argv);
    return app.run();
}
