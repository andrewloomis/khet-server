#include <packetmanager.h>
#include <QWebSocket>
#include <QJsonArray>

PacketManager::PacketManager(std::shared_ptr<Network::Connections> connections)
    : clients(connections)
{}

void PacketManager::handlePacket(const QByteArray &packet, int fromSocketIndex)
{
    auto json = QJsonDocument::fromJson(packet).object();
    auto data = json.value("data").toObject();
    auto packetType = decodePacketType(json);

    switch (packetType) {
    case PacketType::SignUp:
        handleSignUp(data, fromSocketIndex);
        break;
    case PacketType::Login:
        handleLogin(data, fromSocketIndex);
        break;
    case PacketType::Logout:
        handleLogout(data);
        break;
    case PacketType::Salt:
        handleSaltRequest(data, fromSocketIndex);
        break;
    case PacketType::OnlinePlayerQuery:
        handleOnlinePlayerQuery(data);
        break;
    case PacketType::GameRequest:
        handleGameRequest(data);
        break;
    case PacketType::InviteAccepted:
        handleInviteAccepted(data);
        break;
    case PacketType::TurnComplete:
        handleTurnComplete(data);
        break;
    case PacketType::GameOver:
        handleGameOver(data);
        break;
    case PacketType::Rankings:
        handleRankings(data);
        break;
    }
}

void PacketManager::handleSignUp(const QJsonObject& data, int fromSocketIndex)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::SignUp);

    if (userManager.addNewUser(user,
                               data["salt"].toString(),
                               data["hash"].toString()))
    {
        clients->players[fromSocketIndex].setUsername(user);
        response["reply"] = "ok";
    }
    else {
        response["reply"] = "no";
    }
    sendMessage(user, response);
}

void PacketManager::handleLogin(const QJsonObject &data, int fromSocketIndex)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::Login);

    if (userManager.authUser(user,
                             data["hash"].toString()))
    {
        clients->players[fromSocketIndex].setUsername(user);
        response["reply"] = "ok";
    }
    else {
        response["reply"] = "no";
    }
    sendMessage(user, response);
}

void PacketManager::handleLogout(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);
//    auto response = makePacket(user, PacketType::Logout);
//    sendMessage(user, response);

    userManager.playerNotOnline(user);
    auto player = clients->getPlayer(user);
    if (player)
    {
        player->logout();
    }
    gameManagers.removeAll(getGameManagerForUser(user));
}

void PacketManager::handleSaltRequest(const QJsonObject &data, int fromSocketIndex)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::Salt);

    auto salt = userManager.getSaltFromUser(user);
    response["reply"] = salt;
    clients->sockets[fromSocketIndex]->
            sendBinaryMessage(QJsonDocument(response).toJson());
//    sendMessage(user, response);
}

void PacketManager::handleOnlinePlayerQuery(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::OnlinePlayerQuery);

    auto players = userManager.getOnlinePlayers(user);
    QJsonArray playerArray;
    for (const auto& player : players)
    {
        playerArray.append(player);
    }
    response["reply"] = playerArray;
    sendMessage(user, response);
}

void PacketManager::handleGameRequest(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::GameInvite);

    auto opponent = data.value("opponent").toString();
    auto config = data.value("config").toString();
    response["config"] = config;
    if (opponent == "khetai")
    {
        response["fromUser"] = "khetai";
        sendMessage(user, response);
    }
    else
    {
        response["fromUser"] = user;
        sendMessage(opponent, response);
    }
}

void PacketManager::handleInviteAccepted(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);
    auto opponent = data.value("opponent").toString();
    gameManagers.removeAll(getGameManagerForUser(user));
    gameManagers.removeAll(getGameManagerForUser(opponent));
    userManager.playerNotOnline(user);
    if (opponent != "khetai") userManager.playerNotOnline(opponent);

    auto configStr = data.value("config").toString();
    GameConfig config;
    if (configStr == "imhotep")
    {
        config = GameConfig::Imhotep;
    }
    else if (configStr == "dynasty")
    {
        config = GameConfig::Dynasty;
    }
    else {
        config = GameConfig::Classic;
    }
    auto gameManager = std::make_shared<GameManager>(user, opponent, config);
    gameManagers.append(gameManager);

    if (opponent == "khetai")
    {
        QJsonObject colorMessage;
        colorMessage["command"] = "set_color";
        colorMessage["color"] = gameManager->player1Color() ==
                Color::Red ? "red" : "grey";
        sendMessage(user, colorMessage);

        // khetai goes first
        if (gameManager->player2Color() == Color::Grey)
        {
            Move move = gameManager->bestAIMove();
            gameManager->executeTurn(move);

            auto response = makePacket("khetai", PacketType::TurnComplete);

            QJsonObject opponentTurnInfo;
            opponentTurnInfo["piece_index"] = move.pieceIndex;
            opponentTurnInfo["piece_angle"] = move.movedAngle;
            opponentTurnInfo["x_pos"] = move.movedPosition.x;
            opponentTurnInfo["y_pos"] = move.movedPosition.y;
            response["opponent_turn_info"] = opponentTurnInfo;
            sendMessage(user, response);
        }
    }
    else
    {
        auto response = makePacket(user, PacketType::GameRequest);

        response["opponent"] = user;
        response["reply"] = "ok";

        sendMessage(opponent, response);

        QJsonObject colorMessage;
        colorMessage["command"] = "set_color";
        colorMessage["color"] = gameManager->player1Color() ==
                Color::Red ? "red" : "grey";
        sendMessage(user, colorMessage);

        QJsonObject colorMessage2;
        colorMessage2["command"] = "set_color";
        colorMessage2["color"] = gameManager->player2Color() ==
                Color::Red ? "red" : "grey";
        sendMessage(opponent, colorMessage2);
    }
}

