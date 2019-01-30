#ifndef GAMEMANAGER_H
#define GAMEMANAGER_H

#include <QString>
#include <QObject>
#include <khetlib/player.h>
#include <khetlib/game.h>
#include <khetlib/khettypes.h>

class GameManager : public QObject
{
    Q_OBJECT
public:
    GameManager(QString player1, QString player2);
    bool containsUser(const QString& username);
    QString opponentForPlayer(const QString& playerName);
    Color player1Color() const { return mPlayer1.pieceColor(); }
    Color player2Color() const { return mPlayer2.pieceColor(); }
    bool syncGame(const Game& otherGame);
    void executeTurn(size_t index, int angle, Position pos);

signals:
    void sendEndGame(QString player1, QString player2, Color winner);

private slots:
    void endGame(int index);

private:
    void pickColor();

    Player mPlayer1;
    Player mPlayer2;
    Game game;
};

#endif // GAMEMANAGER_H
