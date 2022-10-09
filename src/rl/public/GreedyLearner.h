//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_GREEDYLEARNER_H
#define RLEXPERIMENTS_GREEDYLEARNER_H

#include "LearningPolicy.h"

#include <cereal/types/unordered_map.hpp>
#include <cereal/types/memory.hpp>
#include <unordered_map>
#include <random>

namespace RL {
    template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
    class GreedyLearner : public LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus> {
    public:
        using Base = LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus>;

        GreedyLearner() = delete;

        GreedyLearner(const AgentId &anAgentId, const LearningSettings &aLearningSettings) :
                Base(anAgentId, aLearningSettings) {}

        virtual ~GreedyLearner() {}

        Action GetNextAction(const State &aCurrentState) {
            static std::random_device dev;
            static std::mt19937 rng(dev());

            Action result;

            if (Base::myLearningSettings.myIsTraining)
            {
                std::uniform_real_distribution<> uniFltDistribution(0.f, 1.f);

                if (uniFltDistribution(rng) < Base::myLearningSettings.myRandomEpsilon)
                {
                    result = ExplorationJob(aCurrentState);
                }
                else
                {
                    result = GreedyJob(aCurrentState);
                }

                Base::myLearningSettings.myRandomEpsilon *= (1.f - Base::myLearningSettings.myRandomEpsilonDecay);
            }
            else
            {
                result = GreedyJob(aCurrentState);
            }

            return result;
        }

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(cereal::base_class<Base>(this),
                    CEREAL_NVP(myActionValueScores));
        }

    protected:
        virtual Action ExplorationJob(const State &aCurrentState) const = 0;
        virtual Action GreedyJob(const State &aCurrentState) const = 0;

        using ActionValueScoresMap = std::unordered_map<Action, float>;
        ActionValueScoresMap myActionValueScores;
    };
}
#endif //RLEXPERIMENTS_GREEDYLEARNER_H
