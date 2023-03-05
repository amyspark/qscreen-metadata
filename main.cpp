#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QWindow>

#include "screens.h"
#include "ui_mainwidget.h"

class WdgMain: public QWidget, public Ui::MainWidget
{
public:
    WdgMain(QWidget* parent = nullptr)
        : QWidget(parent)
        , Ui::MainWidget()
    {
        setupUi(this);
    }
};

class WindowMain: public QMainWindow
{
public:
    WindowMain(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags())
        : QMainWindow(parent, flags)
    {
        centralWidget = new WdgMain(this);
        setCentralWidget(centralWidget);
        setContentsMargins(0, 0, 0, 0);
    }

    void populate(QScreen *s)
    {
        CrossPlatformScreen screen(s);
        centralWidget->txtId->setText(QString::number(QApplication::screens().indexOf(s)));
        centralWidget->txtName->setText(screen.name());
        centralWidget->txtManufacturer->setText(screen.manufacturer());
        centralWidget->txtModel->setText(screen.model());
        centralWidget->txtSerial->setText(screen.serialNumber());
    }

    void showEvent(QShowEvent *e) override
    {
        QMainWindow::showEvent(e);
        populate(windowHandle()->screen());
        connect(windowHandle(), &QWindow::screenChanged, this, &WindowMain::populate);
    }

private:
    WdgMain *centralWidget = nullptr;
};

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    WindowMain mainWin;
    mainWin.show();
    return app.exec();
}
