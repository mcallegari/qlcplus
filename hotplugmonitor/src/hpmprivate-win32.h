#ifndef HPMPRIVATE_WIN32_H
#define HPMPRIVATE_WIN32_H

#include <Windows.h>
#include <QObject>
#include <qwindowdefs.h>

class HotPlugMonitor;

/****************************************************************************
 * HPMPrivate declaration
 ****************************************************************************/

class HPMPrivate: public QObject
{
    Q_OBJECT

public:
    HPMPrivate(HotPlugMonitor* parent);
    virtual ~HPMPrivate();

    void start();
    void stop();

    void setWinId(WId id);
    bool processWinEvent(MSG* message, long* result);

protected:
    static bool extractVidPid(const QString& dbccName, uint* vid, uint* pid);

private:
    HotPlugMonitor* m_hpm;
    HDEVNOTIFY m_hDeviceNotify;
};

#endif
