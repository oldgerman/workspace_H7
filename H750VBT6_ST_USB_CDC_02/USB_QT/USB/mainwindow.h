#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QWidget>
#include <QTimer>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT /* 激活元对象，提供了信号和槽机制 */

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    int UsbOpenFlag = 0;     /* USB连接打开和关闭标志，0标志当前关闭，1表示当前打开   */
    int DataSampleStart = 0; /* USB启动数据采集标志，0停止数据采集，1表示启动数据采集 */
    int LedTurnOn = 1;   /* USB控制LED4翻转标志标志，0停止翻转，1表示执行一次翻转 */
    int pushButton_3_clicked = false;

    QTimer *timer;

    QLineSeries *series;

private slots: /* 槽函数声明标志 */
    /* 声明槽函数 */
    void on_pushButton_clicked();

    void on_groupBox_clicked();

    void on_pushButton_2_clicked();

    void usbdataupdate();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
