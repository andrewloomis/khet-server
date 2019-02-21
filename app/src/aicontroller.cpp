#include <aicontroller.h>
#include <khetlib/khettypes.h>
#include <algorithm>
#include <QRandomGenerator>
#include <QDebug>
#include <algorithm>
#include <thread>

AIController::AIController(std::shared_ptr<Game> game)
    : gameState(game)
{
}

Move AIController::findBestMove(Color playerColor)
{
    computeTimer.restart();
    int depth = 4;
    auto move = findBestMoveSingleThreaded(depth, playerColor);
    qDebug() << "Multi:";
    qDebug() << "computed best move in" << computeTimer.elapsed()/1000.0f << "sec";
    qDebug() << "evaluated" << evaluatedMoves << "moves, "
             << "skipped about" << skippedMoves << "moves, for a total of about"
             << evaluatedMoves+skippedMoves << "moves\n";

//    computeTimer.restart();
//    auto move2 = findBestMoveSingleThreaded(depth, playerColor);
//    qDebug() << "Single:";
//    qDebug() << "computed best move in" << computeTimer.elapsed()/1000.0f << "sec";
//    qDebug() << "evaluated" << evaluatedMoves << "moves, "
//             << "skipped about" << skippedMoves << "moves, for a total of about"
//             << evaluatedMoves+skippedMoves << "moves\n";
    return move;
}

Move AIController::findBestMoveMultiThreaded(int depth, Color playerColor)
{
    computeTimer.restart();
    auto startingMoves = generatePossibleMoves(playerColor, *gameState);
    std::vector<Move> finalMoves;
    std::atomic_bool isFinalMovesBeingWritten = false;
    std::atomic_bool readingGameState = false;
    finalMoves.reserve(startingMoves.size());

    auto maxThreads = std::thread::hardware_concurrency();
    qDebug() << "starting moves for" << (playerColor == Color::Red ? "red" : "grey")
             << ":" << startingMoves.size();
    qDebug() << "max threads:" << maxThreads;
    std::vector<std::thread> threads;
    std::atomic_int doneCounter = 0;
    for (int movesComplete = 0;
         movesComplete < startingMoves.size();
         movesComplete++)
    {
        auto& move = startingMoves[movesComplete];
        move.track = movesComplete;

        threads.push_back(std::thread([&,move]{
            int track = move.track;
            qDebug() << "starting thread" << track;
            while(readingGameState);
            readingGameState = true;
            Game oldGame = *gameState;
            readingGameState = false;

            Game game = createNewGameNode(oldGame, move);

            auto startingInnerMoves = generatePossibleMoves(game.currentPlayerTurn(), game);
            for (auto& innerMove : startingInnerMoves)
            {
                innerMove.track = track;
            }
            auto finalMove = nextMove(depth-1, depth-1, playerColor, game.currentPlayerTurn(),
                                      game, Move::minInit(), Move::maxInit(), &startingInnerMoves);
            while(isFinalMovesBeingWritten);
            isFinalMovesBeingWritten = true;
            qDebug() << "track:" << finalMove.track << "movetrack:" << track << "value:" << finalMove.value;
            finalMoves.push_back(finalMove);
            isFinalMovesBeingWritten = false;
//            doneCounter++;
        }));
    }
//    while(doneCounter < threads.size())
//    {
//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(100ms);
//    }
    for (auto& thread : threads)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    auto move = std::max_element(finalMoves.begin(), finalMoves.end());
    int bestTrack = move->track;
    Move actual = startingMoves[bestTrack];
    if (actual.movedAngle == -1)
    {
        actual.movedAngle = gameState->getPieceAngle(actual.pieceIndex);
    }
    qDebug() << "FINAL: index:" << actual.pieceIndex
             << "x:" << actual.movedPosition.x << "y:" << actual.movedPosition.y
             << "angle:" << actual.movedAngle
             << "value:" << move->value << "track:" << move->track;
//    qDebug() << "Times:" << "moveGen:"
//             << std::accumulate(moveGenTimes.begin(),
//                                moveGenTimes.end(), 0)/(uint64_t)moveGenTimes.size()
//             << "gameEval:" << std::accumulate(gameEvalTimes.begin(),
//                                               gameEvalTimes.end(), 0)/(uint64_t)gameEvalTimes.size()
//             << "gameNode:" << std::accumulate(gameNodeTimes.begin(),
//                                               gameNodeTimes.end(), 0)/(uint64_t)gameNodeTimes.size();
    return actual;
}

