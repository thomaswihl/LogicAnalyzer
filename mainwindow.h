#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "serialport.h"

#include <QMainWindow>
#include <QList>
#include <QTime>
#include <QMouseEvent>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual bool event(QEvent *event);
    virtual void paintEvent(QPaintEvent *);
    virtual void closeEvent(QCloseEvent* event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

protected:
    float time(uint32_t data);

    
private:
    static const QEvent::Type DATA_AVAILABLE = QEvent::User;
    Ui::MainWindow *ui;
    SerialPort mSerial;
    QList<uint32_t> mData;
    QTime mTriggerTime;
    float mOffset;
    float mPixelPerSecond;
    int mPressed;
};

#endif // MAINWINDOW_H
