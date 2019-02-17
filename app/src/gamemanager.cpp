#include "gamemanager.h"
#include <QRandomGenerator>
#include <QDebug>

GameManager::GameManager(QString player1, QString player2)
    : mPlayer1{player1}, mPlayer2{player2},
      game(std::make_shared<Game>()), aicontroller(game)
{
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
//    qDebug() << "*index:" << move.pieceIndex
//             << "*value:" << move.value;
//    executeTurn(static_cast<size_t>(move.pieceIndex),
//                move.movedAngle, move.movedPosition);
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
//        auto turn = game->currentPlayerTurn();
//        game->calculateBeamCoords(turn == Color::Red ? 0 : 9,
//                                 turn == Color::Red ? 0 : 7);
//        game->nextTurn();
    }
    else
    {
        qDebug() << "index or angle out of range";
    }
}

//void GameManager::executeTurn(size_t index, int angle, Position pos)
//{
//    if (index < 30 && angle != -1)
//    {
//        game->updatePieceAngle(index, angle);
//        game->updatePiecePosition(index, pos.x, pos.y);
//        auto turn = game->currentPlayerTurn();
//        game->calculateBeamCoords(turn == Color::Red ? 0 : 9,
//                                 turn == Color::Red ? 0 : 7);
//        game->nextTurn();
//    }
//    else
//    {
//        qDebug() << "index or angle out of range";
//    }
//}

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
