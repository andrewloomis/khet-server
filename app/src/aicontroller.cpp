#include <aicontroller.h>
#include <khetlib/khettypes.h>
#include <algorithm>
#include <QRandomGenerator>
#include <QDebug>

AIController::AIController(std::shared_ptr<Game> game)
    : gameState(game)
{
}

Move AIController::findBestMove(Color playerColor)
{
//    computeTimer.start();
    auto startingMoves = generatePossibleMoves(playerColor, *gameState);
    int i = 0;
    qDebug() << "starting moves for" << (playerColor == Color::Red ? "red" : "grey") << ":";
    for (auto& move : startingMoves)
    {
        move.track = i++;
        qDebug() << "index:" << move.pieceIndex
                 << "x:" << move.movedPosition.x << "y:" << move.movedPosition.y
                 << "angle:" << move.movedAngle << "track:" << move.track;
    }
    int depth = 4;
    Move move = nextMove(depth, depth, playerColor, playerColor, *gameState,
                    Move::minInit(), Move::maxInit(), &startingMoves);
//    Move actual = startingMoves[QRandomGenerator::global()->bounded(0,startingMoves.size()-1)];
    Move actual = startingMoves[move.track];
    if (actual.movedAngle == -1)
    {
        actual.movedAngle = gameState->getPieceAngle(actual.pieceIndex);
    }
    qDebug() << "FINAL: index:" << actual.pieceIndex
             << "x:" << actual.movedPosition.x << "y:" << actual.movedPosition.y
             << "angle:" << actual.movedAngle
             << "value:" << move.value << "track:" << move.track;
    return actual;
}

Game AIController::createNewGameNode(Game game, const Move &move) const
{
    auto index = static_cast<size_t>(move.pieceIndex);
    Position pos = move.movedPosition;
    game.updatePiecePosition(index, pos.x, pos.y);
    if (move.movedAngle != -1)
    {
        game.updatePieceAngle(index, move.movedAngle);
    }
    game.setLastMove(move);
    game.endTurn();
    return game;
}

Move AIController::nextMove(int depth, int totalDepth, Color myColor, Color currentTurnColor,  Game currentGame,
                            Move minScoreMoveForAI, Move maxScoreMoveForOpponent, std::vector<Move>* startingMoves)
{
//    qDebug() << "DEPTH:" << depth;
    if (depth == 0)
    {
        Move move = currentGame.getLastMove();
        int moveValue = evaluateBoardState(myColor, currentGame);
        move.value = moveValue;
        return move;
    }
    std::vector<Move> moves;
    if (depth == totalDepth)
    {
        qDebug() << "starting";
        moves = *startingMoves;
    }
    else
    {
        moves = generatePossibleMoves(currentTurnColor, currentGame);
        int currentTrack = currentGame.getLastMove().track;
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
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(currentGame, move);

            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);

            value = newTurn > value ? newTurn : value;
//            qDebug() << "move: index:" << newTurn.pieceIndex
//                     << "x:" << newTurn.movedPosition.x << "y:" << newTurn.movedPosition.y
//                     << "angle:" << newTurn.movedAngle
//                     << "value:" << newTurn.value << "track:" << newTurn.track;

            minScoreMoveForAI = minScoreMoveForAI > value ? minScoreMoveForAI : value;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
//                qDebug() << "\nPRUNE\n";
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
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(currentGame, move);

            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);

            value = value < newTurn ? value : newTurn;
//            qDebug() << "move: index:" << newTurn.pieceIndex
//                     << "x:" << newTurn.movedPosition.x << "y:" << newTurn.movedPosition.y
//                     << "angle:" << newTurn.movedAngle
//                     << "value:" << newTurn.value << "track:" << newTurn.track;

            maxScoreMoveForOpponent = maxScoreMoveForOpponent < value ?
                        maxScoreMoveForOpponent : value;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
//                qDebug() << "\nPRUNE\n";
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
    return score + variant;
}

std::vector<Move> AIController::generatePossibleMoves(Color playerColor, const Game& game)
{
    const auto& pieces = game.getPieces();
    std::vector<Move> moves;
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
//    std::vector<Move> temp;
//    temp.push_back(moves[0]);
//    temp.push_back(moves[1]);
//    temp.push_back(moves[2]);
    return moves;
}

