#!/bin/bash

# a script to make a .tar.gz (with version in the name)
# the archive will contain a directory $PACKAGE-$VERSION with
# all the files in the current directory with some exclusions:
# CVS files and build directories

PACKAGE=qtfindreplacedialog
VERSION=1.1

tar \
    --exclude-vcs \
    --exclude='*~' \
    --exclude='*.tar.gz' \
    --exclude='*.user' \
    --exclude='build*' \
    -czf $PACKAGE-$VERSION.tar.gz \
    --transform="s,^.,$PACKAGE-$VERSION," --verbose .

