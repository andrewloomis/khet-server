#include <aicontroller.h>
#include <khetlib/khettypes.h>
#include <algorithm>
#include <QRandomGenerator>
#include <QDebug>

AIController::AIController(std::shared_ptr<Game> game)
    : gameState(game)
{}

Move AIController::findBestMove(Color playerColor)
{
//    computeTimer.start();
//    gameState->printPieceLayout();
    auto startingMoves = generatePossibleMoves(playerColor, *gameState);
    int i = 0;
//    qDebug() << "starting moves:";
    for (auto& move : startingMoves)
    {
        move.track = i++;
//        qDebug() << "index:" << move.pieceIndex
//                 << "x:" << move.movedPosition.x << "y:" << move.movedPosition.y
//                 << "angle:" << move.movedAngle << "track:" << move.track;
    }
//    while(1);
    int depth = 5;
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

Game AIController::createNewGameNode(Game oldGame, const Move &move) const
{
    qDebug() << "move: index:" << move.pieceIndex
             << "x:" << move.movedPosition.x << "y:" << move.movedPosition.y
             << "angle:" << move.movedAngle
             << "value:" << move.value << "track:" << move.track;
    Game newGame = oldGame;
    auto index = static_cast<size_t>(move.pieceIndex);
    Position pos = move.movedPosition;
    newGame.updatePiecePosition(index, pos.x, pos.y);
    if (move.movedAngle != -1)
    {
        newGame.updatePieceAngle(index, move.movedAngle);
    }
    newGame.setLastMove(move);
//    newGame.calculateBeamCoords(myColor == Color::Red ? 0 : 9,
//                                myColor == Color::Red ? 0 : 7);
    newGame.endTurn();
//    qDebug() << "newgame1";
//    newGame.printPieceLayout();
//    newGame.nextTurn();
    return newGame;
}

Move AIController::nextMove(int depth, int totalDepth, Color myColor, Color currentTurnColor,  Game currentGame,
                            Move minScoreMoveForAI, Move maxScoreMoveForOpponent, std::vector<Move>* startingMoves)
{
    qDebug() << "DEPTH:" << depth;
    if (depth == 0)
    {
        return currentGame.getLastMove();
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
//    qDebug() << "Possible:";
//    for (auto& move : moves)
//    {
//        qDebug()<< "index:" << move.pieceIndex << "track:" << move.track;
//        qDebug()<< "x:" << move.movedPosition.x
//                << "y:" << move.movedPosition.y << "\n";
//    }
//    qDebug() << "moves:" << moves.size();
    auto game = currentGame;
//    qDebug() << "game";
//    game.printPieceLayout();

    // maximizing player
    if (myColor == currentTurnColor)
    {
        qDebug() << "max";
        Move value = Move::minInit();
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(game, move);
//            qDebug() << "newgame";
//            newGame.printPieceLayout();

//            Color nextColorTurn = currentTurnColor == Color::Red ? Color::Grey : Color::Red;
            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);
            int newTurnValue = evaluateBoardState(myColor, newGame);
            newTurn.value = newTurnValue;

            value = newTurn > value ? newTurn : value;
//            qDebug() << "move:" << "value:" << value.value
//                     << "index" << value.pieceIndex;

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
        qDebug() << "min";
        Move value = Move::maxInit();
        for (auto& move : moves)
        {
            auto newGame = createNewGameNode(game, move);

//            Color nextColorTurn = currentTurnColor == Color::Red ? Color::Grey : Color::Red;
            auto newTurn = nextMove(depth-1, totalDepth, myColor, newGame.currentPlayerTurn(),
                                    newGame, minScoreMoveForAI, maxScoreMoveForOpponent);
            int newTurnValue = evaluateBoardState(myColor, newGame);
            newTurn.value = newTurnValue;

            value = value < newTurn ? value : newTurn;
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

std::vector<Move> AIController::generatePossibleMoves(Color playerColor, const Game& game)
{
    const auto pieces = game.getPieces();
    std::vector<Move> moves;
    for (auto& piece : pieces)
    {
//        if (!piece->isKilled() && piece->color() == playerColor && (piece->index() == 0 || piece->index() == 10))
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
    return moves;
}

