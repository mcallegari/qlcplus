#ifndef DMXKEYPAD_H
#define DMXKEYPAD_H

#include <QWidget>
#include <QStateMachine>
#include <QList>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

class DmxKeyPad : public QWidget
{
    Q_OBJECT
public:
    explicit DmxKeyPad(QWidget *parent = 0);

private:
    QList<int>* m_KPSelectedChannels;
    QStateMachine* m_KPStateMachine;
    QState* m_KPState_Init; // Clear string, reset List of channels to modify (but do NOT reset universe)
    QState* m_KPState_Channel; // Wait for channel(range) specification
    QState* m_KPState_Value; // Wait for value(range) specification

    QGridLayout* lay;

    void setupUi();

private slots:
    //void KP_clear();

signals:

public slots:

};

#endif // DMXKEYPAD_H
