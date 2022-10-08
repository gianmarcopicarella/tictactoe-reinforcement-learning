//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_GREEDYLEARNER_H
#define RLEXPERIMENTS_GREEDYLEARNER_H

#include "LearningPolicy.h"

#include <unordered_map>
#include <random>

namespace rl {
    template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
    class GreedyLearner : public LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus> {
    public:
        using Base = LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus>;

        GreedyLearner() = delete;

        GreedyLearner(const AgentId &anAgentId, const LearningSettings &aLearningSettings) :
                Base(anAgentId, aLearningSettings) {}

        Action GetNextAction(const State &aCurrentState) {
            static std::random_device dev;
            static std::mt19937 rng(dev());

            Action result;

            if (Base::myLearningSettings.myIsTraining) {
                std::uniform_real_distribution<> uniFltDistr(0.f, 1.f);

                if (uniFltDistr(rng) < Base::myLearningSettings.myGreedyEpsilon) {
                    result = ExplorationJob(aCurrentState);
                } else {
                    result = GreedyJob(aCurrentState);
                }

                Base::myLearningSettings.myGreedyEpsilon *= (1.f - Base::myLearningSettings.myGreedyEpsilonDecay);
            } else {
                result = GreedyJob(aCurrentState);
            }

            return result;
        }

    protected:
        using ActionValueScoresMap = std::unordered_map<Action, float>;
        ActionValueScoresMap myActionValueScores;

        virtual Action ExplorationJob(const State &aCurrentState) const = 0;

        virtual Action GreedyJob(const State &aCurrentState) const = 0;
    };
}
#endif //RLEXPERIMENTS_GREEDYLEARNER_H
