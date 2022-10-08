//
// Created by Gianmarco Picarella on 08/10/22.
//

#include "EpsilonOptimalOpponent.h"

#include "BoardStatusEnum.h"

#include "GameUtils.h"

#include <random>

namespace TTT
{
    uint32_t EpsilonOptimalOpponent::GetNextAction(const uint32_t& aCurrentState)
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        auto nextMoves = TTT::Utils::GenerateMoves(myId, aCurrentState);

        assert(nextMoves.size() > 0);

        std::uniform_real_distribution<> uniFltDistr(0.f, 1.f);

        if (uniFltDistr(rng) < myRandomEpsilon)
        {
            std::uniform_int_distribution<> uniIntDistr(0, nextMoves.size() - 1);
            return nextMoves[uniIntDistr(rng)];
        }
        else
        {
            const auto nextPlayer = static_cast<Player>((~static_cast<uint32_t>(myId)) & 0x3);

            std::vector<std::pair<uint32_t, int>> nextMovesScores;

            for (const auto nextMove : nextMoves)
            {
                nextMovesScores.push_back(std::make_pair(nextMove, TicTacToeMinimax(nextMove, nextPlayer)));
            }

            std::sort(nextMovesScores.begin(), nextMovesScores.end(), [](auto& p1, auto& p2) {
                return p1.second > p2.second;
            });

            const auto equalMinMovesCount = std::count_if(nextMovesScores.begin(), nextMovesScores.end(), [&](const auto& p) {
                return nextMovesScores[0].second == p.second;
            });

            std::uniform_int_distribution<> uniIntDistr(0, equalMinMovesCount - 1);

            return nextMovesScores[uniIntDistr(rng)].first;
        }
    }
    int EpsilonOptimalOpponent::TicTacToeMinimax(const uint32_t aBoard, const Player aPlayer)
    {
        switch (TTT::Utils::GetBoardStatus(myId, aBoard))
        {
            case BoardStatus::Win: return 1;
            case BoardStatus::Draw: return 0;
            case BoardStatus::Lose: return -1;
            default: break;
        }

        const auto nextPlayer = static_cast<Player>((~static_cast<uint32_t>(aPlayer)) & 0x3);

        if (aPlayer == myId)
        {
            auto  value = std::numeric_limits<int>::min();

            for (const auto nextMove : TTT::Utils::GenerateMoves(aPlayer, aBoard))
            {
                value = std::max(value, TicTacToeMinimax(nextMove, nextPlayer));
            }

            return value;
        }
        else
        {
            auto  value = std::numeric_limits<int>::max();

            for (const auto nextMove : TTT::Utils::GenerateMoves(aPlayer, aBoard))
            {
                value = std::min(value, TicTacToeMinimax(nextMove, nextPlayer));
            }

            return value;
        }
    }
}
