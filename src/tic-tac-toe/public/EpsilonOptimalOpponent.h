//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_EPSILONOPTIMALOPPONENT_H
#define RLEXPERIMENTS_EPSILONOPTIMALOPPONENT_H

#include <Agent.h>
#include <cstdint>

#include "PlayerEnum.h"

namespace TTT
{
class EpsilonOptimalOpponent : public RL::Agent<Player, uint32_t, uint32_t>
    {
    public:
        using Base = RL::Agent<Player, uint32_t, uint32_t>;

        EpsilonOptimalOpponent(const Player& aTrainerId, float aRandomEpsilon) : Base(aTrainerId), myRandomEpsilon(aRandomEpsilon) {}

        uint32_t GetNextAction(const uint32_t& aCurrentState);

    private:
        float myRandomEpsilon;
        int TicTacToeMinimax(const uint32_t aBoard, const Player aPlayer,  int anAlpha, int aBeta);
    };

}
#endif //RLEXPERIMENTS_EPSILONOPTIMALOPPONENT_H
