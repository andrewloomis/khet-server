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

//        auto json = QJsonDocument::fromJson(message).object();
//        auto data = json.value("data").toObject();
//        auto user = data["user"].toString();


//        QJsonObject response;
//        response["user"] = user;
//        if (json.value("request") == "signup")
//        {
//            packetManager.handleSignUp(data);
//            response["response"] = "signup";
//            if (userManager.addNewUser(user,
//                                       data["salt"].toString(),
//                                       data["hash"].toString()))
//            {
//                clients.players[socketIndex].setUsername(user);
//                response["reply"] = "ok";
//            }
//            else {
//                response["reply"] = "no";
//            }
//        }
//        else if (json.value("request") == "login")
//        {
//            response["response"] = "login";
//            if (userManager.authUser(user,
//                                     data["hash"].toString()))
//            {
//                clients.players[socketIndex].setUsername(user);
//                response["reply"] = "ok";
//            }
//            else {
//                response["reply"] = "no";
//            }
//        }
//        else if (json.value("request") == "logout")
//        {
//            response["response"] = "logout";
//            response["reply"] = "ok";
//            userManager.playerNotOnline(user);
//            clients.players[socketIndex].logout();
//            gameManagers.removeAll(getGameManagerForUser(user));
//        }
//        else if (json.value("request") == "salt")
//        {
//            response["response"] = "salt";
//            auto salt = userManager.getSaltFromUser(user);
//            response["reply"] = salt;
//        }
//        else if (json.value("request") == "online_player_query")
//        {
//            response["response"] = "online_player_query";
//            auto players = userManager.getOnlinePlayers(user);
//            QJsonArray playerArray;
//            for (auto& player : players)
//            {
//                playerArray.append(player);
//            }
//            response["reply"] = playerArray;
//        }
//        else if (json.value("request") == "game_request")
//        {
//            response["request"] = "game_invite";
//            response["fromUser"] = user;
//            auto opponent = data.value("opponent").toString();
//            if (opponent == "khetai")
//            {
//                auto gameManager = std::make_shared<GameManager>(user, opponent);
//                gameManagers.append(gameManager);

//                response["response"] = "game_request";
//                response["opponent"] = user;
//                response["reply"] = "ok";
//                userManager.playerNotOnline(user);

//                QJsonObject colorMessage;
//                colorMessage["command"] = "set_color";
//                colorMessage["color"] = gameManager->player1Color() ==
//                        Color::Red ? "red" : "grey";
//                clients.getSocketForPlayer(user)->
//                        sendBinaryMessage(QJsonDocument(colorMessage).toJson());

//                if (gameManager->player2Color() == Color::Grey)
//                {
//                    Move move = gameManager->bestAIMove();
//                    gameManager->executeTurn(static_cast<size_t>(move.pieceIndex),
//                                             move.movedAngle, move.movedPosition);

//                    response["command"] = "your_turn";
//                    QJsonObject opponentTurnInfo;
//                    opponentTurnInfo["piece_index"] = move.pieceIndex;
//                    opponentTurnInfo["piece_angle"] = move.movedAngle;
//                    opponentTurnInfo["x_pos"] = move.movedPosition.x;
//                    opponentTurnInfo["y_pos"] = move.movedPosition.y;
//                    response["opponent_turn_info"] = opponentTurnInfo;
//                    clients.getSocketForPlayer(user)->
//                            sendBinaryMessage(QJsonDocument(response).toJson());
//                }
//            }
//            else {
//                for (int i = 0; i < clients.players.length(); i++)
//                {
//                    if (clients.players[i].getUsername() == opponent)
//                    {
//                        clients.sockets[i]->sendBinaryMessage(QJsonDocument(response).toJson());
//                        return;
//                    }
//                }
//            }
//        }
//        else if (json.value("reply") == "invite_accepted")
//        {
//            auto opponent = data.value("opponent").toString();
//            response["response"] = "game_request";
//            response["opponent"] = user;
//            response["reply"] = "ok";
//            userManager.playerNotOnline(user);
//            userManager.playerNotOnline(opponent);

