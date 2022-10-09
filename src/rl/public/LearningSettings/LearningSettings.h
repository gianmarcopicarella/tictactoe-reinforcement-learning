//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_LEARNINGSETTINGS_H
#define RLEXPERIMENTS_LEARNINGSETTINGS_H

#include <unordered_map>
#include <cereal/cereal.hpp>

namespace RL
{
    template<typename ActionStatus>
    struct BaseLearningSettings {
        float myLearningRate = 0.0f;
        bool myIsTraining = false;

        using StaticScoresMap = std::unordered_map<ActionStatus, float>;
        StaticScoresMap myStaticScores;

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(CEREAL_NVP(myLearningRate), CEREAL_NVP(myIsTraining), CEREAL_NVP(myStaticScores));
        }
    };
}
#endif //RLEXPERIMENTS_LEARNINGSETTINGS_H
