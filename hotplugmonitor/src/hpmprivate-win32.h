#ifndef HPMPRIVATE_WIN32_H
#define HPMPRIVATE_WIN32_H

#include <Windows.h>
#include <QWidget>

class HotPlugMonitor;

/****************************************************************************
 * HPMPrivate declaration
 ****************************************************************************/

class HPMPrivate : public QWidget
{
    Q_OBJECT

public:
    HPMPrivate(HotPlugMonitor* parent);
    ~HPMPrivate();

    void start();
    void stop();

protected:
    bool winEvent(MSG* message, long* result);
    static bool extractVidPid(const QString& dbccName, uint* vid, uint* pid);

private:
    HotPlugMonitor* m_hpm;
    HDEVNOTIFY m_hDeviceNotify;
};

#endif