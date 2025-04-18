find_package(Qt6 COMPONENTS Quick QuickControls2 DBus REQUIRED)
find_package(Waylib REQUIRED Server)

qt_standard_project_setup(REQUIRES 6.7)

if(QT_KNOWN_POLICY_QTP0001)
    qt_policy(SET QTP0001 NEW)
    # the RESOURCE_PREFIX argument for qt_add_qml_module() defaults to ":/qt/qml/"
endif()
if(POLICY CMP0071)
    # https://cmake.org/cmake/help/latest/policy/CMP0071.html
    cmake_policy(SET CMP0071 NEW)
endif()

set(QML_IMPORT_PATH ${CMAKE_BINARY_DIR}/src/server/ CACHE STRING "" FORCE)

find_package(PkgConfig REQUIRED)
pkg_search_module(PIXMAN REQUIRED IMPORTED_TARGET pixman-1)
pkg_search_module(WAYLAND REQUIRED IMPORTED_TARGET wayland-server)

set(TARGET waygreet)

add_executable(${TARGET}
    main.cpp
)

qt_add_qml_module(${TARGET}
    URI WayGreet
    VERSION "1.0"

    SOURCES
        helper.h helper.cpp
        output.h output.cpp
        qmlengine.h qmlengine.cpp
        rootcontainer.h rootcontainer.cpp
        ipc.h ipc.cpp
        sessionipc.h sessionipc.cpp
        powermanager.h powermanager.cpp
        session.h session.cpp
        sessionmodel.h sessionmodel.cpp
        usermodel.h usermodel.cpp
        wayconfig.h wayconfig.cpp
    QML_FILES PrimaryOutput.qml
    QML_FILES CopyOutput.qml
    QML_FILES OutputMenuBar.qml
    QML_FILES Greeter.qml SimpleButton.qml SimpleComboBox.qml
)

target_compile_definitions(${TARGET}
    PRIVATE
    SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
    PROJECT_BINARY_DIR="${PROJECT_BINARY_DIR}"
    $<$<BOOL:${START_DEMO}>:START_DEMO>
)

target_link_libraries(${TARGET}
    PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::DBus
    Waylib::WaylibServer
    PkgConfig::PIXMAN
    PkgConfig::WAYLAND
)

install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})
