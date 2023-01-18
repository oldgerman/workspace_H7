#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    /* 创建QApplication对象，用于管理应用程序资源*/
    QApplication a(argc, argv);

    /* 创建主窗体并显示 */
    MainWindow w;
    w.show();

    /* QT接管全部，开始执行 */
    return a.exec();
}
