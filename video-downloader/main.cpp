#include <QCoreApplication>
#include "downloader.h"
#include <QTimer>
#include "../keys.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Downloader dl(APP_KEY, APP_SECRET);

    QObject::connect(&dl, SIGNAL(finished()), &a, SLOT(quit()));

    QTimer::singleShot(0, &dl, SLOT(main()));

    return a.exec();
}

