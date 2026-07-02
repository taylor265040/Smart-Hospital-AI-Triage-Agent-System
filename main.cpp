#include "mainwindow.h"
#include "src/style/theme.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Theme::applyTheme(a);

    MainWindow w;
    w.show();

    return a.exec();
}
