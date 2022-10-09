//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_GAMEUTILS_H
#define RLEXPERIMENTS_GAMEUTILS_H

#include <string>
#include <cstdint>
#include <set>
#include <vector>
#include <random>

#include "PlayerEnum.h"
#include "BoardStatusEnum.h"

#include <Agent.h>
#include <LearningPolicy.h>

namespace TTT
{
namespace Utils
{
std::string BoardToString(const uint32_t aBoard);

BoardStatus GetBoardStatus(const Player aMovingPlayer, const uint32_t aBoard);

std::vector<uint32_t> GenerateMoves(const Player aPlayerToMove, const uint32_t aCurrentBoard);

void GenerateBoards(const Player anAgentPlayer, std::set<uint32_t>& someOutValidBoards);

template <typename LearningSettings>
void Simulate(RL::LearningPolicy<TTT::Player, uint32_t, uint32_t, LearningSettings, TTT::BoardStatus>& aLearningAgent,
              RL::Agent<TTT::Player, uint32_t, uint32_t>& aTrainerAgent,
              int anIterationsCount,
              bool aFirstMoveFromLearnerFlag = true,
              std::function<void(const std::vector<uint32_t>&, int)> onEpisodeEndCallback = nullptr)
{
        static std::random_device dev;
        static std::mt19937 rng(dev());

        std::vector<uint32_t> gameplayHistory;

        for (auto episodeIdx = 0; episodeIdx < anIterationsCount; ++episodeIdx)
        {
            uint32_t board = 0x00000000;
            TTT::Player player = aFirstMoveFromLearnerFlag ? aLearningAgent.GetAgentId() : aTrainerAgent.GetAgentId();

            while (TTT::Utils::GetBoardStatus(player, board) == TTT::BoardStatus::Intermediate)
            {
                if (player == aLearningAgent.GetAgentId())
                {
                    board = aLearningAgent.GetNextAction(board);
                }
                else
                {
                    board = aTrainerAgent.GetNextAction(board);
                }

                // Add new move
                gameplayHistory.push_back(board);

                // Swap player
                player = static_cast<TTT::Player>((~static_cast<uint32_t>(player)) & 0x3);
            }

            if (onEpisodeEndCallback != nullptr)
            {
                onEpisodeEndCallback(gameplayHistory, episodeIdx);
            }

            // Update values

            if (aLearningAgent.GetLearningSettings().myIsTraining)
            {
                aLearningAgent.Update(gameplayHistory);
            }

            // Clear history
            gameplayHistory.clear();
        }
    }
}
}

#endif //RLEXPERIMENTS_GAMEUTILS_H
