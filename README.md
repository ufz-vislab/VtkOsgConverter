ParaView OpenSG-Exporter
========================

Compile against ParaView 3.8.1 development install for Windows x64
------------------------------------------------------------------

- Make sure you have Visual Studio 2008 with x64 compilers installed
- Install [ParaView 3.8.1 development][pv_dev]
- Build Qt 4.6.2 in `C:\qt\qt-4.6.2-x64`
- Install [Python 2.6][python] in `C:\Python\Python26-x64`
- Install [Python 2.5][python25] for OpenSGs Scons-build system
- Build OpenSG from the Visual Studio x64 prompt
 - Insert in file Sconstruct in line 848:

			env['ENV']['TMP'] = os.environ.get('TMP')

 - Run on the prompt:

			scons.bat compiler=msvc90x64 qt4=no glut=no gif=no tif=yes jpg=yes png=yes jasper=no exr=no zlib=yes

- Configure ParaView-Plugin like this:

		-DOpenSG_LIBRARY_DIRS:FILEPATH="F:/libs/opensg/Build/win32-msvc90x64/installed/lib" -DOpenSG_INCLUDE_DIR:PATH="F:/libs/opensg/Build/win32-msvc90x64/installed/include" -DOPENSG_LIBRARY_DIR:STRING="F:/libs/opensg/Build/win32-msvc90x64/installed/lib" -DQT_QMAKE_EXECUTABLE:FILEPATH="C:/qt/qt-4.6.2-x64-msvc90/bin/qmake.exe" -DParaView_DIR:PATH="C:/ParaView-3.8.1-x64-dev"

- Remove the following line from *OpenSG_Exporter properties / Linker / Input / Additional Dependencies*:

		"C:\Program Files (x86)\Microsoft DirectX SDK (February 2010)\Lib\x64\d3d9.lib"

- Build the plugin and run the `INSTALL` target
- Set the plugin in ParaView to auto-load

You can now export the entire visible scene with *File / Export / .osb*.


[pv_dev]:http://paraview.org/files/v3.8/ParaView-Development-3.8.1-Win64-x86.exe
[python]:http://www.python.org/ftp/python/2.6.6/python-2.6.6.amd64.msi
[python25]:http://www.python.org/ftp/python/2.5.4/python-2.5.4.msi