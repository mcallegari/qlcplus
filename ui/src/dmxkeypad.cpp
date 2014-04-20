#include "dmxkeypad.h"

#define DMXKEYPAD_BUTTON_SIZE   40

DmxKeyPad::DmxKeyPad(QWidget *parent) :
    QWidget(parent)
{
    qDebug() << Q_FUNC_INFO << "parent:" << parent;

    setupUi();

    m_KPSelectedChannels = new QList<quint32>();

    m_KPStateMachine = new QStateMachine(this);
    m_KPState_Init = new QState();
    m_KPState_Channel = new QState();
    m_KPState_ChannelTHRU = new QState();
    m_KPState_StepSize = new QState();
    m_KPState_Value = new QState();
    m_KPState_ValueTHRU = new QState();

    // Valid/possible transitions from one state to another. Triggered by SIGNALs
    m_KPState_Init->addTransition(this, SIGNAL(SM_InitDone()), m_KPState_Channel);

    m_KPState_Channel->addTransition(this, SIGNAL(SM_ChannelsDone()), m_KPState_Value);
    m_KPState_Channel->addTransition(this, SIGNAL(SM_ChannelTHRU()), m_KPState_ChannelTHRU);
    m_KPState_Channel->addTransition(this, SIGNAL(SM_AddRange()), m_KPState_Channel);
    m_KPState_Channel->addTransition(this, SIGNAL(SM_SubtractRange()), m_KPState_Channel);

    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_ChannelsDone()), m_KPState_Value);
    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_ByStart()), m_KPState_StepSize);
    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_AddRange()), m_KPState_Channel);
    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_SubtractRange()), m_KPState_Channel);

    m_KPState_StepSize->addTransition(this, SIGNAL(SM_ChannelsDone()), m_KPState_Value);

    m_KPState_Value->addTransition(this, SIGNAL(SM_ValueTHRU()), m_KPState_ValueTHRU);

    // Transitions from every possible state to Init via SM_Reset()
    m_KPState_Channel->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_ChannelTHRU->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_StepSize->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_Value->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);
    m_KPState_ValueTHRU->addTransition(this, SIGNAL(SM_Reset()), m_KPState_Init);

    // Connect the entered() and exited() SIGNALs to some SLOTs that do the actual work/calculations
    connect(m_KPState_Init, SIGNAL(entered()), this, SLOT(SM_Init()));
    connect(m_KPState_Channel, SIGNAL(exited()), this, SLOT(SM_ChannelExited()));
    connect(m_KPState_ChannelTHRU, SIGNAL(exited()), this, SLOT(SM_ChannelTHRUExited()));
    connect(m_KPState_StepSize, SIGNAL(exited()), this, SLOT(SM_StepSizeExited()));

    // Add all states to the StateMachine and start it
    m_KPStateMachine->addState(m_KPState_Init);
    m_KPStateMachine->addState(m_KPState_Channel);
    m_KPStateMachine->addState(m_KPState_ChannelTHRU);
    m_KPStateMachine->addState(m_KPState_StepSize);
    m_KPStateMachine->addState(m_KPState_Value);
    m_KPStateMachine->addState(m_KPState_ValueTHRU);
    m_KPStateMachine->setInitialState(m_KPState_Init);
    m_KPStateMachine->start();
}

void DmxKeyPad::setupUi()
{
    qDebug() << Q_FUNC_INFO;

    // Layout
    lay = new QGridLayout(this);
    lay->setContentsMargins(1, 1, 1, 1);
    this->setFixedWidth(3 * DMXKEYPAD_BUTTON_SIZE + 15);
    this->setLayout(lay);

    // LineEdit to display current command status
    m_commandDisplay = new QLineEdit(this);
    m_commandDisplay->setEnabled(false);
    m_commandDisplay->setReadOnly(true);
    lay->addWidget(m_commandDisplay, 0, 0, 1, 3);

    // Buttons in the order they are layed out
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

    // Connections handling button clicks
    connect(m_0,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_1,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_2,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_3,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_4,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_5,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_6,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_7,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_8,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_9,     SIGNAL(clicked()), this, SLOT(addDigitToNumber()));
    connect(m_CLR,   SIGNAL(clicked()), this, SLOT(KP_CLR()));
    connect(m_AT,    SIGNAL(clicked()), this, SLOT(KP_AT()));
    connect(m_MINUS, SIGNAL(clicked()), this, SLOT(KP_MINUS()));
    connect(m_THRU,  SIGNAL(clicked()), this, SLOT(KP_THRU()));
    connect(m_PLUS,  SIGNAL(clicked()), this, SLOT(KP_PLUS()));
    connect(m_BY,    SIGNAL(clicked()), this, SLOT(KP_BY()));
    connect(m_FULL,  SIGNAL(clicked()), this, SLOT(KP_FULL()));
    connect(m_ENTER, SIGNAL(clicked()), this, SLOT(KP_ENTER()));
}

