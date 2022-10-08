//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_QLEARNINGSETTINGS_H
#define RLEXPERIMENTS_QLEARNINGSETTINGS_H

#include "LearningSettings.h"

namespace rl
{
    template<typename ActionStatus>
    struct QLearningSettings : public BaseLearningSettings<ActionStatus>
    {
        float myGamma = 0.0f;
        float myGreedyEpsilon = 0.0f;
        float myGreedyEpsilonDecay = 0.0f;
    };
}

#endif //RLEXPERIMENTS_QLEARNINGSETTINGS_H
