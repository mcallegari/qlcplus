#include "dmxkeypad.h"

#define DMXKEYPAD_BUTTON_SIZE   40

DmxKeyPad::DmxKeyPad(QWidget *parent) :
    QWidget(parent)
{
    setupUi();

    m_KPSelectedChannels = new QList<int>();
    m_KPStateMachine = new QStateMachine(this);
    m_KPState_Init = new QState();
    m_KPState_Channel = new QState();
    m_KPState_Value = new QState();
    m_KPStateMachine->setInitialState(m_KPState_Init);
    m_KPStateMachine->start();
}

void DmxKeyPad::setupUi()
{
    lay = new QGridLayout(this);
    lay->setContentsMargins(1, 1, 1, 1);
    this->setFixedWidth(3 * DMXKEYPAD_BUTTON_SIZE + 15);

    this->setLayout(lay);

    QLineEdit* kP_commandDisplay = new QLineEdit(this);
    kP_commandDisplay->setEnabled(false);
    kP_commandDisplay->setReadOnly(true);
    lay->addWidget(kP_commandDisplay, 0, 0, 1, 3);

    QPushButton* kP_7 = new QPushButton(this);
    kP_7->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_7->setText("7");
    lay->addWidget(kP_7, 1, 0);
    QPushButton* kP_8 = new QPushButton(this);
    kP_8->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_8->setText("8");
    lay->addWidget(kP_8, 1, 1);
    QPushButton* kP_9 = new QPushButton(this);
    kP_9->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_9->setText("9");
    lay->addWidget(kP_9, 1, 2);
    QPushButton* kP_4 = new QPushButton(this);
    kP_4->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_4->setText("4");
    lay->addWidget(kP_4, 2, 0);
    QPushButton* kP_5 = new QPushButton(this);
    kP_5->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_5->setText("5");
    lay->addWidget(kP_5, 2, 1);
    QPushButton* kP_6 = new QPushButton(this);
    kP_6->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_6->setText("6");
    lay->addWidget(kP_6, 2, 2);
    QPushButton* kP_1 = new QPushButton(this);
    kP_1->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_1->setText("1");
    lay->addWidget(kP_1, 3, 0);
    QPushButton* kP_2 = new QPushButton(this);
    kP_2->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_2->setText("2");
    lay->addWidget(kP_2, 3, 1);
    QPushButton* kP_3 = new QPushButton(this);
    kP_3->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_3->setText("3");
    lay->addWidget(kP_3, 3, 2);
    QPushButton* kP_0 = new QPushButton(this);
    kP_0->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_0->setText("0");
    lay->addWidget(kP_0, 5, 0);
    QPushButton* kP_CLR = new QPushButton(this);
    kP_CLR->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_CLR->setText("CLR");
    lay->addWidget(kP_CLR, 5, 1);
    QPushButton* kP_AT = new QPushButton(this);
    kP_AT->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_AT->setText("AT");
    lay->addWidget(kP_AT, 5, 2);
    QPushButton* kP_MINUS = new QPushButton(this);
    kP_MINUS->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_MINUS->setText("-");
    lay->addWidget(kP_MINUS, 6, 0);
    QPushButton* kP_THRU = new QPushButton(this);
    kP_THRU->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_THRU->setText("THRU");
    lay->addWidget(kP_THRU, 6, 1);
    QPushButton* kP_PLUS = new QPushButton(this);
    kP_PLUS->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_PLUS->setText("+");
    lay->addWidget(kP_PLUS, 6, 2);
    QPushButton* kP_BY = new QPushButton(this);
    kP_BY->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_BY->setText("BY");
    lay->addWidget(kP_BY, 7, 0);
    QPushButton* kP_FULL = new QPushButton(this);
    kP_FULL->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_FULL->setText("FULL");
    lay->addWidget(kP_FULL, 7, 1);
    QPushButton* kP_ENTER = new QPushButton(this);
    kP_ENTER->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    kP_ENTER->setText("ENTER");
    lay->addWidget(kP_ENTER, 7, 2);
}
