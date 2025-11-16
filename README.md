

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


# For python backend

run these commands to start working
```
python -m venv venv
pip install requirements.txt
```


to create python build exe file
- Install OCR tesseract and setup path in env file
- Download UPX for compress pyinstaller exe file
```
auto-py-to-exe
```
- select the path of `main.py`
- select advance settings > upx path
- Connnect the build exe with frontend

- Direct pyinstaller command
```
pyinstaller --noconfirm --onefile --console --upx-dir "D:\Pulkit\PycharmProjects\Fiverr\upx-5.0.2-win64"  "D:\Pulkit\parsemind-studio\backend_py\app\main.py"
```

