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

        const auto nextMoves = TTT::Utils::GenerateMoves(myId, aCurrentState);

        assert(nextMoves.size() > 0);

        std::uniform_real_distribution<> floatDistribution(0.f, 1.f);

        if (floatDistribution(rng) < myRandomEpsilon)
        {
            std::uniform_int_distribution<> uniIntDistr(0, nextMoves.size() - 1);
            return nextMoves[uniIntDistr(rng)];
        }
        else
        {
            const auto nextPlayer = static_cast<Player>((~static_cast<uint32_t>(myId)) & 0x3);

            std::vector<std::pair<int, uint32_t>> nextMovesScores;
            nextMovesScores.reserve(nextMoves.size());

            for (const auto nextMove : nextMoves)
            {
                const auto moveValue = TicTacToeMinimax(nextMove, nextPlayer, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                nextMovesScores.emplace_back(moveValue, nextMove);
            }

            std::sort(nextMovesScores.begin(), nextMovesScores.end(), std::greater<>());

            const auto equalMinMovesCount = std::count_if(nextMovesScores.begin(), nextMovesScores.end(), [&](const auto& p) {
                return nextMovesScores[0].first == p.first;
            });

            std::uniform_int_distribution<> intDistribution(0, equalMinMovesCount - 1);

            return nextMovesScores[intDistribution(rng)].second;
        }
    }

    int EpsilonOptimalOpponent::TicTacToeMinimax(const uint32_t aBoard, const Player aPlayer, int anAlpha, int aBeta)
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
                value = std::max(value, TicTacToeMinimax(nextMove, nextPlayer, anAlpha, aBeta));

                if(value >= aBeta)
                {
                    break;
                }
                anAlpha = std::max(anAlpha, value);
            }

            return value;
        }
        else
        {
            auto  value = std::numeric_limits<int>::max();

            for (const auto nextMove : TTT::Utils::GenerateMoves(aPlayer, aBoard))
            {
                value = std::min(value, TicTacToeMinimax(nextMove, nextPlayer, anAlpha, aBeta));

                if(value <= anAlpha)
                {
                    break;
                }
                aBeta = std::min(aBeta, value);
            }

            return value;
        }
    }
}
