//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_TICTACTOEQLEARNER_H
#define RLEXPERIMENTS_TICTACTOEQLEARNER_H

#include <QLearningPolicy.h>
#include "TicTacToeSettings.h"

#include "PlayerEnum.h"
#include "BoardStatusEnum.h"

namespace TTT
{
    namespace
    {
        constexpr auto defaultAgentId = Player::Cross;
    }

class TicTacToeQLearner : public RL::QLearnerPolicy<Player, uint32_t, uint32_t, TicTacToeSettings<BoardStatus>, BoardStatus>
{
public:
    using Base = RL::QLearnerPolicy<Player, uint32_t, uint32_t, TicTacToeSettings<BoardStatus>, BoardStatus>;

    TicTacToeQLearner() : Base(defaultAgentId, TicTacToeSettings<BoardStatus>{}) {}
    TicTacToeQLearner(const Player& anAgentId, const TicTacToeSettings<BoardStatus>& aLearningSettings);

protected:
    bool IsAgentLastMove(const uint32_t& aLastMove, BoardStatus& anOutMoveStatus) const;
    std::vector<uint32_t> ComputeAgentActions(const uint32_t& aCurrentState) const;
    uint32_t ExplorationJob(const uint32_t& aCurrentState) const;
    uint32_t GreedyJob(const uint32_t& aCurrentState) const;
};
}

#endif //RLEXPERIMENTS_TICTACTOEQLEARNER_H
