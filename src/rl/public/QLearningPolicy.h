//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_QLEARNINGPOLICY_H
#define RLEXPERIMENTS_QLEARNINGPOLICY_H

#include "GreedyLearner.h"

namespace rl
{
    template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
    class QLearnerPolicy : public GreedyLearner<AgentId, State, Action, LearningSettings, ActionStatus> {

    public:
        using Base = GreedyLearner<AgentId, State, Action, LearningSettings, ActionStatus>;

        QLearnerPolicy(const AgentId &anAgentId, const LearningSettings &aLearningSettings) :
            Base(anAgentId, aLearningSettings) {}

    void Update(const std::vector <State> &aGameplayHistory)
    {
        ActionStatus lastMoveStatus;
        const auto isLastMoveFromAgent = IsAgentLastMove(aGameplayHistory.back(), lastMoveStatus);

        if (!isLastMoveFromAgent) {
            const auto &agentMove = aGameplayHistory[aGameplayHistory.size() - 2];

            const auto reward = Base::myLearningSettings.myStaticScores[lastMoveStatus]/* myActionValueScores[aGameplayHistory.back()]*/;

            Base::myActionValueScores[agentMove] +=
                    Base::myLearningSettings.myLearningRate * (reward - Base::myActionValueScores[agentMove]);
        }

        const auto startingMoveIndex = isLastMoveFromAgent ?
                                       aGameplayHistory.size() - 3 : aGameplayHistory.size() - 4;

        for (int moveIndex = startingMoveIndex; moveIndex > -1; moveIndex -= 2) {
            const auto &nextState = aGameplayHistory[moveIndex + 1];

            const auto& nextAgentMoves = ComputeAgentActions(nextState);

            assert(nextAgentMoves.size() > 0);

            const uint32_t maxValueMove = *std::max_element(nextAgentMoves.begin(), nextAgentMoves.end(),
                                                            [&](auto &m1, auto &m2) {
                                                                assert(Base::myActionValueScores.find(m1) !=
                                                                               Base::myActionValueScores.end());
                                                                assert(Base::myActionValueScores.find(m2) !=
                                                                               Base::myActionValueScores.end());

                                                                return Base::myActionValueScores.find(m1)->second <
                                                                        Base::myActionValueScores.find(m2)->second;
                                                            });

            const auto agentMove = aGameplayHistory[moveIndex];

            assert(Base::myActionValueScores.find(agentMove) != Base::myActionValueScores.end());

            Base::myActionValueScores[agentMove] += Base::myLearningSettings.myLearningRate *
                                              (Base::myLearningSettings.myGamma * Base::myActionValueScores[maxValueMove] -
                                                      Base::myActionValueScores[agentMove]);
        }
    }

    protected:
        virtual bool IsAgentLastMove(const State &aLastMove, ActionStatus& anOutMoveStatus) const = 0;
        virtual std::vector<Action> ComputeAgentActions(const State& aCurrentState) const = 0;
};
}

#endif //RLEXPERIMENTS_QLEARNINGPOLICY_H
