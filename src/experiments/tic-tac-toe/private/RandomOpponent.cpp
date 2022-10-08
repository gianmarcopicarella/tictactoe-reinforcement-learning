//
// Created by Gianmarco Picarella on 08/10/22.
//

#include "RandomOpponent.h"
#include "GameUtils.h"

#include <random>

namespace TTT
{
    uint32_t RandomOpponent::GetNextAction(const uint32_t& aCurrentState)
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextMoves = TTT::Utils::GenerateMoves(myId, aCurrentState);

        assert(nextMoves.size() > 0);

        std::uniform_int_distribution<> uniIntDistr(0, nextMoves.size() - 1);

        return nextMoves[uniIntDistr(rng)];
    }
}