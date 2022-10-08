//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_TICTACTOEQLEARNER_H
#define RLEXPERIMENTS_TICTACTOEQLEARNER_H

#include "QLearningPolicy.h"
#include "LearningSettings/QLearningSettings.h"

#include "PlayerEnum.h"
#include "BoardStatusEnum.h"

namespace TTT
{
class TicTacToeQLearner : public rl::QLearnerPolicy<Player, uint32_t, uint32_t, rl::QLearningSettings<BoardStatus>, BoardStatus>
{
public:
    using Base = rl::QLearnerPolicy<Player, uint32_t, uint32_t, rl::QLearningSettings<BoardStatus>, BoardStatus>;

    TicTacToeQLearner() = delete;

    TicTacToeQLearner(const Player& anAgentId, const rl::QLearningSettings<BoardStatus>& aLearningSettings);

protected:
    bool IsAgentLastMove(const uint32_t& aLastMove, BoardStatus& anOutMoveStatus) const;
    std::vector<uint32_t> ComputeAgentActions(const uint32_t& aCurrentState) const;
    uint32_t ExplorationJob(const uint32_t& aCurrentState) const;
    uint32_t GreedyJob(const uint32_t& aCurrentState) const;
};
}

#endif //RLEXPERIMENTS_TICTACTOEQLEARNER_H
