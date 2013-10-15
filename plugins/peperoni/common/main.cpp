#include <QCoreApplication>
#if defined(WIN32) || defined(Q_OS_WIN)
#   include "win32ioenumerator.h"
#else
#   include "unixioenumerator.h"
#endif

int main(int argc, char** argv)
{
    QCoreApplication a(argc, argv);
#ifdef WIN32
    Win32IOEnumerator e;
#else
    UnixIOEnumerator e;
#endif
    e.rescan();
    return 0;
}
