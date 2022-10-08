// TicTacToeRL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cstdint>
#include <random>
#include <functional>

#include <LearningSettings/QLearningSettings.h>

#include <TicTacToeQLearner.h>
#include <EpsilonOptimalOpponent.h>

#include <PlayerEnum.h>
#include <BoardStatusEnum.h>
#include <RandomOpponent.h>
#include <GameUtils.h>

template <typename LearningSettings>
void Simulate(rl::LearningPolicy<TTT::Player, uint32_t, uint32_t, LearningSettings, TTT::BoardStatus>& aLearningAgent,
              rl::Agent<TTT::Player, uint32_t, uint32_t>& aTrainerAgent,
              bool aFirstMoveFromLearnerFlag = true,
              std::function<void(const std::vector<uint32_t>&, int)> onEpisodeEndCallback = nullptr)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());

    std::vector<uint32_t> gameplayHistory;

    const auto episodesCount = aLearningAgent.GetLearningSettings().myIsTraining ?
                               aLearningAgent.GetLearningSettings().myTrainEpisodesCount :
                               aLearningAgent.GetLearningSettings().myTestingEpisodesCount;

    for (auto episodeIdx = 0; episodeIdx < episodesCount; ++episodeIdx)
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

int main()
{
    rl::QLearningSettings<TTT::BoardStatus> QLSettings;

    QLSettings.myGamma = 0.9f;
    QLSettings.myGreedyEpsilon = 0.3f;
    QLSettings.myGreedyEpsilonDecay = 0.000001f;
    QLSettings.myLearningRate = 0.5f;

    QLSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Win, 1.f));
    QLSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Draw, 0.f));
    QLSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Lose, -1.f));
    QLSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Intermediate, 0.5f));

    QLSettings.myTrainEpisodesCount = 100000;
    QLSettings.myTestingEpisodesCount = 100000;
    QLSettings.myIsTraining = true;

    // Create the learning agent
    TTT::TicTacToeQLearner learningAgent{ TTT::Player::Cross, QLSettings };

    // Create the random opponent
    TTT::RandomOpponent randomOpponent{ TTT::Player::Nought };

    // Create the mixed random-optimal opponent with custom probability
    TTT::EpsilonOptimalOpponent optimalOpponent{ TTT::Player::Nought, 0.3f };

    // Create a custom episode callback
    int win = 0, draw = 0, lose = 0;

    const auto episodeCallback = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex) {
        switch (TTT::Utils::GetBoardStatus(learningAgent.GetAgentId(), aGameplayHistory.back()))
        {
            case TTT::BoardStatus::Win:
                win += 1;
                break;;
            case TTT::BoardStatus::Lose:
                lose += 1;
                break;
            case TTT::BoardStatus::Draw:
                draw += 1;
                break;
        }

        if (anEpisodeIndex % 20 == 0)
        {
            std::cout << "Win: " << win << "  / Draw: " << draw << "  / Lose: " << lose << std::endl;
        }
    };

    // Training Phase
    Simulate(
            static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<TTT::EpsilonOptimalOpponent::Base&>(optimalOpponent),
            true,
            episodeCallback);

    // Testing Phase
    std::cout << "\n!!! TESTING PHASE !!!" << std::endl;

    learningAgent.SetTrainingMode(false);

    // Serialize
    //learningAgent.SerializeActionValueScoresMap("policy.txt");

    win = 0, draw = 0, lose = 0;

    Simulate(
            static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<TTT::EpsilonOptimalOpponent::Base&>(optimalOpponent),
            true,
            episodeCallback);

    std::cout <<
              (win / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Wins\n" <<
              (draw / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Draws\n" <<
              (lose / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Loses\n\n";

    // Random

    win = 0, draw = 0, lose = 0;

    Simulate(
            static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<TTT::EpsilonOptimalOpponent::Base&>(randomOpponent),
            true,
            episodeCallback);

    std::cout <<
              (win / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Wins\n" <<
              (draw / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Draws\n" <<
              (lose / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Loses\n";
}