/*********************************************************************
 * Button handling slots
 *********************************************************************/

void DmxKeyPad::KP_CLR()
{
    qDebug() << Q_FUNC_INFO;

    emit SM_Reset();
}

void DmxKeyPad::KP_AT()
{
    qDebug() << Q_FUNC_INFO;

    appendToCommand(" AT ");
    emit SM_ChannelsDone(); // Change state machine to "Values" state
}

void DmxKeyPad::KP_MINUS()
{
    qDebug() << Q_FUNC_INFO;

    if (m_KPStateMachine->configuration().contains(m_KPState_Channel) || m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU))
    {
        emit SM_SubtractRange(); // re-calculate the current (pre-Subtract range)
        m_currentChannel = 0;
        m_subtractFromRange = true;
        appendToCommand(" - ");
    }
}

void DmxKeyPad::KP_THRU()
{
    qDebug() << Q_FUNC_INFO;

    if (m_KPStateMachine->configuration().contains(m_KPState_Channel))
    {
        m_rangeStartChan = m_currentChannel;
        m_currentChannel = 0;
        appendToCommand(" THRU ");
        emit SM_ChannelTHRU();
    }
    else if (m_KPStateMachine->configuration().contains(m_KPState_Value))
    {
        m_fanStartValue = m_currentValue;
        m_currentValue = 0;
        appendToCommand(" THRU ");
        emit SM_ValueTHRU();
    }
}

void DmxKeyPad::KP_PLUS()
{
    qDebug() << Q_FUNC_INFO;

    if (m_KPStateMachine->configuration().contains(m_KPState_Channel) || m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU))
    {
        emit SM_AddRange(); // re-calculate the current (pre-Add range)
        m_currentChannel = 0;
        m_addToRange = true;
        appendToCommand(" + ");
    }
}

void DmxKeyPad::KP_BY()
{
    qDebug() << Q_FUNC_INFO;

    appendToCommand(" BY ");
    emit SM_ByStart();
}

void DmxKeyPad::KP_FULL()
{
    qDebug() << Q_FUNC_INFO;

    KP_AT(); // FULL always refers to a value, not a channel. So this makes sure we end channel selection if FULL is requested
    if (m_KPStateMachine->configuration().contains(m_KPState_Value)) {
        m_currentValue = 255;
        appendToCommand("FULL");
        KP_ENTER(); // FULL implies ENTER for simplicity. If you want "FULL" and FAN function, use 255 please!
    }
}

void DmxKeyPad::KP_ENTER()
{
    qreal valueStepSize = 0;
    qint32 i = 0;

    qDebug() << Q_FUNC_INFO << "currentValue:" << m_currentValue << "fanStartValue" << m_fanStartValue;

    // Remove invalid channel
    if (m_KPSelectedChannels->contains(0)) m_KPSelectedChannels->removeAll(0);

    if (m_KPStateMachine->configuration().contains(m_KPState_ValueTHRU))
    {
        valueStepSize = (m_currentValue - m_fanStartValue) / (qreal)(m_KPSelectedChannels->count() - 1);
    }
    else
    {
        m_fanStartValue = m_currentValue;
    }

    qDebug() << "valueStepSize" << valueStepSize;

    uint chan;
    foreach(chan, *m_KPSelectedChannels)
    {
        if (chan < 1) continue;

        qDebug() << "KEYPAD: SET CHANNEL" << chan << "TO" << m_fanStartValue + i*valueStepSize;
        emit newChanValue(chan - 1, m_fanStartValue + i*valueStepSize);
        i++;
    }
    emit newValuesDone();
    emit SM_Reset();
}

void DmxKeyPad::addDigitToNumber()
{
    qDebug() << Q_FUNC_INFO << "sender:" << (QPushButton*)sender();

    if (sender() == 0) return;
    addDigitToNumber(((QPushButton*)sender())->text().toInt());
}

/*********************************************************************
 * private (helper) methods
 *********************************************************************/

