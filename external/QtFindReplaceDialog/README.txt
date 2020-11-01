QtFindReplaceDialog is an implementation of a Find/Replace Qt dialog
to be used in qt text edit based applications.
A simple Find (only) dialog is also provided.

You can use these dialogs in your applications either by copying
the files in the dialogs directory, or by building the dialogs as library.

You can build the library by using qmake and then make.
For instance, if you use a shadow build (which is reccomended),
simply create a build directory and enter there. Then run

  qmake -recursive ../qtfindreplacedialog.pro
  make

A running example, using the find and find/replace dialogs,
can be found in the directory examples (this will be built also).

You can specify additional arguments for qmake, for instance,
if you want to build the library statically use CONFIG+=static.
Then, you can install it using make install, possibly by
using a specific installation root:

  INSTALL_ROOT=$HOME/usr/local make install
