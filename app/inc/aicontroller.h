#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <khetlib/game.h>
#include <QElapsedTimer>

class AIController
{
public:
    AIController(std::shared_ptr<Game> game);
    Move findBestMove(Color playerColor);
    Move nextMove(int depth, Color myColor, Color currentTurnColor, Game currentGame,
                  Move minScoreMoveForAI, Move maxScoreMoveForOpponent);
//    Game nextTurn(Color playerColor, Color playerTurn, Game currentGame);
    std::vector<Move> generatePossibleMoves(Color playerColor);
    int evaluateBoardState(Color playerColor, const Game& game);

private:
    std::shared_ptr<Game> gameState;
    QElapsedTimer computeTimer;
};

#endif // AICONTROLLER_H
