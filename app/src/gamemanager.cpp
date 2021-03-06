#include "gamemanager.h"
#include <QRandomGenerator>
#include <QDebug>

GameManager::GameManager(QString player1, QString player2, GameConfig config)
    : mPlayer1{player1}, mPlayer2{player2},
      game(std::make_shared<Game>()), aicontroller(game)
{
    game->startGame(config);
    pickColor();
    if (player2 == "khetai")
    {
        opponentAI = true;
    }
}

void GameManager::pickColor()
{
    if (QRandomGenerator::global()->bounded(1000000) > 500000)
    {
        mPlayer1.setColor(Color::Grey);
        mPlayer2.setColor(Color::Red);
    }
    else
    {
        mPlayer1.setColor(Color::Red);
        mPlayer2.setColor(Color::Grey);
    }
}

bool GameManager::containsUser(const QString& username)
{
    return (mPlayer1.getUsername() == username ||
            mPlayer2.getUsername() == username);
}

void GameManager::printPieceLayout() const
{
    game->printPieceLayout();
}

void GameManager::executeTurn(const Move& move)
{
    if (move.pieceIndex < 30)
    {
        auto pos = move.movedPosition;
        game->updatePiecePosition(move.pieceIndex, pos.x, pos.y);
        if (move.movedAngle != -1)
        {
            game->updatePieceAngle(move.pieceIndex, move.movedAngle);
        }
        game->setLastMove(move);
        game->endTurn();
    }
    else
    {
        qDebug() << "index or angle out of range";
    }
}

QString GameManager::opponentForPlayer(const QString &playerName)
{
    return mPlayer1.getUsername() == playerName ?
                mPlayer2.getUsername() : mPlayer1.getUsername();
}

void GameManager::endGame(int index)
{
    auto winner = game->getPieceColor(static_cast<size_t>(index)) ==
            Color::Red ? Color::Grey : Color::Red;
    emit sendEndGame(mPlayer1.getUsername(), mPlayer2.getUsername(), winner);
}
