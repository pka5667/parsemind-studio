cmake_minimum_required(VERSION 3.16)

# Qt requirements
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Core)

# Python embedding
find_package(Python3 COMPONENTS Development REQUIRED)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Your executable
qt_add_executable(ParseMindShell
    main.cpp
    mainwindow.cpp
    PythonBackend.cpp
    mainwindow.h
    mainwindow.ui
)

# Link Qt + Python
target_link_libraries(ParseMindShell PRIVATE
    Qt6::Widgets
    Qt6::Network
    Qt6::Core
    Python3::Python
)

# Python include path
target_include_directories(ParseMindShell PRIVATE
    ${Python3_INCLUDE_DIRS}
)

set_target_properties(ParseMindShell PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# --- COPY NUITKA MODULE AFTER BUILD ---

# Folder where Nuitka outputs your compiled pyd
set(PY_BACKEND_DIR "${CMAKE_SOURCE_DIR}/embedded_py")

# Grab *.pyd file
file(GLOB PY_BACKEND_MODULE "${PY_BACKEND_DIR}/*.pyd")

if(NOT PY_BACKEND_MODULE)
    message(WARNING "No Nuitka .pyd module found in ${PY_BACKEND_DIR}.")
endif()

# Copy .pyd next to exectuable at build time
add_custom_command(
    TARGET ParseMindShell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${PY_BACKEND_MODULE}
            $<TARGET_FILE_DIR:ParseMindShell>
    COMMENT "Copying Nuitka module into output directory..."
)
