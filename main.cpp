#include "mainwindow.h"
#include <QApplication>
#include<signal.h>

void handle_signal(int num)
{
}

int main(int argc, char *argv[])
{
    struct sigaction handler;
    handler.sa_handler = handle_signal;
    sigaction(SIGUSR1, &handler, 0);
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    
    return a.exec();
}
