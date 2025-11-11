find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_add_executable(ParseMindShell
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
)

target_link_libraries(ParseMindShell PRIVATE Qt6::Widgets)
