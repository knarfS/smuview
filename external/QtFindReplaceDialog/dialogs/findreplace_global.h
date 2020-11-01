/*
 * Copyright (C) 2009  Lorenzo Bettini <http://www.lorenzobettini.it>
 * See COPYING file that comes with this distribution
 */

#ifndef FINDREPLACE_GLOBAL_H
#define FINDREPLACE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifndef FINDREPLACESHARED_EXPORT

#if defined(FINDREPLACE_LIBRARY)
#define FINDREPLACESHARED_EXPORT Q_DECL_EXPORT
#else
#define FINDREPLACESHARED_EXPORT Q_DECL_IMPORT
#endif

#endif

#endif // FINDREPLACE_GLOBAL_H
