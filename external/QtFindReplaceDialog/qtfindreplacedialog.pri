!contains( included_modules, QtFindReplaceDialog/qtfindreplacedialog.pri) {
		included_modules += QtFindReplaceDialog/qtfindreplacedialog.pri

INCLUDEPATH += $$PWD/dialogs
DEPENDPATH += $$PWD/dialogs

QtFindReplaceDialog-uselib:!QtFindReplaceDialog-buildlib {
    LIBS += -L$$QTFINDREPLACEDIALOG_LIBDIR -l$$QTFINDREPLACEDIALOG_LIBNAME
} else {
	SOURCES +=	$$PWD/dialogs/finddialog.cpp \
				$$PWD/dialogs/findform.cpp \
				$$PWD/dialogs/findreplaceform.cpp \
				$$PWD/dialogs/findreplacedialog.cpp

	HEADERS +=	$$PWD/dialogs/finddialog.h \
				$$PWD/dialogs/findform.h \
				$$PWD/dialogs/findreplaceform.h \
				$$PWD/dialogs/findreplacedialog.h \
				$$PWD/dialogs/findreplace_global.h

	FORMS +=	$$PWD/dialogs/findreplaceform.ui \
				$$PWD/dialogs/findreplacedialog.ui

	!QtFindReplaceDialog-buildlib:DEFINES += "FINDREPLACESHARED_EXPORT="
}

}
