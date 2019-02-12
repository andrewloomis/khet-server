#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <QJsonDocument>
#include <QJsonObject>
#include "usermanager.h"
#include "gamemanager.h"
#include "customtypes.h"

class PacketManager
{
public:
    PacketManager(std::shared_ptr<Network::Connections> connections);
    void handlePacket(const QByteArray& packet, int fromSocketIndex);

    void handleSignUp(const QJsonObject& data, int fromSocketIndex);
    void handleLogin(const QJsonObject& data, int fromSocketIndex);
    void handleLogout(const QJsonObject& data);
    void handleSaltRequest(const QJsonObject& data, int fromSocketIndex);
    void handleOnlinePlayerQuery(const QJsonObject& data);
    void handleGameRequest(const QJsonObject& data);
    void handleInviteAccepted(const QJsonObject& data);
    void handleTurnComplete(const QJsonObject& data);
    void handleGameOver(const QJsonObject& data);
    void handleRankings(const QJsonObject& data);

private:
//    enum class PacketResponse
//    {
//        Login = 0,
//        SignUp,

//    };
    enum class PacketType
    {
        SignUp = 0,
        Login,
        Logout,
        Salt,
        OnlinePlayerQuery,
        GameRequest,
        GameInvite,
        TurnComplete,
        GameOver,
        Rankings,
        InviteAccepted,
        InviteDeclined,
        Error
    };

    QJsonObject makePacket(const QString& user,
                                  const PacketType& type);
    QString extractSenderUsername(const QJsonObject& packet);
    PacketType decodePacketType(const QJsonObject& packet);
    std::shared_ptr<GameManager> getGameManagerForUser(QString user);
    void sendMessage(const QString& toUser,
                     const QJsonObject& message);
    UserManager userManager;
    QList<std::shared_ptr<GameManager>> gameManagers;
    std::shared_ptr<Network::Connections> clients;
};

#endif // PACKETMANAGER_H
