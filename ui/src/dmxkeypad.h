#ifndef DMXKEYPAD_H
#define DMXKEYPAD_H

#include <QWidget>
#include <QStateMachine>
#include <QList>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QDebug>

#include "simpledesk.h"

class DmxKeyPad : public QWidget
{
    Q_OBJECT
public:
    explicit DmxKeyPad(QWidget *parent = 0);

private:
    QList<uint>* m_KPSelectedChannels;
    uint m_rangeStartChan;
    uint m_currentChannel;
    uchar m_currentValue;
    uchar m_byStepSize;

    QStateMachine* m_KPStateMachine;
    QState* m_KPState_Init; // Clear string, reset List of channels to modify (but do NOT reset universe)
    QState* m_KPState_Channel; // Wait for channel(range) specification
    QState* m_KPState_ChannelTHRU; // Wait for second part of channel "THRU" specification (range end)
    QState* m_KPState_StepSize; // Wait for entry of step size (entered by "BY")
    QState* m_KPState_Value; // Wait for value(range) specification

    QGridLayout* lay;

    QLineEdit* m_commandDisplay;

    QPushButton* m_0;
    QPushButton* m_1;
    QPushButton* m_2;
    QPushButton* m_3;
    QPushButton* m_4;
    QPushButton* m_5;
    QPushButton* m_6;
    QPushButton* m_7;
    QPushButton* m_8;
    QPushButton* m_9;
    QPushButton* m_CLR;
    QPushButton* m_AT;
    QPushButton* m_MINUS;
    QPushButton* m_THRU;
    QPushButton* m_PLUS;
    QPushButton* m_BY;
    QPushButton* m_FULL;
    QPushButton* m_ENTER;

    void setupUi();
    void calculateTHRURange();

private slots:
    void KP_CLR();
    void KP_AT();
    void KP_MINUS();
    void KP_THRU();
    void KP_PLUS();
    void KP_BY();
    void KP_FULL();
    void KP_ENTER();

    void SM_Init();
    void SM_ChannelTHRUExited();

    void addDigitToNumber();

signals:
    void SM_Reset();
    void SM_InitDone();
    void SM_ChannelsDone();
    void SM_ChannelTHRU();
    void SM_ByStart();

    void newChanValue(uint channel, uchar value);

private:
    void addDigitToNumber(quint8 digit);
    void appendToCommand(QString text);

signals:

public slots:

};

#endif // DMXKEYPAD_H
