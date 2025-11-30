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

    void setKeyPassthruEnabled(bool enabled);

protected:
    void keyPressEvent(QKeyEvent *) override;

private:
    bool m_keyPassthru;
};

#endif // VCSCROLLAREA_H
