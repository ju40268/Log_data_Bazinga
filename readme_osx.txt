This document references a Google document located at:
https://docs.google.com/a/logitech.com/document/d/1wELOZB3jodt_mhpQeT36bZX-j-Nq6fIy2Xn_7Pl0S98/edit


Dynamic libraries
================================================

- Pre-built dynamic libraries for Qt 5.0 can be downloaded from http://qt-project.org/downloads. The file to download is “Qt 5.1.1 for Mac (428 MB)”.
- By default, the pre-built version of Qt for Mac OS X installs in ~/Qt5.1.1.
- The OS X Qt installer will hang if target location happens to be in /usr/local. Please install in ~/Qt5.1.1
- After installing Qt with dynamic libraries, it is not possible to move the Qt directory, since Qt hard codes library paths inside libraries and executables.
- Qt has a script that inserts the dynamic libraries into an application bundle. Pre 4.6 it uses to be a hassle to include Qt dynamic libraries into a Mac application bundle, but Qt now does it for you.
- With dynamic libraries the application bundle will have a bigger size. For an internal tool it doesn’t matter.

Building Bazinga
================================================

- To generate an XCode project, you can use:
   
    qmake -spec macx-xcode project.pro

- I created an XCode project for Bazinga using the following command line:
    ~/Qt5.1.1/5.1.1/clang_64/bin/qmake -spec macx-xcode Bazinga.pro

- Problem with this approach is that any new file added to Bazinga will require either
    o Adding the file manually to the XCode project (preferred option for small number of files)
    o Regenerating the XCode project with qmake -spec, and applying again all the settings below (preferred option if a lot of files were added or deleted)


XCode Settings
================================================

- There are many places where path to Qt is hard coded (eg. /Users/Test/ …). It would be good to replace with ~/

- OS X deployment target has to be set to 10.7 at least (otherwise, getting compile errors)

- C++ language dialect has to be set to C++ 11 [-std=c++11]

- C++ standard library has to be set to libc++ (LLVM C++ standard library with C++11 support)

- To bundle the .app with the dynamic libraries, use:

  (from the built app bundle directory)
 ~/Qt5.1.1/5.1.1/clang_64/bin/macdeployqt Bazinga.app