Move AIController::findBestMoveSingleThreaded(int depth, Color playerColor)
{
    computeTimer.restart();
    auto startingMoves = generatePossibleMoves(playerColor, *gameState);

    qDebug() << "starting moves for" << (playerColor == Color::Red ? "red" : "grey") << ":";
    int i = 0;
    for (auto& move : startingMoves)
    {
        move.track = i++;
        qDebug() << "index:" << move.pieceIndex
                 << "x:" << move.movedPosition.x << "y:" << move.movedPosition.y
                 << "angle:" << move.movedAngle << "track:" << move.track;
    }
    Move move = nextMove(depth, depth, playerColor, playerColor, *gameState,
                    Move::minInit(), Move::maxInit(), &startingMoves);
    int bestTrack = move.track;
    Move actual = startingMoves[bestTrack];
    if (actual.movedAngle == -1)
    {
        actual.movedAngle = gameState->getPieceAngle(actual.pieceIndex);
    }
    qDebug() << "FINAL: index:" << actual.pieceIndex
             << "x:" << actual.movedPosition.x << "y:" << actual.movedPosition.y
             << "angle:" << actual.movedAngle
             << "value:" << move.value << "track:" << move.track;
//    qDebug() << "Times:" << "moveGen:"
//             << std::accumulate(moveGenTimes.begin(),
//                                moveGenTimes.end(), 0)/(uint64_t)moveGenTimes.size()
//             << "gameEval:" << std::accumulate(gameEvalTimes.begin(),
//                                               gameEvalTimes.end(), 0)/(uint64_t)gameEvalTimes.size()
//             << "gameNode:" << std::accumulate(gameNodeTimes.begin(),
//                                               gameNodeTimes.end(), 0)/(uint64_t)gameNodeTimes.size();
    return actual;
}

Game AIController::createNewGameNode(Game game, const Move &move)
{
//    computeTimer.restart();
    auto index = static_cast<size_t>(move.pieceIndex);
    Position pos = move.movedPosition;
    game.updatePiecePosition(index, pos.x, pos.y);
    if (move.movedAngle != -1)
    {
        game.updatePieceAngle(index, move.movedAngle);
    }
    game.setLastMove(move);
    game.endTurn();
//    gameNodeIterations++;
//    gameNodeTimes.push_back(computeTimer.nsecsElapsed());
    return game;
}

Move AIController::nextMove(int depth, int totalDepth, Color myColor, Color currentTurnColor,  Game currentGame,
                            Move minScoreMoveForAI, Move maxScoreMoveForOpponent, std::vector<Move>* startingMoves)
{
//    qDebug() << "DEPTH:" << depth;
    if (depth == 0)
    {
        evaluatedMoves++;
        Move move = currentGame.getLastMove();
        int moveValue = evaluateBoardState(myColor, currentGame);
        move.value = moveValue;
        if (move.track == -1)
        {
            qDebug() << "0*****";
            currentGame.printPieceLayout();
            while(1);
        }

        return move;
    }
    std::vector<Move> moves;
    if (depth == totalDepth)
    {
        if (!startingMoves)
        {
            qDebug() << "help";
            while(1);
        }
//        qDebug() << "starting";
        moves = *startingMoves;
    }
    else
    {
        moves = generatePossibleMoves(currentTurnColor, currentGame);
        int currentTrack = currentGame.getLastMove().track;
        if (currentTrack == -1)
        {
            qDebug() << "first*****" << depth;
            currentGame.printPieceLayout();
            while(1);
        }

        for (auto& move : moves)
        {
            move.track = currentTrack;
        }
    }

    // maximizing player
    if (myColor == currentTurnColor)
    {
//        qDebug() << "max";
        Move value = Move::minInit();
        size_t index = 0;
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(currentGame, move);

            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);

            if (newTurn.track == -1)
            {
                qDebug() << "max*****" << depth;
                newGame.printPieceLayout();
                while(1);
            }

            value = newTurn > value ? newTurn : value;
//            qDebug() << "move: index:" << newTurn.pieceIndex
//                     << "x:" << newTurn.movedPosition.x << "y:" << newTurn.movedPosition.y
//                     << "angle:" << newTurn.movedAngle
//                     << "value:" << newTurn.value << "track:" << newTurn.track;

            minScoreMoveForAI = minScoreMoveForAI > value ? minScoreMoveForAI : value;
            index++;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
//                qDebug() << "\nPRUNE\n";
                skippedMoves += pow(moves.size()-index, depth);
                break;
            }
        }
