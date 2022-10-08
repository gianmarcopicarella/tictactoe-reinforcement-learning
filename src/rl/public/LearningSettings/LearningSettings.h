//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_LEARNINGSETTINGS_H
#define RLEXPERIMENTS_LEARNINGSETTINGS_H

#include <unordered_map>

namespace rl {
    template<typename ActionStatus>
    struct BaseLearningSettings {
        int myTrainEpisodesCount = 0;
        int myTestingEpisodesCount = 0;
        float myLearningRate = 0.0f;
        bool myIsTraining = false;

        using StaticScoresMap = std::unordered_map<ActionStatus, float>;
        StaticScoresMap myStaticScores;
    };
}
#endif //RLEXPERIMENTS_LEARNINGSETTINGS_H
