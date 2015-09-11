#include "downloader.h"

Downloader::Downloader(QString key, QString secret, QObject *parent) : QObject(parent)
{
    _dropbox = new QDropbox(key, secret, QDropbox::Plaintext, "api.dropbox.com", this);
}

void Downloader::main()
{
    std::cout << "initiating connection (key = " << _dropbox->key().toStdString()
              << ", secret = " << _dropbox->sharedSecret().toStdString() << ")" << std::endl;

    // check for existing token
    if(!setTokenFromFile())
        connect();

    // list all videos
    QStringList videos = filterVideos("/dropbox");
    std::cout << "found " << videos.size() << " video(s) in /dropbox/" << std::endl;

    // download
    QStringList::iterator it;
    QTextStream strin(stdin, QIODevice::ReadOnly);
    for(it=videos.begin(); it != videos.end(); ++it) {
        QString video = *it;
        QString confirmation = "x";
        while(confirmation.toLower().compare("y") != 0 && confirmation.toLower().compare("n")) {
            std::cout << "start download of '" << video.toStdString() << "'? [Y/n] ";
            confirmation = strin.readLine();
        }

        if(confirmation.toLower().compare("y") == 0) {
            downloadFile(video);
        }
    }

    emit finished();
}

void Downloader::authorise()
{
    std::cout << "please visit the following link to authorise this app:" << std::endl << std::endl
              << _dropbox->authorizeLink().toString().toStdString() << std::endl
              << std::endl << "Press any key to continue..." << std::endl;
    std::cin.get();
}

bool Downloader::connect()
{
    // request anonymous token
    if(!_dropbox->requestTokenAndWait()) {
        std::cout << "requesting initial token failed: " << _dropbox->error() << " - " << _dropbox->errorString().toStdString() << std::endl;
        return false;
    }

    // request access token for real access
    bool worked = _dropbox->requestAccessTokenAndWait();

    if(!worked) {
        // ask the user to authorise this application when the token is expired
        while(_dropbox->error() == QDropbox::TokenExpired) {
            // authorise app
            authorise();
            _dropbox->requestAccessTokenAndWait();
        }

        if(_dropbox->error() != QDropbox::NoError) {
            std::cout << "requesting access token failed: " << _dropbox->error() << " - " << _dropbox->errorString().toStdString() << std::endl;
            return false;
        }
    }

    // save access token
    QFile tokenSaveFile("token.txt");
    if(!tokenSaveFile.open(QIODevice::WriteOnly|QIODevice::Truncate|QIODevice::Text))
    {
        qDebug() << "Could not save token: " << tokenSaveFile.errorString();
        return false;
    }
    QTextStream stream(&tokenSaveFile);
    stream << _dropbox->token() << endl;
    stream << _dropbox->tokenSecret() << endl;
    stream.flush();
    tokenSaveFile.close();

    return true;
}

bool Downloader::setTokenFromFile()
{
    QFile tokenReadFile("token.txt");
    if(tokenReadFile.exists())
    {
        if(tokenReadFile.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream instream(&tokenReadFile);
            QString token = instream.readLine().trimmed();
            QString secret = instream.readLine().trimmed();
            qDebug() << "using token '" << token << "' for connection";
            if(!token.isEmpty() && !secret.isEmpty())
            {
                _dropbox->setToken(token);
                _dropbox->setTokenSecret(secret);
                tokenReadFile.close();
                return true;
            }
            tokenReadFile.close();
        }
    }

    return false;
}

QStringList Downloader::filterVideos(QString folder)
{
    QStringList videos;

    // get metadata from folder
    QDropboxFile df(folder, _dropbox);
    QDropboxFileInfo info = df.metadata();
    if(!info.isDir()) {
        std::cout << "directory " << folder.toStdString() << " not found" << std::endl;
        emit finished();
        return videos;
    }

    // search for all videos by using the mime type of the folder contents
    QList<QDropboxFileInfo> contentInfo = info.contents();
    QList<QDropboxFileInfo>::iterator it;

    for(it=contentInfo.begin(); it!=contentInfo.end(); ++it) {
        info = *it;
        if(info.mimeType().startsWith("video/")) {
            videos << QString("%1%2").arg(info.root()).arg(info.path());
        }
    }

    return videos;
}

void Downloader::downloadFile(QString file)
{
    std::cout << "downloading... ";

    QDropboxFile dFile(file, _dropbox);
    if(!dFile.open(QIODevice::ReadOnly)) {
        std::cout << "opening remote file failed" << std::endl;
        return;
    }


    QFile lFile(QString("downloaded_%1").arg(file.replace("/", "_")));
    if(!lFile.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        std::cout << "opening local file failed" << std::endl;
        return;
    }

    lFile.write(dFile.readAll());

    std::cout << "ok" << std::endl;
    dFile.close();
    lFile.close();
}

