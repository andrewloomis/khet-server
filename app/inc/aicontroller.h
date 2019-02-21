#ifndef AICONTROLLER_H
#define AICONTROLLER_H

#include <khetlib/game.h>
#include <QElapsedTimer>

class AIController
{
public:
    AIController(std::shared_ptr<Game> game);
    Move findBestMove(Color playerColor);
    Move nextMove(int depth, int totalDepth, Color myColor, Color currentTurnColor, Game currentGame,
                  Move minScoreMoveForAI, Move maxScoreMoveForOpponent, std::vector<Move>* startingMoves = nullptr);
    std::vector<Move> generatePossibleMoves(Color playerColor, const Game& game);
    int evaluateBoardState(Color playerColor, const Game& game);

private:
    Game createNewGameNode(Game oldGame, const Move& move);
    Move findBestMoveSingleThreaded(int depth, Color playerColor);
    Move findBestMoveMultiThreaded(int depth, Color playerColor);

    std::shared_ptr<Game> gameState;
    QElapsedTimer computeTimer;

    std::vector<unsigned long long> gameNodeTimes;
    std::vector<unsigned long long> moveGenTimes;
    std::vector<unsigned long long> gameEvalTimes;
    std::atomic<unsigned long> evaluatedMoves = 0;
    std::atomic<unsigned long> skippedMoves = 0;
};

#endif // AICONTROLLER_H
