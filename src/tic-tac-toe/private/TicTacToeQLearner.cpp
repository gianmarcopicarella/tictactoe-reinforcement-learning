//
// Created by Gianmarco Picarella on 08/10/22.
//

#include "TicTacToeQLearner.h"

#include "GameUtils.h"
#include <set>

namespace TTT
{
    TicTacToeQLearner::TicTacToeQLearner(const Player& anAgentId, const TicTacToeSettings<BoardStatus>& aLearningSettings) :
            Base(anAgentId, aLearningSettings)
    {
        std::set<uint32_t> validBoardStates;

        const auto otherPlayer = static_cast<Player>((~static_cast<uint32_t>(myId)) & 0x3);
        const auto startingPlayer = myLearningSettings.myIsAgentFirstToMove ? myId : otherPlayer;

        TTT::Utils::GenerateBoards(myId, startingPlayer, validBoardStates);

        for (auto boardState : validBoardStates)
        {
            const auto boardScore = myLearningSettings.myStaticScores[TTT::Utils::GetBoardStatus(myId, boardState)];
            myActionValueScores.insert(std::make_pair(boardState, boardScore));
        }
    }

    std::vector<uint32_t> TicTacToeQLearner::ComputeAgentActions(const uint32_t& aCurrentState) const
    {
        return TTT::Utils::GenerateMoves(myId, aCurrentState);
    }

    bool TicTacToeQLearner::IsAgentLastMove(const uint32_t& aLastMove, BoardStatus& anOutMoveStatus) const
    {
        anOutMoveStatus = TTT::Utils::GetBoardStatus(myId, aLastMove);

        return anOutMoveStatus == BoardStatus::Win ||
               (anOutMoveStatus == BoardStatus::Draw && myId == Player::Cross);
    }
    uint32_t TicTacToeQLearner::ExplorationJob(const uint32_t& aCurrentState) const
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextAgentMoves = TTT::Utils::GenerateMoves(myId, aCurrentState);

        assert(nextAgentMoves.size() > 0);

        std::uniform_int_distribution<> uniIntDistr(0, nextAgentMoves.size() - 1);

        return nextAgentMoves[uniIntDistr(rng)];
    }
    uint32_t TicTacToeQLearner::GreedyJob(const uint32_t& aCurrentState) const
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextAgentMoves = TTT::Utils::GenerateMoves(myId, aCurrentState);

        assert(nextAgentMoves.size() > 0);

        // Find max
        const uint32_t maxValueMove = *std::max_element(nextAgentMoves.begin(), nextAgentMoves.end(), [&](auto& m1, auto& m2) {
            assert(myActionValueScores.find(m1) != myActionValueScores.end());
            assert(myActionValueScores.find(m2) != myActionValueScores.end());

            return myActionValueScores.find(m1)->second < myActionValueScores.find(m2)->second;
        });

        std::vector<uint32_t> maxMoves;

        constexpr auto floatEpsilon = 0.0001f;

        for (auto agentMove : nextAgentMoves)
        {
            if (std::fabs(myActionValueScores.find(maxValueMove)->second - myActionValueScores.find(agentMove)->second) < floatEpsilon)
            {
                maxMoves.push_back(agentMove);
            }
        }

        assert(maxMoves.size() > 0);

        // Select one of the random max
        std::uniform_int_distribution<> uniIntDistr(0, maxMoves.size() - 1);

        return maxMoves[uniIntDistr(rng)];
    }
}