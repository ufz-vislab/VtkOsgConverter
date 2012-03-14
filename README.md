ParaView OpenSG-Exporter
========================

Mac OS Build
--------------

- Install [Homebrew](http://mxcl.github.com/homebrew/)

```shell
/usr/bin/ruby -e "$(/usr/bin/curl -fsSL https://raw.github.com/mxcl/homebrew/master/Library/Contributions/install_homebrew.rb)"
```

- Install Qt and OpenSG

```shell
brew install qt open-sg
```

- Clone ParaView

```shell
git clone --recursive https://github.com/Kitware/ParaView.git
```

- Build ParaView

```shell
mkdir ParaView_Build
cd ParaView_Build
ccmake ../ParaView -DBUILD_SHARED_LIBS=ON -DCMAKE_BUILD_TYPE=Release
make -j
```

- Clone the plugin

```shell
git clone https://bilke@github.com/bilke/pv_opensg_plugin.git
```

- Build the plugin

```shell
mkdir pv_opensg_plugin_build
cd pv_opensg_plugin_build
ccmake ../pv_opensg_plugin -DParaView_DIR:PATH=../ParaView_Build -DCMAKE_BUILD_TYPE=Release
make -j install
```

Windows Build
-------------

### Install with ParaView 3.8.1 for Windows x64 ###

- Install [ParaView 3.8.1][pv]
- Copy the unzipped dlls to  `[Your ParaView install directory]/bin`
- Start ParaView
- Open *Tools / Manage Plugins*
 - On the right side click on *Load New ...*
  - Goto the ParaView bin directory, select *OpenSG_Exporter.dll* and click *Ok*
  - Expand the new OpenSG_Exporter entry by clicking on the plus sign and check *Auto Load*
 - From now on the plugin gets loaded automatically on startup
- Export the entire visible scene through *File / Export / .osb*

### Compile against ParaView 3.8.1 development install for Windows x64 ###

- Make sure you have Visual Studio 2008 with x64 compilers installed
- Install [ParaView 3.8.1 development][pv_dev]
- Build Qt 4.6.2 in `C:\qt\qt-4.6.2-x64`
- Install [Python 2.6][python] in `C:\Python\Python26-x64`
- Install [Python 2.5][python25] in `C:\Python\Python25` for OpenSGs Scons-build system
- Build OpenSG from the Visual Studio x64 prompt
 - Insert in file Sconstruct in line 848:
 ```shell
 env['ENV']['TMP'] = os.environ.get('TMP')
 ```

 - Run on the prompt:
 ```shell
 path=%PATH%;C:\Python\Python25
 scons.bat compiler=msvc90x64 qt4=no glut=no gif=no tif=yes jpg=yes png=yes jasper=no exr=no zlib=no
 ```

- Configure ParaView-Plugin like this:

		-DOpenSG_LIBRARY_DIRS:FILEPATH="F:/libs/opensg/Build/win32-msvc90x64/installed/lib" -DOpenSG_INCLUDE_DIR:PATH="F:/libs/opensg/Build/win32-msvc90x64/installed/include" -DOPENSG_LIBRARY_DIR:STRING="F:/libs/opensg/Build/win32-msvc90x64/installed/lib" -DQT_QMAKE_EXECUTABLE:FILEPATH="C:/qt/qt-4.6.2-x64-msvc90/bin/qmake.exe" -DParaView_DIR:PATH="C:/ParaView-3.8.1-x64-dev"

- Remove the following line from *OpenSG_Exporter properties / Linker / Input / Additional Dependencies*:

		"C:\Program Files (x86)\Microsoft DirectX SDK (February 2010)\Lib\x64\d3d9.lib"

- Build the plugin and run the `INSTALL` target

Usage
-----

- Start ParaView
- Set the plugin in ParaView to auto-load
- You can now export the entire visible scene with *File / Export / .osb*.

[pv]:http://paraview.org/files/v3.8/ParaView-3.8.1-Win64-x86.exe
[pv_dev]:http://paraview.org/files/v3.8/ParaView-Development-3.8.1-Win64-x86.exe
[python]:http://www.python.org/ftp/python/2.6.6/python-2.6.6.amd64.msi
[python25]:http://www.python.org/ftp/python/2.5.4/python-2.5.4.msi