#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <iostream>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include "qtdropbox/qdropbox.h"
#include "qtdropbox/qdropboxfile.h"
#include "../keys.h"

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(QString key, QString secret, QObject* parent = 0);


signals:
    /*!
     * \brief emitted when the main function is finished and the app can be closed
     */
    void finished();

public slots:
    /*!
     * \brief main routine. acts like main() of a cpp program
     */
    void main();

    /*!
     * \brief call this to prompt the user for re(authorisation) of the app with dropbox
     */
    void authorise();

private:
    QDropbox* _dropbox; //!< API to dropbox

    bool connect(); //!< connect to dropbox and save access token
    bool setTokenFromFile(); //!< reuse a previously saved token
    QStringList filterVideos(QString folder); //!< use file metadata to check for videos in a folder
    void downloadFile(QString file); //!< download a frile from dropbox
};

#endif // DOWNLOADER_H