void PacketManager::handleTurnComplete(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);

    auto gameManager = getGameManagerForUser(user);
    qDebug() << "test";
    gameManager->printPieceLayout();
    if (gameManager != nullptr)
    {
        auto index = data.value("piece_index").toInt();
        auto angle = data.value("piece_angle").toInt();
        auto xPos = data.value("x_pos").toInt();
        auto yPos = data.value("y_pos").toInt();
        gameManager->executeTurn(Move{index, Position{xPos, yPos}, angle});
        gameManager->printPieceLayout();

        QJsonObject opponentTurnInfo;

        if (gameManager->opponentForPlayer(user) == "khetai" && !gameManager->isGameOver())
        {
            auto response = makePacket("khetai", PacketType::TurnComplete);

            Move move = gameManager->bestAIMove();
            gameManager->executeTurn(move);
            gameManager->printPieceLayout();
//            gameManager->executeTurn(static_cast<size_t>(move.pieceIndex),
//                                     move.movedAngle, move.movedPosition);

            opponentTurnInfo["piece_index"] = move.pieceIndex;
            opponentTurnInfo["piece_angle"] = move.movedAngle;
            opponentTurnInfo["x_pos"] = move.movedPosition.x;
            opponentTurnInfo["y_pos"] = move.movedPosition.y;
            response["opponent_turn_info"] = opponentTurnInfo;
            sendMessage(user, response);
        }
        else
        {
            auto response = makePacket(user, PacketType::TurnComplete);
            opponentTurnInfo["piece_index"] = index;
            opponentTurnInfo["piece_angle"] = angle;
            opponentTurnInfo["x_pos"] = xPos;
            opponentTurnInfo["y_pos"] = yPos;
            response["opponent_turn_info"] = opponentTurnInfo;
            sendMessage(gameManager->opponentForPlayer(user), response);
        }
    }
    else
    {
        qDebug() << "gameManager not found";
    }
}

void PacketManager::handleGameOver(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);

    userManager.addGameData(data.value("opponent").toString(), true);
    userManager.addGameData(data.value("user").toString(), false);
    gameManagers.removeAll(getGameManagerForUser(user));
}

void PacketManager::handleRankings(const QJsonObject &data)
{
    auto user = extractSenderUsername(data);
    auto response = makePacket(user, PacketType::Rankings);

    response["rankings"] = userManager.getRankingsData();
    sendMessage(user, response);
}

QString PacketManager::extractSenderUsername(const QJsonObject &packet)
{
    return packet.value("user").toString();
}

QJsonObject PacketManager::makePacket(const QString &user, const PacketType &type)
{
    QJsonObject packet;
    switch (type)
    {
    case PacketType::SignUp:
        packet["response"] = "signup";
        break;
    case PacketType::Login:
        packet["response"] = "login";
        break;
    case PacketType::Logout:
        packet["response"] = "logout";
        packet["reply"] = "ok";
        break;
    case PacketType::Salt:
        packet["response"] = "salt";
        break;
    case PacketType::OnlinePlayerQuery:
        packet["response"] = "online_player_query";
        break;
    case PacketType::GameRequest:
        packet["response"] = "game_request";
        break;
    case PacketType::GameInvite:
        packet["request"] = "game_invite";
        break;
    case PacketType::TurnComplete:
        packet["command"] = "your_turn";
        break;
    case PacketType::Rankings:
        packet["response"] = "rankings";
        break;
    }
    packet["user"] = user;
    return packet;
}

void PacketManager::sendMessage(const QString &toUser,
                                const QJsonObject &message)
{
    auto socket = clients->getSocketForPlayer(toUser);
    if (socket)
    {
        socket->sendBinaryMessage(QJsonDocument(message).toJson());
    }
}

PacketManager::PacketType PacketManager::decodePacketType(const QJsonObject &packet)
{
    auto request = packet.value("request").toString();
    if (request == "signup")
    {
        return PacketType::SignUp;
    }
    else if (request == "login")
    {
        return PacketType::Login;
    }
    else if (request == "logout")
    {
        return PacketType::Logout;
    }
    else if (request == "salt")
    {
        return PacketType::Salt;
    }
    else if (request == "online_player_query")
    {
        return PacketType::OnlinePlayerQuery;
    }
    else if (request == "game_request")
    {
        return PacketType::GameRequest;
    }
    else if (request == "turn_complete")
    {
        return PacketType::TurnComplete;
    }
    else if (request == "game_over")
    {
        return PacketType::GameOver;
    }
    else if (request == "rankings")
    {
        return PacketType::Rankings;
    }

    auto reply = packet.value("reply").toString();
    if (reply == "invite_accepted")
    {
        return PacketType::InviteAccepted;
    }
    return PacketType::Error;
}

std::shared_ptr<GameManager> PacketManager::getGameManagerForUser(QString user)
{
    for (auto& gameManager : gameManagers)
    {
        if (gameManager->containsUser(user))
        {
            return gameManager;
        }
    }
    return nullptr;
}
