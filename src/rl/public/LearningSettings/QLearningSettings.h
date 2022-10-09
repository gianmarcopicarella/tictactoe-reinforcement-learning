//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_QLEARNINGSETTINGS_H
#define RLEXPERIMENTS_QLEARNINGSETTINGS_H

#include "LearningSettings.h"

#include <cereal/types/base_class.hpp>

namespace RL
{
    template<typename ActionStatus>
    struct QLearningSettings : public BaseLearningSettings<ActionStatus>
    {
        float myGamma = 0.0f;
        float myRandomEpsilon = 0.0f;
        float myRandomEpsilonDecay = 0.0f;

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(cereal::base_class<BaseLearningSettings<ActionStatus>>(this),
                    CEREAL_NVP(myGamma), CEREAL_NVP(myRandomEpsilon), CEREAL_NVP(myRandomEpsilonDecay));
        }
    };
}

#endif //RLEXPERIMENTS_QLEARNINGSETTINGS_H
