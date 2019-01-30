#ifndef SERVER_H
#define SERVER_H

#include "usermanager.h"
#include "gamemanager.h"
#include "khetlib/player.h"
#include <QtWebSockets/QWebSocketServer>
#include <QtCore/QList>
#include <memory>

class Server : public QObject
{
    Q_OBJECT
public:
    Server();

private:
    struct Connections {
        QList<QWebSocket*> sockets;
        QList<Player> players;
        Player* getPlayer(const QString& name)
        {
            for (int i = 0; i < players.length(); i++)
            {
                if (players[i].getUsername() == name)
                {
                    return &players[i];
                }
            }
            return nullptr;
        }
        QWebSocket* getSocketForPlayer(const QString& name)
        {
            for (int i = 0; i < players.length(); i++)
            {
                if (players[i].getUsername() == name)
                {
                    return sockets[i];
                }
            }
            return nullptr;
        }
    } clients;

    std::shared_ptr<GameManager> getGameManagerForUser(QString user);

    QWebSocketServer* server;
    UserManager userManager;
    QList<std::shared_ptr<GameManager>> gameManagers;
//    QList<QWebSocket *> m_clients;
    const quint16 port = 60001;

private slots:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);
    void endGame(QString player1, QString player2, Color winner);
};

#endif // SERVER_H
