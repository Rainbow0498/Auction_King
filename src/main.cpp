#include "ui/MainWindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    bbae::MainWindow window;
    window.show();
    return app.exec();
}

