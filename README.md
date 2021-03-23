# Cjing3D
Cjing3D is a very junk C++ engine
####

### Platforms:
- Windows 10 Desktop [Visual Studio 2019]  
- Linux (TODO)

### How to build: 
#### Windows
###### Prerequisites
* Visual Studio 2019 on Windows
* Python 
* Git client

Download the source code and move to the root directory, then call these commands:
```
cd editor
..\build win32 -all
```
it will create the sln in "editor\build\win32\Cjing3D.sln".  

### 3rd party libraries:
 * asio     (optional, for net moduler)
 * bullet   (optional, for physics moduler)
 * catch
 * fcpp
 * imgui
 * json
 * lua
 * nvtt     (optional)
 * optick   (optional)
 * physfs
 * rectPacker
 * remotery (unused)
 * stb
 * tiny
 * tlsf
