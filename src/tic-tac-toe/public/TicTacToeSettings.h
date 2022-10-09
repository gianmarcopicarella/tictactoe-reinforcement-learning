//
// Created by Gianmarco Picarella on 09/10/22.
//

#ifndef RLEXPERIMENTS_TICTACTOESETTINGS_H
#define RLEXPERIMENTS_TICTACTOESETTINGS_H

#include <LearningSettings/QLearningSettings.h>

namespace TTT
{
template<typename ActionStatus>
struct TicTacToeSettings : public RL::QLearningSettings<ActionStatus>
    {
        bool myIsAgentFirstToMove { true };

        template<class Archive>
        void serialize(Archive & archive)
        {
            archive(cereal::base_class<RL::QLearningSettings<ActionStatus>>(this), CEREAL_NVP(myIsAgentFirstToMove));
        }
    };
}

#endif //RLEXPERIMENTS_TICTACTOESETTINGS_H
