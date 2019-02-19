#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QString>
#include <QObject>
#include <khetlib/player.h>
#include <khetlib/game.h>
#include <khetlib/khettypes.h>
#include "aicontroller.h"

class GameManager : public QObject
{
    Q_OBJECT
public:
    GameManager(QString player1, QString player2);
    bool containsUser(const QString& username);
    QString opponentForPlayer(const QString& playerName);
    Color player1Color() const { return mPlayer1.pieceColor(); }
    Color player2Color() const { return mPlayer2.pieceColor(); }
    Move bestAIMove() {
        return aicontroller.findBestMove(mPlayer1.getUsername() ==
                                         "khetai" ? mPlayer1.pieceColor() :
                                                    mPlayer2.pieceColor());
    }
    void printPieceLayout() const;
//    void executeTurn(size_t index, int angle, Position pos);
    bool isGameOver() const { return game->isGameOver(); }
    void executeTurn(const Move& move);
    bool isOpponentAI() const { return opponentAI; }

signals:
    void sendEndGame(QString player1, QString player2, Color winner);

private slots:
    void endGame(int index);

private:
    void pickColor();

    Player mPlayer1;
    Player mPlayer2;
    std::shared_ptr<Game> game;
    AIController aicontroller;

    bool opponentAI = false;
};

#endif // GAMEMANAGER_H
