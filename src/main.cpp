#include <iostream>
#include <cstdint>
#include <functional>

#include <TicTacToeQLearner.h>
#include <EpsilonOptimalOpponent.h>
#include <RandomOpponent.h>

#include <PlayerEnum.h>
#include <BoardStatusEnum.h>
#include <GameUtils.h>

#include <CLI/CLI.hpp>

#include <sstream>
#include <cereal/archives/json.hpp>

int main(int argc, char **argv)
{
    constexpr auto appDescription = "A reinforcement learning test applied to TicTacToe";
    constexpr auto appName = "TicTacToe learner";

    CLI::App app {appDescription , appName };

    app.require_subcommand(1);

    TTT::TicTacToeSettings<TTT::BoardStatus> agentSettings;

    agentSettings.myGamma = 0.9f;
    agentSettings.myRandomEpsilon = 0.3f;
    agentSettings.myRandomEpsilonDecay = 0.000001f;
    agentSettings.myLearningRate = 0.5f;

    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Win, 1.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Draw, 0.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Lose, -1.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Intermediate, 0.5f));

    // Common params
    auto iterationsCount = 50000;

    // Add Train subcommand
    auto train = app.add_subcommand("train", "Train a policy using QLearning");

    train->add_option("-i", iterationsCount, "Set the number of iterations");

    std::vector<float> rewards;
    auto rewardsParam = train->add_option("--rewards", rewards, "Set reward values (win, draw, lose, intermediate)");

    rewardsParam->expected(4);

    // Train settings
    auto gammaParam = train->add_option("-g", agentSettings.myGamma, "Set gamma value");
    auto epsilonParam = train->add_option("-e", agentSettings.myRandomEpsilon, "Set epsilon value");
    auto epsilonDecayParam = train->add_option("-d", agentSettings.myRandomEpsilonDecay, "Set epsilon decay value");
    auto learningRateParam = train->add_option("-r", agentSettings.myLearningRate, "Set learning rate value");

    gammaParam->check(CLI::Range(0.f,1.f));
    epsilonParam->check(CLI::Range(0.f,1.f));
    epsilonDecayParam->check(CLI::Range(0.f,1.f));
    learningRateParam->check(CLI::Range(0.f,1.f));

    auto isAgentNought { false };
    train->add_flag("--nought", isAgentNought, "Set agent side to nought");

    auto isAgentDelayed { false };
    train->add_flag("--delay", isAgentDelayed, "Set agent to play after the trainer");

    agentSettings.myIsAgentFirstToMove = !isAgentDelayed;

    std::string savePath;
    auto savePathParam = train->add_option("-o", savePath, "Set policy save path");

    savePathParam->required();
    savePathParam->check(CLI::NonexistentPath);

    auto epsilonValue { 0.0f };
    auto epsilonOptimalParam = train->add_option("--optimal", epsilonValue, "Set the opponent type to epsilon optimal");

    epsilonOptimalParam->check(CLI::Range(0.f,1.f));

    // Add Test subcommand
    auto test = app.add_subcommand("test", "Test a trained policy against an opponent");

    test->add_option("-i", iterationsCount, "Set the number of iterations");

    std::string loadPath;
    auto loadPathParam = test->add_option("-l", loadPath, "Set policy load path");

    loadPathParam->required();
    loadPathParam->check(CLI::ExistingFile);

    auto isEpsilonOptimalOpponent { false };
    test->add_flag("--optimal", isEpsilonOptimalOpponent, "Set the opponent type to epsilon optimal");

    train->callback([&]() {

        if(!rewardsParam->empty())
        {
            agentSettings.myStaticScores[TTT::BoardStatus::Win] = rewards[0];
            agentSettings.myStaticScores[TTT::BoardStatus::Draw] = rewards[1];
            agentSettings.myStaticScores[TTT::BoardStatus::Lose] = rewards[2];
            agentSettings.myStaticScores[TTT::BoardStatus::Intermediate] = rewards[3];
        }

        const auto agentSide = isAgentNought ? TTT::Player::Nought : TTT::Player::Cross;
        const auto opponentSide = isAgentNought ? TTT::Player::Cross : TTT::Player::Nought;

        agentSettings.myIsTraining = true;

        TTT::TicTacToeQLearner learningAgent{ agentSide, agentSettings };

        RL::Agent<TTT::Player, uint32_t, uint32_t> * opponentPtr;

        if(epsilonOptimalParam->empty())
        {
            opponentPtr = new TTT::RandomOpponent{opponentSide};
        }
        else
        {
            opponentPtr = new TTT::EpsilonOptimalOpponent{opponentSide, epsilonValue};
        }

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
                *opponentPtr,
                iterationsCount,
                learningAgent.GetLearningSettings().myIsAgentFirstToMove);

        learningAgent.SetTrainingMode(false);

        // save agent to disk
        std::ofstream os(savePath);
        assert(os.is_open());

        {
            cereal::JSONOutputArchive archive(os);
            archive(CEREAL_NVP(learningAgent));
        }

        os.close();

        std::cout << "Training done" << std::endl;
    });


    test->callback([&]() {
        // load agent from disk
        std::ifstream os(loadPath);

        assert(os.is_open());

        cereal::JSONInputArchive archive(os);

        TTT::TicTacToeQLearner trainedAgent;

        archive(trainedAgent);

        os.close();

        trainedAgent.SetTrainingMode(false);

        RL::Agent<TTT::Player, uint32_t, uint32_t> * opponentPtr;

        const auto opponentSide = trainedAgent.GetAgentId() == TTT::Player::Cross ? TTT::Player::Nought : TTT::Player::Cross;

        if(isEpsilonOptimalOpponent)
        {
            constexpr auto testPhaseEpsilonValue = 0.f;
            opponentPtr = new TTT::EpsilonOptimalOpponent{opponentSide, testPhaseEpsilonValue};
        }
        else
        {
            opponentPtr = new TTT::RandomOpponent{opponentSide};
        }

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(trainedAgent),
                *opponentPtr,
                iterationsCount,
                trainedAgent.GetLearningSettings().myIsAgentFirstToMove);

        std::cout << "Testing done" << std::endl;
/*        std::cout << "Win = " <<
            100*win/static_cast<float>(iterationsCount) <<
            "% // Draw = " <<
            100*draw/static_cast<float>(iterationsCount) <<
            "% // Lose = " <<
            100*lose/static_cast<float>(iterationsCount) << "%" << std::endl;
*/
    });

    // Parse command line params
    CLI11_PARSE(app, argc, argv);
}