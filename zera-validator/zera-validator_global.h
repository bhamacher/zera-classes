#ifndef ZERAVALIDATOR_GLOBAL_H
#define ZERAVALIDATOR_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(ZERAVALIDATOR_LIBRARY)
#  define ZERAVALIDATORSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ZERAVALIDATORSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // ZERAVALIDATOR_GLOBAL_H