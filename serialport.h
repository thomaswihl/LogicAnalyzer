#ifndef SERIALPORT_H
#define SERIALPORT_H

#include <QObject>
#include <QRunnable>
#include <QMutex>
#include <QEvent>

#include <stdint.h>
#include <pthread.h>

class SerialPort : public QObject
{
    Q_OBJECT
public:
    explicit SerialPort(QObject* parent, QEvent::Type dataAvailable);
    ~SerialPort();

    void setPort(const QString& device);
    void open();
    void close();
    void get(QList<uint32_t>& data);
    void clearData() { mSignalData.clear(); }
    
signals:
    
public slots:

protected:
    static void* start(void* obj);
    void run();
    void feedByte(uint8_t byte);
    void init();
    
private:
    pthread_t mThreadId;
    QEvent::Type mDataAvailableEvent;
    QString mDevice;
    unsigned mBytesReceived;
    bool mQuit;
    uint32_t mData;
    bool mSynced;
    uint8_t mSyncData[4];
    QMutex mLock;
    QList<uint32_t> mSignalData;
};

#endif // SERIALPORT_H
