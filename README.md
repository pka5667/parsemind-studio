

Dev Setup
Install QT6 online installer, which install qt creater as well.
Install cmake

In vscode 
    add qt extension pack
    qt core
    qt c++ extension pack
    add cmake extension
    add cmake tools extension

setup cpp compiler for this project to -> C:/QT/_version_/mingw

CmakeLists automatically triggers builds

in vscode launch.json:
    add Debug Qt Application with Visual Studio Debugger

in vscode c_cpp_properties.json:
    include path for C:/QT/_version_/mingw/bin/**, include, lib

Command to make final build: cmake --build build --config Release

