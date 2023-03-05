#include <QScreen>

class CrossPlatformScreen
{
public:
    CrossPlatformScreen(QScreen* screen)
        : screen(screen)
    {}

    QString name() const;

    QString manufacturer() const;
    QString model() const;
    QString serialNumber() const;

private:
    QScreen *screen;
};
