#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkRequest>

#include <QtCore/QtDebug>

#include "httpserver.hpp"
#include "qtael.hpp"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    auto b = new qtael::Async([](const qtael::Await &await) -> void {
        qDebug() << "wait for http server open ...";
        // NOTE yield to main event loop until 1000ms passed
        await(1000);

        qDebug() << "make request";
        QNetworkAccessManager nasm;
        QNetworkRequest r(QUrl("http://localhost:1080/"));
        auto reply = nasm.get(r);
        // NOTE yield to main event loop until request finished
        await(reply, &QNetworkReply::finished);
        auto c = reply->readAll();
        qDebug() << c;
    });
    // NOTE when coroutine finished (i.e. reaches end or return), `finished()`
    // emitted
    b->connect(b, SIGNAL(finished()), SLOT(deleteLater()));
    a.connect(b, SIGNAL(finished()), SLOT(quit()));
    b->start();

    HttpServer server;
    bool ok = server.listen(1080);
    qDebug() << "http server ok:" << ok;

    return a.exec();
}
