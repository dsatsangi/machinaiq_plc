cmake_minimum_required(VERSION 3.22)

project(machinaiq_uplc
    VERSION 1.0.0
    DESCRIPTION "MachinaIQ UPLC Industrial HMI"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

# Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Network
    SerialPort
)

qt_standard_project_setup()

set(SOURCES
    src/main.cpp
    src/app/Application.cpp
    src/ui/MainWindow.cpp
    src/ui/HomeScreen.cpp
    src/ui/SettingsScreen.cpp
    src/ui/widgets/StatusBanner.cpp
    src/ui/widgets/ScrewCountWidget.cpp
    src/ui/widgets/ChannelSelector.cpp
    src/ui/widgets/StatBox.cpp
    src/comms/CommManager.cpp
    src/comms/SerialHandler.cpp
    src/comms/TcpHandler.cpp
    src/state/StateManager.cpp
    src/config/ConfigManager.cpp
)

set(HEADERS
    src/app/Application.h
    src/ui/MainWindow.h
    src/ui/HomeScreen.h
    src/ui/SettingsScreen.h
    src/ui/widgets/StatusBanner.h
    src/ui/widgets/ScrewCountWidget.h
    src/ui/widgets/ChannelSelector.h
    src/ui/widgets/StatBox.h
    src/comms/CommManager.h
    src/comms/SerialHandler.h
    src/comms/TcpHandler.h
    src/state/StateManager.h
    src/state/AppState.h
    src/config/ConfigManager.h
    src/config/models/HierarchyModels.h
)

qt_add_executable(machinaiq_uplc
    ${SOURCES}
    ${HEADERS}
    resources/resources.qrc
)

target_include_directories(machinaiq_uplc PRIVATE src)

target_link_libraries(machinaiq_uplc PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::SerialPort
)

# Pi Zero optimisation flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(machinaiq_uplc PRIVATE -O2 -pipe)
endif()

# ── Installation ──────────────────────────────────────────────────────────────
include(GNUInstallDirs)

install(TARGETS machinaiq_uplc
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES config/default_config.json
    DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/machinaiq_uplc
    RENAME config.json
)

install(FILES systemd/machinaiq-uplc.service
    DESTINATION lib/systemd/system
)

# ── CPack / .deb ─────────────────────────────────────────────────────────────
include(cmake/packaging.cmake)
