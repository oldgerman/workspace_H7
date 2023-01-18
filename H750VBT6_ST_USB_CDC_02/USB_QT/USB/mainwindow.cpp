#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lusb0_usb.h"

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QCategoryAxis>

//#pragma warning (disable:4819)

#define m_dev_VENDOR    0xC251	 /* Vendor ID of the m_dev         */
#define m_dev_PRODUCT   0x3505	 /* Product ID of the m_dev        */
#define m_dev_DATA_SIZE 10240    /* Number of bytes to get at once */

char RecTraData[m_dev_DATA_SIZE] = {0};
struct usb_bus *bus;
struct usb_device * m_dev = NULL;
struct usb_dev_handle *m_dev_handle;

int OpenUSB(void)
{
    int ret;

    QString str;

    /* 第1步：USB 初始化 */
    usb_init();

    /* 第2步：寻找系统上的usb总线，任何usb设备都通过usb总线和计算机总线通信。进而和其他设备通信。此函数返回总线数 */
    ret = usb_find_busses();
    if(ret < 0)
    {

    }

    /* 第3步：寻找总线上的usb设备，这个函数必要在调用usb_find_busses()后使用。以上的三个函数都是一开始就要用到的，此函数返回设备数量 */
    ret = usb_find_devices();
    if(ret < 0)
    {
        str = str.asprintf("寻找USB设备失败：%d", ret);
        qDebug() << str;
        return -1;
    }
    else
    {
        /* 成功 */
        str = str.asprintf("寻找USB设备成功：%d", ret);
        qDebug() << str;
    }

    /*  第4步：这个函数返回总线的列表，在高一些的版本中已经用不到了 */
    for(bus = usb_get_busses(); bus; bus = bus->next)
    {
        struct usb_device *dev;
        for(dev = bus->devices; dev; dev = dev->next)
        {
            if(dev->descriptor.idVendor == m_dev_VENDOR && dev->descriptor.idProduct == m_dev_PRODUCT)
            m_dev=dev;
        }
    }
    if(!m_dev)
    {  
        str = str.asprintf("设备查找失败：%d", -2);
        qDebug() << str;
        return -2;
    }

    /*  第5步：
        打开要使用的设备，在对硬件进行操作前必须要调用usb_open 来打开设备，这里大家看到有两个结构体
        usb_dev_handle 和 usb_device 是我们在开发中经常碰到的，有必要把它们的结构看一看。在libusb 中
        的usb.h和usbi.h中有定义。

        这里我们不妨理解为返回的 usb_dev_handle 指针是指向设备的句柄，而行参里输入就是需要打开的设备。
    */
    m_dev_handle = usb_open(m_dev);
    if(!m_dev_handle)
    {
        str = str.asprintf("设备打开失败：%d", -3);
        qDebug() << str;
        return -3;
    }

    /*  第6步：
        设置当前设备使用的configuration，参数configuration 是你要使用的configurtation descriptoes中的
        bConfigurationValue, 返回0成功，<0失败( 一个设备可能包含多个configuration,比如同时支持高速和
        低速的设备就有对应的两个configuration,详细可查看usb标准)

    */
    if(usb_set_configuration(m_dev_handle, 1) < 0)
    {
         /* 失败 未处理 */
    }

    /*
        第7步：
        注册与操作系统通信的接口，这个函数必须被调用，因为只有注册接口，才能做相应的操作。

        Interface 指 bInterfaceNumber. (usb_release_interface 与之相对应，也是必须调用的函数)
    */
    if(usb_claim_interface(m_dev_handle, 1) < 0) //claim_interface 0指向第一个设备
    {
         /* 失败 未处理 */
    }

    return 1;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{ 
    ui->setupUi(this);

    /* ====================================  */
    QChart *chart = new QChart();
    chart->legend()->hide();
    chart->setTitle("Simple line chart example");

    /* ====================================  */
    QValueAxis *axisX = new QValueAxis;
    axisX->setTickCount(10);
    axisX->setMin(0);
    axisX->setMax(10);
    chart->addAxis(axisX, Qt::AlignBottom);

    /* ====================================  */
    series = new QLineSeries();
    chart->addSeries(series);

    /* ====================================  */
    QValueAxis *axisY = new QValueAxis;
    axisY->setLinePenColor(series->pen().color());
    axisY->setMin(0);
    axisY->setMax(100);

    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setGeometry(QRect(10, 360, 791, 261));
    chartView->setParent(this);

    /* 创建定时器 */
    timer = new QTimer(this);

    /* 关联槽函数 */
    connect(timer, SIGNAL(timeout()), this, SLOT(usbdataupdate()));

    /* 启动定时器，100ms一次 */
    timer->start(100);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    QString debugstr;
    int usbstatus;
    char str[64];

    if (UsbOpenFlag == 0)
    {
        /* 打开USB连接 */
        usbstatus = OpenUSB();

        if (usbstatus == -1)
        {
            ui->label->setText("USB开关状态：寻找USB设备失败");
        }
        else if (usbstatus == -2)
        {
            ui->label->setText("USB开关状态：设备查找失败，VID和PID不匹配");
        }
        else if (usbstatus == -3)
        {
            ui->label->setText("USB开关状态：设备打开失败");
        }
        else
        {
            /* 打开成功 */
            UsbOpenFlag = 1;

            ui->label->setText("USB开关状态：设备打开成功");

            ui->pushButton->setText("关闭USB设备");

            debugstr = debugstr.asprintf("设备名：%s", m_dev->filename);
            ui->label_2->setText(debugstr);

            usb_get_string_simple(m_dev_handle, m_dev->descriptor.iManufacturer, str, sizeof(str));
            debugstr = debugstr.asprintf("厂商：%s", str);

            ui->label_3->setText(debugstr);

            usb_get_string_simple(m_dev_handle, m_dev->descriptor.iProduct, str, sizeof(str));
            debugstr = debugstr.asprintf("产品：%s", str);
            ui->label_4->setText(debugstr);

            usb_get_string_simple(m_dev_handle, m_dev->descriptor.iSerialNumber, str, sizeof(str));
            debugstr = debugstr.asprintf("序列号：%s",str);

            ui->label_5->setText(debugstr);
        }
    }
    else
    {
        UsbOpenFlag = 0;
        DataSampleStart = 0;
        usb_release_interface(m_dev_handle, 1);
        //usb_reset(m_dev_handle);
       // usb_close(m_dev_handle);

        ui->label->setText("USB开关状态：关闭");
        ui->pushButton->setText("打开USB设备");
        ui->pushButton_2->setText("开启数据采集");
    }

    qDebug()<<str;
}

void MainWindow::usbdataupdate()
{
    int ret;
    QString debugstr;

    static uint32_t timeStamp = 0; //时间戳
    qDebug("timer：%lums", timeStamp);
    timeStamp += 100;   //定时器，100ms一次

    if ((UsbOpenFlag == 1) && (DataSampleStart == 1))
    {
        /*
           H750VBT6_ST_USB_CDC_02工程的 usbd_cdc.h 中断端点的地址 CDC_CMD_EP 是 0x82U， 0x81和0x01作为BULK端点IN/OUT地址
           因为 m_dev->config->interface->altsetting->endpoint->bEndpointAddress值为0x82，所以需要减1才是BULK端点IN地址
           注意：硬汉哥的教程中RL-USB的配置是STM32F429BIT6的 0x81 作为 中断端点地址, 0x82和0x02作为BULK端点IN/OUT地址
        */
        ret = usb_bulk_read(m_dev_handle,
                            m_dev->config->interface->altsetting->endpoint->bEndpointAddress - 1,   //endpoint IN 地址 0x81
                            RecTraData,
                            2048,
                            128);

        qDebug("bEndpointAddress: 0x%02X", m_dev->config->interface->altsetting->endpoint->bEndpointAddress - 1);   //打印IN端点地址
        /* 显示返回的数据个数 */
        debugstr = debugstr.asprintf("返回字节数：%d", ret);

        ui->label_7->setText(debugstr);

        /* 显示前25个数据 */
        debugstr = debugstr.asprintf("返回部分数据：%02X %02X %02X %02X %02X ",
                      RecTraData[0], RecTraData[1], RecTraData[2], RecTraData[3], RecTraData[4]);

        for (int i = 5; i < 25; i+=5)
        {
            debugstr += debugstr.asprintf("%02X %02X %02X %02X %02X ",
                  RecTraData[i], RecTraData[i+1], RecTraData[i+2], RecTraData[i+3], RecTraData[i+4]);
        }

        ui->label_8->setText(debugstr);

        series->clear();
        for(int i = 0; i < 10; i++)
        {
            *series << QPointF(i, RecTraData[i]);
        }

        /* fibre框架处理USB接收的消息要求至少末尾要有回车符 */
        RecTraData[1] = '\r';
        RecTraData[2] = '\n';

        if(DataSampleStart == 1)
        {
            RecTraData[0] = 0x02;
            /* 执行发送 */
            ret = usb_bulk_write(m_dev_handle,
                                 1,                 //endpoint OUT 地址 0x01
                                 RecTraData,
                                 3,                 //以 一个数字+"\r\n" 的格式发送命令，共3个字符
                                 128);
            debugstr = debugstr.asprintf("发送命令：%02X", RecTraData[0]);
        }


        ui->label_6->setText(debugstr);
    }

}

void MainWindow::on_pushButton_2_clicked()
{
    if (UsbOpenFlag == 1)
    {
        /* 启动数据采集*/
        if (DataSampleStart == 0)
        {
            DataSampleStart = 1;
            ui->pushButton_2->setText("停止数据采集");

        }
        /* 停止数据采集 */
        else
        {
            DataSampleStart = 0;
            ui->pushButton_2->setText("开启数据采集");
        }
    }
}

void MainWindow::on_groupBox_clicked()
{

}
