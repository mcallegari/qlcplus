#include "dmxkeypad.h"

#define DMXKEYPAD_BUTTON_SIZE   40

DmxKeyPad::DmxKeyPad(QWidget *parent) :
    QWidget(parent)
{
    setupUi();

    m_KPSelectedChannels = new QList<quint32>();
    m_KPStateMachine = new QStateMachine(this);
    m_KPState_Init = new QState();
    m_KPState_Channel = new QState();
    m_KPState_ChannelTHRU = new QState();
    m_KPState_Value = new QState();

    m_KPState_Init->addTransition(this, SIGNAL(SM_InitDone()), m_KPState_Channel);
    m_KPState_Channel->addTransition(this, SIGNAL(SM_ChannelsDone()), m_KPState_Value);
    m_KPState_Channel->addTransition(this, SIGNAL(SM_ChannelTHRU()), m_KPState_ChannelTHRU);

    m_KPState_Channel->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_Value->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);

    m_KPStateMachine->addState(m_KPState_Init);
    m_KPStateMachine->addState(m_KPState_Channel);
    m_KPStateMachine->addState(m_KPState_ChannelTHRU);
    m_KPStateMachine->addState(m_KPState_Value);
    m_KPStateMachine->setInitialState(m_KPState_Init);
    m_KPStateMachine->start();

    connect(m_KPState_Init, SIGNAL(entered()), this, SLOT(SM_Init()));
}

void DmxKeyPad::setupUi()
{
    lay = new QGridLayout(this);
    lay->setContentsMargins(1, 1, 1, 1);
    this->setFixedWidth(3 * DMXKEYPAD_BUTTON_SIZE + 15);

    this->setLayout(lay);

    m_commandDisplay = new QLineEdit(this);
    m_commandDisplay->setEnabled(false);
    m_commandDisplay->setReadOnly(true);
    lay->addWidget(m_commandDisplay, 0, 0, 1, 3);

    m_7 = new QPushButton(this);
    m_7->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_7->setText("7");
    lay->addWidget(m_7, 1, 0);
    m_8 = new QPushButton(this);
    m_8->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_8->setText("8");
    lay->addWidget(m_8, 1, 1);
    m_9 = new QPushButton(this);
    m_9->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_9->setText("9");
    lay->addWidget(m_9, 1, 2);
    m_4 = new QPushButton(this);
    m_4->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_4->setText("4");
    lay->addWidget(m_4, 2, 0);
    m_5 = new QPushButton(this);
    m_5->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_5->setText("5");
    lay->addWidget(m_5, 2, 1);
    m_6 = new QPushButton(this);
    m_6->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_6->setText("6");
    lay->addWidget(m_6, 2, 2);
    m_1 = new QPushButton(this);
    m_1->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_1->setText("1");
    lay->addWidget(m_1, 3, 0);
    m_2 = new QPushButton(this);
    m_2->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_2->setText("2");
    lay->addWidget(m_2, 3, 1);
    m_3 = new QPushButton(this);
    m_3->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_3->setText("3");
    lay->addWidget(m_3, 3, 2);
    m_0 = new QPushButton(this);
    m_0->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_0->setText("0");
    lay->addWidget(m_0, 5, 0);
    m_CLR = new QPushButton(this);
    m_CLR->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_CLR->setText("CLR");
    lay->addWidget(m_CLR, 5, 1);
    m_AT = new QPushButton(this);
    m_AT->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_AT->setText("AT");
    lay->addWidget(m_AT, 5, 2);
    m_MINUS = new QPushButton(this);
    m_MINUS->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_MINUS->setText("-");
    lay->addWidget(m_MINUS, 6, 0);
    m_THRU = new QPushButton(this);
    m_THRU->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_THRU->setText("THRU");
    lay->addWidget(m_THRU, 6, 1);
    m_PLUS = new QPushButton(this);
    m_PLUS->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_PLUS->setText("+");
    lay->addWidget(m_PLUS, 6, 2);
    m_BY = new QPushButton(this);
    m_BY->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_BY->setText("BY");
    lay->addWidget(m_BY, 7, 0);
    m_FULL = new QPushButton(this);
    m_FULL->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_FULL->setText("FULL");
    lay->addWidget(m_FULL, 7, 1);
    m_ENTER = new QPushButton(this);
    m_ENTER->setFixedSize(DMXKEYPAD_BUTTON_SIZE, DMXKEYPAD_BUTTON_SIZE);
    m_ENTER->setText("ENTER");
    lay->addWidget(m_ENTER, 7, 2);

    connect(m_0, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_1, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_2, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_3, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_4, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_5, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_6, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_7, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_8, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_9, SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_CLR, SIGNAL(clicked()), this, SLOT(KP_CLR()));
    connect(m_AT, SIGNAL(clicked()), this, SLOT(KP_AT()));
    connect(m_MINUS, SIGNAL(clicked()), this, SLOT(KP_MINUS()));
    connect(m_THRU, SIGNAL(clicked()), this, SLOT(KP_THRU()));
    connect(m_PLUS, SIGNAL(clicked()), this, SLOT(KP_PLUS()));
    connect(m_BY, SIGNAL(clicked()), this, SLOT(KP_BY()));
    connect(m_FULL, SIGNAL(clicked()), this, SLOT(KP_FULL()));
    connect(m_ENTER, SIGNAL(clicked()), this, SLOT(KP_ENTER()));
}

