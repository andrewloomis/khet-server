#include "server.h"
#include "QtWebSockets/QWebSocket"
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Server::Server()
    : clients(std::make_shared<Network::Connections>()),
      server(new QWebSocketServer("Khet Server", QWebSocketServer::SecureMode, this)),
      packetManager(clients)
{
    QSslConfiguration sslConfiguration;
    QFile certFile(":/server.crt");
    QFile keyFile(":/server.key");
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);
    server->setSslConfiguration(sslConfiguration);

    if (server->listen(QHostAddress::Any, port))
    {
        qDebug() << "SSL Server listening on port" << port;
        connect(server, &QWebSocketServer::newConnection,
                this, &Server::onNewConnection);
        connect(server, &QWebSocketServer::sslErrors,
                this, &Server::onSslErrors);
    }
}

void Server::onNewConnection()
{
    QWebSocket *pSocket = server->nextPendingConnection();

    qDebug() << "Client connected:" << pSocket->peerName() << pSocket->origin();

    connect(pSocket, &QWebSocket::textMessageReceived, this, &Server::processTextMessage);
    connect(pSocket, &QWebSocket::binaryMessageReceived,
            this, &Server::processBinaryMessage);
    connect(pSocket, &QWebSocket::disconnected, this, &Server::socketDisconnected);

    clients->sockets << pSocket;
    clients->players << Player{};
}

void Server::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        pClient->sendTextMessage(message);
    }
}

void Server::processBinaryMessage(QByteArray message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        qDebug() << "Message Received:" << message;
        int socketIndex = clients->sockets.indexOf(pClient);
        packetManager.handlePacket(message, socketIndex);
    }
}

void Server::socketDisconnected()
{
    qDebug() << "Client disconnected";
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        auto socketIndex = clients->sockets.indexOf(pClient);
        clients->sockets.removeAll(pClient);
        clients->players.removeAll(clients->players[socketIndex]);
        pClient->deleteLater();
    }
}

void Server::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl errors occurred";
}
