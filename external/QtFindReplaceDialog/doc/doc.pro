# for installing documentation

HTMLDIR=$$PWD/output/html

html_docs.files = $$HTMLDIR/*
html_docs.path = /share/doc/qtfindreplacedialog
html_docs.CONFIG += no_check_exist

INSTALLS += html_docs

OTHER_FILES = Doxyfile \
    qtfindreplacedialog.dox
