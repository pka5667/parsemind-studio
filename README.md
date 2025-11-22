

# FOR CPP
## Using Cpp for UI

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

Command to make final build: `cmake --build build --config Release`
or
`cmake --build build --target ParseMindShell -j 4`


# For python backend

run these commands to start working
```
python -m venv venv
pip install requirements.txt
```


to create python build exe file
- Install python in system
- Install OCR tesseract and setup path in env file


Use Nauitka to create a cpp module which can be directly embedded in cpp code
```
python -m nuitka --module backend_py/main.py --output-dir=embedded_py --include-package=app --assume-yes-for-downloads
```

Use the `main.*.pyd` file in cpp as a module

In this approach, user will need to have the following installed on their system:
- Python
- Tessorrect
- Ollama


# Download Inno Setup
`https://jrsoftware.org/isdl.php#stable`


# Build process:
- Download Inno Setup
- Build python backend
- Build cpp frontend exe
- Build Inno Setup exe using installer.iss file

