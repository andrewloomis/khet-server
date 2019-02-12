#ifndef SERVER_H
#define SERVER_H

#include "khetlib/player.h"
#include "packetmanager.h"
#include "customtypes.h"

#include <QtWebSockets/QWebSocketServer>
#include <QtCore/QList>
#include <memory>

class Server : public QObject
{
    Q_OBJECT
public:
    Server();

private:
    std::shared_ptr<Network::Connections> clients;

    QWebSocketServer* server;
    PacketManager packetManager;
//    UserManager userManager;
//    QList<std::shared_ptr<GameManager>> gameManagers;
    const quint16 port = 60001;

private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);
//    void endGame(QString player1, QString player2, Color winner);
};

#endif // SERVER_H
