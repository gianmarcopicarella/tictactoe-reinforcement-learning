//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_LEARNINGPOLICY_H
#define RLEXPERIMENTS_LEARNINGPOLICY_H

#include "Agent.h"
#include "LearningSettings/LearningSettings.h"

#include <cereal/types/base_class.hpp>
#include <cereal/types/memory.hpp>
#include <vector>

namespace RL {
    template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
    class LearningPolicy : public Agent<AgentId, State, Action> {
    public:
        using Base = Agent<AgentId, State, Action>;

        LearningPolicy() = delete;

        LearningPolicy(const AgentId &anAgentId, const LearningSettings &aLearningSettings) : Base(anAgentId),
                                                                                              myLearningSettings(aLearningSettings)
        {
            static_assert(std::is_base_of<BaseLearningSettings<ActionStatus>, LearningSettings>::value,
                          "[LearningPolicy]: LearningSettings type is not a child type of BaseLearningSettings");
        }

        virtual ~LearningPolicy() {}

        const LearningSettings &GetLearningSettings() const { return myLearningSettings; }

        void SetTrainingMode(bool aTrainingFlag)
        {
            myLearningSettings.myIsTraining = aTrainingFlag;
        }

        virtual void Update(const std::vector<uint32_t>& aGameplayHistory) = 0;

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(cereal::base_class<Base>(this),
                    CEREAL_NVP(myLearningSettings));
        }

    protected:
        LearningSettings myLearningSettings;
    };
}

#endif //RLEXPERIMENTS_LEARNINGPOLICY_H