//            auto gameManager = std::make_shared<GameManager>(user, opponent);
//            gameManagers.append(gameManager);
//            clients.getSocketForPlayer(opponent)->
//                    sendBinaryMessage(QJsonDocument(response).toJson());

//            QJsonObject colorMessage;
//            colorMessage["command"] = "set_color";
//            colorMessage["color"] = gameManager->player1Color() ==
//                    Color::Red ? "red" : "grey";
//            clients.getSocketForPlayer(user)->
//                    sendBinaryMessage(QJsonDocument(colorMessage).toJson());

//            QJsonObject colorMessage2;
//            colorMessage["command"] = "set_color";
//            colorMessage["color"] = gameManager->player2Color() ==
//                    Color::Red ? "red" : "grey";
//            clients.getSocketForPlayer(opponent)->
//                    sendBinaryMessage(QJsonDocument(colorMessage).toJson());
//            return;
//        }
//        else if (json.value("request") == "turn_complete")
//        {
//            auto gameManager = getGameManagerForUser(user);
//            if (gameManager != nullptr)
//            {
//                auto index = data.value("piece_index").toInt();
//                auto angle = data.value("piece_angle").toInt();
//                auto xPos = data.value("x_pos").toInt();
//                auto yPos = data.value("y_pos").toInt();
//                gameManager->executeTurn(static_cast<size_t>(index), angle, Position{xPos, yPos});
//                response["command"] = "your_turn";
//                QJsonObject opponentTurnInfo;

//                if (gameManager->opponentForPlayer(user) == "khetai")
//                {
//                    Move move = gameManager->bestAIMove();
//                    gameManager->executeTurn(static_cast<size_t>(move.pieceIndex),
//                                             move.movedAngle, move.movedPosition);

//                    opponentTurnInfo["piece_index"] = move.pieceIndex;
//                    opponentTurnInfo["piece_angle"] = move.movedAngle;
//                    opponentTurnInfo["x_pos"] = move.movedPosition.x;
//                    opponentTurnInfo["y_pos"] = move.movedPosition.y;
//                    response["opponent_turn_info"] = opponentTurnInfo;
//                    clients.getSocketForPlayer(user)->
//                            sendBinaryMessage(QJsonDocument(response).toJson());
//                }
//                else
//                {
//                    opponentTurnInfo["piece_index"] = index;
//                    opponentTurnInfo["piece_angle"] = angle;
//                    opponentTurnInfo["x_pos"] = xPos;
//                    opponentTurnInfo["y_pos"] = yPos;
//                    response["opponent_turn_info"] = opponentTurnInfo;
//                    clients.getSocketForPlayer(gameManager->opponentForPlayer(user))->
//                            sendBinaryMessage(QJsonDocument(response).toJson());
//                }



//            }
//            return;
//        }
//        else if (json.value("request") == "game_over")
//        {
//            userManager.addGameData(data.value("opponent").toString(), true);
//            userManager.addGameData(data.value("user").toString(), false);
//            gameManagers.removeAll(getGameManagerForUser(user));
//            return;
//        }
//        else if (json.value("request") == "rankings")
//        {
//            response["response"] = "rankings";
//            response["rankings"] = userManager.getRankingsData();
//        }
//        pClient->sendBinaryMessage(QJsonDocument(response).toJson());
    }
}

//std::shared_ptr<GameManager> Server::getGameManagerForUser(QString user)
//{
//    for (auto& gameManager : gameManagers)
//    {
//        if (gameManager->containsUser(user))
//        {
//            return gameManager;
//        }
//    }
//    return nullptr;
//}

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

//void Server::endGame(QString player1, QString player2, Color winner)
//{
//    QJsonObject message;
//    message["command"] = "game_over";
//    message["winner"] = winner == Color::Red ? "red" : "grey";
//    clients.getSocketForPlayer(player1)->
//            sendBinaryMessage(QJsonDocument(message).toJson());
//    clients.getSocketForPlayer(player2)->
//            sendBinaryMessage(QJsonDocument(message).toJson());
//    gameManagers.removeAll(getGameManagerForUser(player1));
//}
