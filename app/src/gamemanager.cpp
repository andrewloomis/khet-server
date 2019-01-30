#include "gamemanager.h"
#include <QRandomGenerator>
#include <QDebug>

GameManager::GameManager(QString player1, QString player2)
    : mPlayer1{player1}, mPlayer2{player2}
{
    pickColor();
//    connect(&game, &Game::pharoahKilled, this, &GameManager::endGame);
}

void GameManager::pickColor()
{
    if (QRandomGenerator().bounded(1000000) > 500000)
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
    qDebug() << "player1:" << mPlayer1.getUsername()
             << "player2:" << mPlayer2.getUsername();
    return (mPlayer1.getUsername() == username ||
            mPlayer2.getUsername() == username);
}

bool GameManager::syncGame(const Game &otherGame)
{
    return game == otherGame;
}

void GameManager::executeTurn(size_t index, int angle, Position pos)
{
    game.updatePieceAngle(index, angle);
    game.updatePiecePosition(index, pos.x, pos.y);
    auto turn = game.currentPlayerTurn();
    game.calculateBeamCoords(turn == Color::Red ? 0 : 9,
                             turn == Color::Red ? 0 : 7);
    game.nextTurn();
}

QString GameManager::opponentForPlayer(const QString &playerName)
{

    return mPlayer1.getUsername() == playerName ?
                mPlayer2.getUsername() : mPlayer1.getUsername();
}

void GameManager::endGame(int index)
{
    auto winner = game.getPieceColor(static_cast<size_t>(index)) ==
            Color::Red ? Color::Grey : Color::Red;
    emit sendEndGame(mPlayer1.getUsername(), mPlayer2.getUsername(), winner);
}
