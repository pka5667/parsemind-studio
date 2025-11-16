find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(ParseMindShell
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(ParseMindShell PRIVATE Qt6::Widgets)

# On Windows, mark the target as a WIN32 executable (GUI subsystem) so
# no console window is created alongside the Qt UI. If you need a
# console for debugging, build without this property or run the
# backend manually from a terminal.
set_target_properties(ParseMindShell PROPERTIES WIN32_EXECUTABLE TRUE)
