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
    : server(new QWebSocketServer("Khet Server", QWebSocketServer::SecureMode, this))
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

    m_clients << pSocket;
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
        auto json = QJsonDocument::fromJson(message).object();
        auto data = json.value("data").toObject();

        QJsonObject response;
        response["user"] = data["user"].toString();
        if (json.value("request") == "signup")
        {
            response["response"] = "signup";
            if (userManager.addNewUser(data["user"].toString(),
                                       data["salt"].toString(),
                                       data["hash"].toString()))
            {
                response["reply"] = "ok";
            }
            else {
                response["reply"] = "no";
            }
        }
        else if (json.value("request") == "login")
        {
            response["response"] = "login";
            if (userManager.authUser(data["user"].toString(),
                                     data["hash"].toString()))
            {
                response["reply"] = "ok";
            }
            else {
                response["reply"] = "no";
            }
        }
        else if (json.value("request") == "logout")
        {
            response["response"] = "logout";
            response["reply"] = "ok";
            userManager.playerNotOnline(data["user"].toString());
        }
        else if (json.value("request") == "salt")
        {
            response["response"] = "salt";
            auto salt = userManager.getSaltFromUser(data["user"].toString());
            response["reply"] = salt;
        }
        else if (json.value("request") == "online_player_query")
        {
            response["response"] = "online_player_query";
            auto players = userManager.getOnlinePlayers(data["user"].toString());
            QJsonArray playerArray;
            for (auto& player : players)
            {
                playerArray.append(player);
            }
            response["reply"] = playerArray;
        }
        pClient->sendBinaryMessage(QJsonDocument(response).toJson());
    }
}

void Server::socketDisconnected()
{
    qDebug() << "Client disconnected";
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        m_clients.removeAll(pClient);
        pClient->deleteLater();
    }
}

void Server::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl errors occurred";
}
