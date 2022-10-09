//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_RANDOMOPPONENT_H
#define RLEXPERIMENTS_RANDOMOPPONENT_H

#include <Agent.h>
#include <cstdint>

#include "PlayerEnum.h"
#include "BoardStatusEnum.h"

namespace TTT
{
    class RandomOpponent : public RL::Agent<Player, uint32_t, uint32_t>
    {
    public:
        using Base = RL::Agent<Player, uint32_t, uint32_t>;

        RandomOpponent(const Player& aTrainerId) : Base(aTrainerId) {}

        uint32_t GetNextAction(const uint32_t& aCurrentState);
    };
}

#endif //RLEXPERIMENTS_RANDOMOPPONENT_H
