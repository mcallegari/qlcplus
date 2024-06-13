#ifndef VCSCROLLAREA_H
#define VCSCROLLAREA_H

#include <QObject>
#include <QScrollArea>
#include <QWidget>

class VCScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    explicit VCScrollArea(QWidget *parent = nullptr);

    static bool needsOverride(const QKeySequence& keySequence);

    void setKeyOverrides(const QList<QKeySequence> &keyOverrides);
    void clearKeyOverrides();

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    QList<unsigned int> m_overriddenKeyCodes;
};

#endif // VCSCROLLAREA_H
