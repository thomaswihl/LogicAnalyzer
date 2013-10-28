#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPainter>
#include <QCloseEvent>
#include <QTime>
#include <QDebug>
#include <QStaticText>

#include <cmath>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    mSerial(this, DATA_AVAILABLE),
    mOffset(0.0f),
    mPixelPerSecond(1000000.0f)
{
    ui->setupUi(this);
    mTriggerTime = QTime::currentTime();
    mSerial.setPort("/dev/ttyUSB0");
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::event(QEvent* event)
{
    if (event->type() == DATA_AVAILABLE)
    {
        QTime now = QTime::currentTime();
        if (mTriggerTime.msecsTo(now) > 1000)
        {
            qDebug() << "CLEAR";
            mData.clear();
        }
        mTriggerTime = now;
        mSerial.get(mData);
        update();
        return true;
    }
    return QMainWindow::event(event);
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    float dy = height() / 8.0f;

    QPen light(Qt::white, 1);
    QPen medium(Qt::white, 2);
    QPen hard(Qt::white, 4);
    float x = -mOffset * mPixelPerSecond;
    int exp = std::log10(mPixelPerSecond);
    float dx = 10.0f / std::pow(10.0f, exp);
    int count = 0;
    while (x < width())
    {
        if ((count % 10) == 0)
        {
            p.setPen(hard);
            p.drawText(QPointF(x, dy / 2), QString("%1").arg(x / mPixelPerSecond));
        }
        else if ((count % 5) == 0) p.setPen(medium);
        else p.setPen(light);
        p.drawLine(x, 0, x, height());
        x += dx * mPixelPerSecond;
        ++count;
    }


    p.setPen(QPen(Qt::black, 1));

    x = -mOffset * mPixelPerSecond;
    bool v[2][8];

    for (int i = 0; i < mData.size(); ++i)
    {
        uint32_t d = mData[i];
        for (int j = 0; j < 8; ++j)
        {
            v[1][j] = v[0][j];
            v[0][j] = (d & (0x1000000 << j)) != 0;
        }
        if (i != 0)
        {
            float dx = time(d) * mPixelPerSecond;
            if (x + dx > 0 && x < width())
            {
                for (int j = 0; j < 8; ++j)
                {
                    float yl = (j + 0.9f) * dy;
                    float yh = (j + 0.1f) * dy;
                    p.drawLine(x, v[1][j] ? yh : yl, x + dx, v[1][j] ? yh : yl);
                    if (v[1][j] != v[0][j])
                    {
                        p.drawLine(x + dx, v[1][j] ? yh : yl, x + dx, v[0][j] ? yh : yl);
                    }
                    if (j == 7)
                    {
                        p.drawText(QPointF(x + dx, yh), QString("%1").arg(d & 0xffffff));
                        p.drawText(QPointF(x + dx, yl), QString("%1").arg(d >> 24));
                    }
                }
            }
            x += dx;
        }
    }
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    mSerial.close();
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent* event)
{
    //qDebug() << "MOVE" << event;
    int delta = event->x() - mPressed;
    if (event->modifiers() == Qt::ShiftModifier)
    {
        mOffset -= delta / mPixelPerSecond;
    }
    else
    {
        if (delta < 0) mPixelPerSecond /= 1.0f - delta / 10.0f;
        else mPixelPerSecond *= 1.0f + delta / 10.0;
    }
    mPressed = event->x();
    update();
}

void MainWindow::mousePressEvent(QMouseEvent* event)
{
    mPressed = event->x();
    //qDebug() << "PRESS" << event;
}

void MainWindow::mouseReleaseEvent(QMouseEvent* event)
{
    //qDebug() << "RELEASE" << event;
}

float MainWindow::time(uint32_t data)
{
    return ((data & 0xffffff) + 1) / 200000000.0f;
}