void DmxKeyPad::calculateTHRURange()
{
    uint i;
    if (m_currentChannel < m_rangeStartChan)
    {
        for (i = m_currentChannel; i <= m_rangeStartChan; i++)
        {
            m_KPSelectedChannels->append(i);
        }
    } else if (m_currentChannel > m_rangeStartChan)
    {
        for (i = m_currentChannel; i >= m_rangeStartChan; i--)
        {
            m_KPSelectedChannels->append(i);
        }
    } else
    {
        m_KPSelectedChannels->append(m_currentChannel);
    }
}

void DmxKeyPad::KP_CLR()
{
    emit SM_Reset();
}

void DmxKeyPad::KP_AT()
{
    if (m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU)) {
        calculateTHRURange();
    }
    m_KPSelectedChannels->append(m_currentChannel); // Only for single channel! Modify later!
    m_currentChannel = 0;
    m_commandDisplay->setText(QString("%1 AT ").arg(m_commandDisplay->text()));
    emit SM_ChannelsDone(); // Change state machine to "Values" state
}
void DmxKeyPad::KP_MINUS()
{

}

void DmxKeyPad::KP_THRU()
{
    // TODO: "FAN" function => Check current state if channel or value
    m_rangeStartChan = m_currentChannel;
    m_currentChannel = 0;
    m_commandDisplay->setText(QString("%1 THRU ").arg(m_commandDisplay->text()));
    emit SM_ChannelTHRU();
}

void DmxKeyPad::KP_PLUS()
{

}

void DmxKeyPad::KP_BY()
{

}

void DmxKeyPad::KP_FULL()
{
    KP_AT(); // FULL always refers to a value, not a channel. So this makes sure we end channel selection if FULL is requested
    if (m_KPStateMachine->configuration().contains(m_KPState_Value) || m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU)) {
        m_currentValue = 255;
        KP_ENTER(); // Wrong in case we want to support "FULL THRU ..."
    }
}

void DmxKeyPad::KP_ENTER()
{
    quint16 chan;
    foreach(chan, *m_KPSelectedChannels)
    {
        if (chan < 1) continue;
        qDebug() << "KEYPAD: SET CHANNEL" << chan << "TO" << m_currentValue;
        emit newChanValue(chan - 1, m_currentValue);
    }
    emit SM_Reset();
}

void DmxKeyPad::SM_Init()
{
    m_KPSelectedChannels->clear();
    m_commandDisplay->setText("");

    m_currentChannel = 0;
    m_rangeStartChan = 0;
    m_currentValue = 0;

    emit SM_InitDone(); // Changes state machine to "Channel" state
}

void DmxKeyPad::addDigitToNumber()
{
    if (sender() == 0) return;
    addDigitToNumber(((QPushButton*)sender())->text().toInt());
}

void DmxKeyPad::addDigitToNumber(quint8 digit)
{
    if (m_KPStateMachine->configuration().contains(m_KPState_Channel) || m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU))
    {
        if ((m_currentChannel * 10 + digit) > 512) return; // Invalid channel
        m_currentChannel = m_currentChannel * 10 + digit;
        m_commandDisplay->setText(QString("%1%2").arg(m_commandDisplay->text()).arg(digit));
        if (m_currentChannel >= 52) KP_AT();
    } else if (m_KPStateMachine->configuration().contains(m_KPState_Value)) {
        if ((m_currentValue * 10 + digit) > 255) return; // Invalid value
        m_currentValue = m_currentValue * 10 + digit;
        m_commandDisplay->setText(QString("%1%2").arg(m_commandDisplay->text()).arg(digit));
        if (m_currentValue >= 26) KP_ENTER();
        if (m_currentValue == 0) KP_ENTER(); // special case: 0 entered and we expect no leading zero => value is 0!
    }
}
