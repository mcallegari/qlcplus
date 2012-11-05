#include <QCoreApplication>
#ifdef WIN32
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
