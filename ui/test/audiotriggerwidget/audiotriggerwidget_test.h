#ifndef AUDIOTRIGGERWIDGET_TEST_H
#define AUDIOTRIGGERWIDGET_TEST_H

#include <QObject>

class AudioTriggerWidget_Test : public QObject
{
    Q_OBJECT

private slots:
    void basics();
    void display();
};

#endif
