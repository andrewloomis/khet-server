#ifndef SERVER_H
#define SERVER_H

#include "usermanager.h"
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
    QWebSocketServer* server;
    UserManager userManager;
    QList<QWebSocket *> m_clients;
    const quint16 port = 60001;

private Q_SLOTS:
    void onNewConnection();
    void processTextMessage(QString message);
    void processBinaryMessage(QByteArray message);
    void socketDisconnected();
    void onSslErrors(const QList<QSslError> &errors);

};

#endif // SERVER_H