void DmxKeyPad::addDigitToNumber(quint8 digit)
{
    qDebug() << Q_FUNC_INFO << "digit:" << digit;

    if (m_KPStateMachine->configuration().contains(m_KPState_StepSize))
    {
        if ((m_byStepSize * 10 + digit) > 512) return; // Invalid channel
        if (m_byStepSize == 1)
        {
            m_byStepSize = digit;
        }
        else
        {
            m_byStepSize = m_byStepSize * 10 + digit;
        }
        appendToCommand(QString("%1").arg(digit));
    }
    else if (m_KPStateMachine->configuration().contains(m_KPState_Channel) || m_KPStateMachine->configuration().contains(m_KPState_ChannelTHRU))
    {
        if ((m_currentChannel * 10 + digit) > 512) return; // Invalid channel
        m_currentChannel = m_currentChannel * 10 + digit;
        appendToCommand(QString("%1").arg(digit));
    }
    else if (m_KPStateMachine->configuration().contains(m_KPState_Value) || m_KPStateMachine->configuration().contains(m_KPState_ValueTHRU))
    {
        if ((m_currentValue * 10 + digit) > 255) return; // Invalid value
        m_currentValue = m_currentValue * 10 + digit;
        appendToCommand(QString("%1").arg(digit));
    }
}

void DmxKeyPad::calculateTHRURange()
{
    qDebug() << Q_FUNC_INFO << "rangeStart:" << m_rangeStartChan << "currentChannel:" << m_currentChannel << "addToRange:" << m_addToRange << "subtractRange:" << m_subtractFromRange;
    qDebug() << "Selected channels PRE:" << *m_KPSelectedChannels;

    if (!m_addToRange && !m_subtractFromRange) m_KPSelectedChannels->clear();

    uint i;
    if (m_currentChannel < m_rangeStartChan) // range defined in reverse order (higher channel to lower channel)
    {
        for (i = m_rangeStartChan; i >= m_currentChannel; i = i - m_byStepSize)
        {
            if (i == UINT_MAX) break; // don't shoot through the floor if going down to channel 0 was requested
            if (!m_subtractFromRange)
            {
                if (!m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->append(i);
            }
            else
            {
                if (m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->removeAll(i);
            }
        }
    } else if (m_currentChannel > m_rangeStartChan) // range defined in regular order (lower channel to higher channel)
    {
        for (i = m_rangeStartChan; i <= m_currentChannel; i = i + m_byStepSize)
        {
            if (!m_subtractFromRange)
            {
                if (!m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->append(i);
            }
            else
            {
                if (m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->removeAll(i);
            }
        }
    } else // range start = range end => only one channel selected
    {
        if (!m_subtractFromRange)
        {
            if (!m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->append(i);
        }
        else
        {
            if (m_KPSelectedChannels->contains(i)) m_KPSelectedChannels->removeAll(i);
        }
    }

    m_subtractFromRange = false; // Needs another "-" to subtract another range
    if (m_addToRange) m_currentChannel = 0;

    qDebug() << "Selected channels POST:" << *m_KPSelectedChannels;
}

void DmxKeyPad::appendToCommand(QString text)
{
    qDebug() << Q_FUNC_INFO << "text:" << text;

    m_commandDisplay->setText(QString("%1%2").arg(m_commandDisplay->text()).arg(text));
}

/*********************************************************************
 * StateMachine workhorse methods/slots
 *********************************************************************/

void DmxKeyPad::SM_Init()
{
    qDebug() << Q_FUNC_INFO;

    m_KPSelectedChannels->clear();
    m_commandDisplay->setText("");

    m_currentChannel = 0;
    m_rangeStartChan = 0;
    m_currentValue = 0;
    m_fanStartValue = 0;
    m_byStepSize = 1;
    m_addToRange = false;
    m_subtractFromRange = false;

    emit SM_InitDone(); // Changes state machine to "Channel" state
}

void DmxKeyPad::SM_ChannelExited()
{
    qDebug() << Q_FUNC_INFO;

    if (!m_KPSelectedChannels->contains(m_currentChannel)) m_KPSelectedChannels->append(m_currentChannel);

    m_currentChannel = 0; // In case of +/- after a single channel
}

void DmxKeyPad::SM_ChannelTHRUExited()
{
    qDebug() << Q_FUNC_INFO;

    calculateTHRURange();
}

void DmxKeyPad::SM_StepSizeExited()
{
    qDebug() << Q_FUNC_INFO;

    calculateTHRURange();
}
