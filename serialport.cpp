#include "serialport.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdexcept>
#include <unistd.h>
#include <error.h>
#include <errno.h>

#include <QThreadPool>
#include <QCoreApplication>
#include <QDebug>
#include <signal.h>


SerialPort::SerialPort(QObject *parent, QEvent::Type dataAvailable) :
    QObject(parent),
    mThreadId(pthread_self()),
    mDataAvailableEvent(dataAvailable),
    mBytesReceived(0),
    mQuit(false),
    mData(0),
    mSynced(false)
{
    init();
}

SerialPort::~SerialPort()
{
    close();
}

void SerialPort::setPort(const QString& device)
{
    mDevice = device;
    open();
}

void SerialPort::open()
{
    close();
    mQuit = false;
    if (pthread_create(&mThreadId, 0, start, this) != 0) throw std::runtime_error("Couldn't start thread");

}

void SerialPort::close()
{
    if (!pthread_equal(mThreadId, pthread_self()))
    {
        mQuit = true;
        qDebug() << "Quitting";
        pthread_kill(mThreadId, SIGUSR1);
        void* ret;
        pthread_join(mThreadId, &ret);
        qDebug() << "Return" << ret;
        mThreadId = pthread_self();
    }
}

void SerialPort::get(QList<uint32_t>& data)
{
    QMutexLocker ml(&mLock);
    data.append(mSignalData);
    mSignalData.clear();
}

void* SerialPort::start(void* obj)
{
    SerialPort* po = (SerialPort*)obj;
    po->run();
    return (void*)7;
}

void SerialPort::run()
{
    int fd = ::open(mDevice.toAscii().constData(), O_RDWR | O_NOCTTY);
    if (fd < 0) throw std::runtime_error("Couldn't open device");
    struct termios tio;
    bzero(&tio, sizeof(tio));
    cfmakeraw(&tio);
    if (cfsetspeed(&tio, B921600) < 0)
    {
        qDebug() << "Invalid baudrate:" << strerror(errno);
        close();
        return;
    }
    if (tcflush(fd, TCSAFLUSH) < 0)
    {
        qDebug() << "Couldn't flush:" << strerror(errno);
        close();
        return;
    }
    if (tcsetattr(fd, TCSANOW, &tio) < 0)
    {
        qDebug() << "Couldn't set to raw:" << strerror(errno);
        close();
        return;
    }

    uint8_t buf[256];
    while (!mQuit)
    {
        int res = read(fd, buf, sizeof(buf));
        if (res > 0)
        {
            QMutexLocker ml(&mLock);
            for (int i = 0; i < res; ++i) feedByte(buf[i]);
            ml.unlock();
            if (mDataAvailableEvent != 0) QCoreApplication::postEvent(parent(), new QEvent(mDataAvailableEvent));
        }
        else if (res < 0)
        {
            qDebug() << strerror(errno);
        }
    }
    qDebug() << "Closing";
    ::close(fd);
}

void SerialPort::feedByte(uint8_t byte)
{
    mData |= byte << (8 * mBytesReceived);
    ++mBytesReceived;

    if (mBytesReceived == 4)
    {
        mSignalData.append(mData);
        mData = 0;
        mBytesReceived = 0;
    }
    if (!mSynced)
    {
        mSyncData[3] = mSyncData[2];
        mSyncData[2] = mSyncData[1];
        mSyncData[1] = mSyncData[0];
        mSyncData[0] = byte;
        if ((mSyncData[3] != 0xff && mSyncData[2] == 0xff && mSyncData[1] == 0xff && mSyncData[0] == 0xff) ||
            (mSyncData[3] != 0x00 && mSyncData[2] == 0x00 && mSyncData[1] == 0x00 && mSyncData[0] != 0x00))
        {
            init();
            mSynced = true;
        }
    }
}

void SerialPort::init()
{
    mBytesReceived = 0;
    mData = 0;
    memset(mSyncData, 0, sizeof(mSyncData));
}
