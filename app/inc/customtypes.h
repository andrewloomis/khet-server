#ifndef CUSTOMTYPES_H
#define CUSTOMTYPES_H

#include "khetlib/player.h"
#include <QWebSocketServer>
#include <QList>
#include <memory>

namespace Network {
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
    };
}

#endif // CUSTOMTYPES_H