//        qDebug() << "TREE COMPLETE: index:" << value.pieceIndex
//                 << "value:" << value.value;
        return value;
    }
    // minimizing player
    else
    {
//        qDebug() << "min";
        Move value = Move::maxInit();
        size_t index = 0;
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(currentGame, move);

            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);

            if (newTurn.track == -1)
            {
                qDebug() << "min*****" << depth;
                newGame.printPieceLayout();
                while(1);
            }

            value = value < newTurn ? value : newTurn;
//            qDebug() << "move: index:" << newTurn.pieceIndex
//                     << "x:" << newTurn.movedPosition.x << "y:" << newTurn.movedPosition.y
//                     << "angle:" << newTurn.movedAngle
//                     << "value:" << newTurn.value << "track:" << newTurn.track;

            maxScoreMoveForOpponent = maxScoreMoveForOpponent < value ?
                        maxScoreMoveForOpponent : value;
            index++;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
//                qDebug() << "\nPRUNE\n";
                skippedMoves += pow(moves.size()-index, depth);
                break;
            }
        }
//        qDebug() << "TREE COMPLETE: index:" << value.pieceIndex
//                 << "value:" << value.value;
        return value;
    }
}

int AIController::evaluateBoardState(Color playerColor, const Game& game)
{
//    computeTimer.restart();
    int score = 0;
    auto& pieces = game.getPieces();
    for (auto& piece : pieces)
    {
        if (!piece->isKilled())
        {
            if (piece->color() == playerColor)
            {
                score += piece->value();
            }
            else
            {
                score -= piece->value();
            }
        }
    }
    int variant = QRandomGenerator::global()->bounded(-5000,5000);
//    gameEvalTimes.push_back(computeTimer.nsecsElapsed());
    return score + variant;
}

std::vector<Move> AIController::generatePossibleMoves(Color playerColor, const Game& game)
{
//    computeTimer.restart();
    const auto& pieces = game.getPieces();
    std::vector<Move> moves;
    moves.reserve(80);
    for (auto& piece : pieces)
    {
        if (!piece->isKilled() && piece->color() == playerColor)
        {
            auto translations = game.possibleTranslationsForPiece(static_cast<size_t>(piece->index()));
            Position currentPos = piece->position();

            if (translations & Translations::Top)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x, currentPos.y-1}, piece->angle()));
            }
            if (translations & Translations::TopLeft)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y-1}, piece->angle()));
            }
            if (translations & Translations::TopRight)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y-1}, piece->angle()));
            }
            if (translations & Translations::Left)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y}, piece->angle()));
            }
            if (translations & Translations::Right)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y}, piece->angle()));
            }
            if (translations & Translations::Bottom)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x, currentPos.y+1}, piece->angle()));
            }
            if (translations & Translations::BottomLeft)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y+1}, piece->angle()));
            }
            if (translations & Translations::BottomRight)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y+1}, piece->angle()));
            }
            if (piece->type() != PieceType::Pharoah &&
                    piece->type() != PieceType::Obelisk)
            {
                moves.push_back(Move(piece->index(), piece->position(), (piece->angle() == 360) ? 90 : piece->angle() + 90));
                moves.push_back(Move(piece->index(), piece->position(),
                                     (piece->angle() == 0) ? 270 : piece->angle() - 90));
            }
        }
    }
//    moveGenTimes.push_back(computeTimer.nsecsElapsed());
    return moves;
}

