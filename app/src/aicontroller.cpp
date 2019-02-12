#include <aicontroller.h>
#include <khetlib/khettypes.h>
#include <algorithm>
#include <QRandomGenerator>

AIController::AIController(std::shared_ptr<Game> game)
    : gameState(game)
{}

Move AIController::findBestMove(Color playerColor)
{
//    computeTimer.start();
    return nextMove(5, playerColor, playerColor, *gameState,
                    Move::minInit(), Move::maxInit());
//    if (computeTimer.elapsed() > 2000)
//    {
//        return minScoreMoveForAI;
//    }
}

//Game AIController::nextTurn(Color playerColor, Color playerTurn, Game currentGame)
Move AIController::nextMove(int depth, Color myColor, Color currentTurnColor,  Game currentGame,
                            Move minScoreMoveForAI, Move maxScoreMoveForOpponent)
{
//    Move minScoreMoveForAI = alpha;
//    Move maxScoreMoveForOpponent = beta;
    if (depth == 0)
    {
        return currentGame.getLastMove();
    }
    auto moves = generatePossibleMoves(currentTurnColor);
    auto game = currentGame;

    // maximizing player
    if (myColor == currentTurnColor)
    {
        Move value = Move::minInit();
        for (auto& move : moves)
        {
            auto newGame = game;    // generate new node
            auto index = static_cast<size_t>(move.pieceIndex);
            if (move.movedAngle == -1)
            {
                Position pos = move.movedPosition;
                newGame.updatePiecePosition(index, pos.x, pos.y);
            }
            else
            {
                newGame.updatePieceAngle(index, move.movedAngle);
            }
            newGame.calculateBeamCoords(myColor == Color::Red ? 9 : 0,
                                        myColor == Color::Red ? 7 : 0);
            Color nextColorTurn = currentTurnColor == Color::Red ? Color::Grey : Color::Red;
            auto newTurn = nextMove(depth-1, myColor, nextColorTurn,
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);
            int newTurnValue = evaluateBoardState(myColor, newGame);
            newTurn.value = newTurnValue;

            value = newTurn > value ? newTurn : value;
            minScoreMoveForAI = minScoreMoveForAI > value ? minScoreMoveForAI : value;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
                break;
            }
        }
        return value;
    }
    // minimizing player
    else
    {
        Move value = Move::maxInit();
        for (auto& move : moves)
        {
            auto newGame = game;    // generate new node
            auto index = static_cast<size_t>(move.pieceIndex);
            if (move.movedAngle == -1)
            {
                Position pos = move.movedPosition;
                newGame.updatePiecePosition(index, pos.x, pos.y);
            }
            else
            {
                newGame.updatePieceAngle(index, move.movedAngle);
            }
            newGame.calculateBeamCoords(myColor == Color::Red ? 9 : 0,
                                        myColor == Color::Red ? 7 : 0);
            Color nextColorTurn = currentTurnColor == Color::Red ? Color::Grey : Color::Red;
            auto newTurn = nextMove(depth-1, myColor, nextColorTurn,
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);
            int newTurnValue = evaluateBoardState(myColor, newGame);
            newTurn.value = newTurnValue;

            value = value < newTurn ? value : newTurn;
            maxScoreMoveForOpponent = maxScoreMoveForOpponent < value ?
                        maxScoreMoveForOpponent : value;
            if (minScoreMoveForAI >= maxScoreMoveForOpponent)
            {
                break;
            }
        }
        return value;
    }
}

int AIController::evaluateBoardState(Color playerColor, const Game& game)
{
    int score = 0;
    auto pieces = game.getPieces();
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

std::vector<Move> AIController::generatePossibleMoves(Color playerColor)
{
    const auto pieces = gameState->getPieces();
    std::vector<Move> moves;
    for (auto& piece : pieces)
    {
        if (!piece->isKilled() && piece->color() == playerColor)
        {
            auto translations = gameState->
                    possibleTranslationsForPiece(static_cast<size_t>(piece->index()));
            Position currentPos = piece->position();

            if (translations & Translations::Top)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x, currentPos.y-1}));
            }
            if (translations & Translations::TopLeft)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y-1}));
            }
            if (translations & Translations::TopRight)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y-1}));
            }
            if (translations & Translations::Left)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y}));
            }
            if (translations & Translations::Right)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y}));
            }
            if (translations & Translations::Bottom)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x, currentPos.y+1}));
            }
            if (translations & Translations::BottomLeft)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x-1, currentPos.y+1}));
            }
            if (translations & Translations::BottomRight)
            {
                moves.push_back(Move(piece->index(), Position{currentPos.x+1, currentPos.y+1}));
            }
            moves.push_back(Move(piece->index(), piece->position(), piece->angle() + 90));
            moves.push_back(Move(piece->index(), piece->position(), piece->angle() - 90));
        }
    }
    return moves;
}

