//
// Created by Gianmarco Picarella on 08/10/22.
//

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

#include <indicators/cursor_control.hpp>
#include <indicators/progress_spinner.hpp>

#include <matplot/matplot.h>

void PlotCumulativeGameResults(const std::unordered_map<TTT::BoardStatus, int>& someResultsCount)
{
    using namespace matplot;
    bar(std::vector<int> {
            someResultsCount.find(TTT::BoardStatus::Win)->second,
            someResultsCount.find(TTT::BoardStatus::Draw)->second,
            someResultsCount.find(TTT::BoardStatus::Lose)->second});

    gca()->x_axis().ticklabels({"Wins", "Draws", "Loses"});
    gca()->y_axis().label("N. Episodes");
    show();
}

void PlotCumulativeRewardFunction(const std::vector<float>& someCumulativeRewards, int anEpisodesCount)
{
    using namespace matplot;
    std::vector<double> x = linspace(0, anEpisodesCount, someCumulativeRewards.size());
    plot(x, someCumulativeRewards);
    show();
}

int main(int argc, char **argv)
{
    using namespace indicators;
    show_console_cursor(false);

    CLI::App cli;
    cli.require_subcommand(1);

    TTT::TicTacToeSettings<TTT::BoardStatus> agentSettings;

    // Default settings
    agentSettings.myGamma = 0.9f;
    agentSettings.myRandomEpsilon = 0.3f;
    agentSettings.myRandomEpsilonDecay = 0.000001f;
    agentSettings.myLearningRate = 0.5f;

    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Win, 1.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Draw, 0.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Lose, -1.f));
    agentSettings.myStaticScores.insert(std::make_pair(TTT::BoardStatus::Intermediate, 0.5f));

    auto iterationsCount = 50000;

    // Train subcommand
    auto train = cli.add_subcommand("train", "Train a policy using QLearning");

    // Iterations parameter
    train->add_option("-i", iterationsCount, "Number of iterations");

    // Rewards parameter
    std::vector<float> rewards;
    auto rewardsParam = train->add_option("--rewards", rewards, "Reward values (Win, Draw, Lose, Intermediate)");
    rewardsParam->expected(4);

    // Training parameters
    auto gammaParam = train->add_option("-g", agentSettings.myGamma, "Gamma");
    auto epsilonParam = train->add_option("-e", agentSettings.myRandomEpsilon, "Epsilon");
    auto epsilonDecayParam = train->add_option("-d", agentSettings.myRandomEpsilonDecay, "Epsilon decay");
    auto learningRateParam = train->add_option("-r", agentSettings.myLearningRate, "Learning rate");

    gammaParam->check(CLI::Range(0.f,1.f));
    epsilonParam->check(CLI::Range(0.f,1.f));
    epsilonDecayParam->check(CLI::Range(0.f,1.f));
    learningRateParam->check(CLI::Range(0.f,1.f));

    // Plotting flags
    auto rewardPlot { false };
    train->add_flag("--plotReward", rewardPlot, "Plot cumulative reward");

    auto resultsPlot{ false };
    train->add_flag("--plotResults", resultsPlot, "Plot game results");

    // Gameplay flags
    auto isAgentNought { false };
    train->add_flag("--nought", isAgentNought, "Agent side is nought");

    auto isAgentDelayed { false };
    train->add_flag("--delay", isAgentDelayed, "Delay first agent move");

    agentSettings.myIsAgentFirstToMove = !isAgentDelayed;

    // Serialization path
    std::string savePath;
    auto savePathParam = train->add_option("--save", savePath, "Trained policy save path");

    savePathParam->required();

    // Training agent parameter
    auto epsilonValue { 0.0f };
    auto epsilonOptimalParam = train->add_option("--optimal", epsilonValue, "Select epsilon-optimal opponent (Default equals to random)");

    epsilonOptimalParam->check(CLI::Range(0.f,1.f));

    // Test subcommand
    auto test = cli.add_subcommand("test", "Test a trained policy against an opponent");

    // Plotting flag
    auto testResultsPlot { false };
    test->add_option("--resultsPlot", testResultsPlot, "Set results plot save path");

    // Iterations parameter
    test->add_option("-i", iterationsCount, "Number of iterations");

    // Deserialization path parameter
    std::string loadPath;
    auto loadPathParam = test->add_option("--load", loadPath, "Trained policy path");

    loadPathParam->required();
    loadPathParam->check(CLI::ExistingFile);

    // Opponent parameter
    auto isEpsilonOptimalOpponent { false };
    test->add_flag("--optimal", isEpsilonOptimalOpponent, "Select optimal opponent (Default equals to random)");

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

        std::unordered_map<TTT::BoardStatus, int> resultsCounter;
        std::vector<float> cumulativeRewards;

        if(rewardPlot)
        {
            cumulativeRewards.reserve(iterationsCount+1);
            cumulativeRewards.emplace_back(0.f);
        }

        if(resultsPlot)
        {
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Win, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Draw, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Lose, 0));
        }

        indicators::ProgressSpinner spinner{
                option::PostfixText{"Training agent"},
                option::ForegroundColor{Color::yellow},
                option::SpinnerStates{std::vector<std::string>{"⠈", "⠐", "⠠", "⢀", "⡀", "⠄", "⠂", "⠁"}},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
        };

        const auto episodeCallback = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex){
            if(rewardPlot)
            {
                const auto boardStatus = TTT::Utils::GetBoardStatus(learningAgent.GetAgentId(), aGameplayHistory.back());
                const auto reward = learningAgent.GetLearningSettings().myStaticScores.find(boardStatus)->second;

                cumulativeRewards.emplace_back(cumulativeRewards.back() + reward);
            }
            if(resultsPlot)
            {
                const auto boardStatus = TTT::Utils::GetBoardStatus(learningAgent.GetAgentId(), aGameplayHistory.back());
                ++resultsCounter[boardStatus];
            }

            spinner.set_progress(100*(anEpisodeIndex+1)/static_cast<float>(iterationsCount));
        };

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
                *opponentPtr,
                iterationsCount,
                learningAgent.GetLearningSettings().myIsAgentFirstToMove,
                episodeCallback);

        learningAgent.SetTrainingMode(false);

        std::ofstream os(savePath);
        assert(os.is_open() && "Cannot open use the serialization path");

        {
            cereal::JSONOutputArchive archive(os);
            archive(CEREAL_NVP(learningAgent));
        }

        os.close();

        spinner.set_option(option::ForegroundColor{Color::green});
        spinner.set_option(option::PrefixText{"✔"});
        spinner.set_option(option::ShowSpinner{false});
        spinner.set_option(option::ShowPercentage{false});
        spinner.set_option(option::PostfixText{"Agent trained and saved successfully!"});
        spinner.mark_as_completed();

        indicators::show_console_cursor(true);

        if(rewardPlot)
        {
            PlotCumulativeRewardFunction(cumulativeRewards, iterationsCount);
        }

        if(resultsPlot)
        {
            PlotCumulativeGameResults(resultsCounter);
        }
    });

    test->callback([&]() {
        std::ifstream os(loadPath);
        assert(os.is_open() && "Cannot open use the deserialization path");

        cereal::JSONInputArchive archive(os);
        TTT::TicTacToeQLearner trainedAgent;

        archive(trainedAgent);
        os.close();

        trainedAgent.SetTrainingMode(false);

        RL::Agent<TTT::Player, uint32_t, uint32_t> * opponentPtr;

        const auto opponentSide = trainedAgent.GetAgentId() == TTT::Player::Cross ? TTT::Player::Nought : TTT::Player::Cross;

        if(isEpsilonOptimalOpponent)
        {
            opponentPtr = new TTT::EpsilonOptimalOpponent{opponentSide, epsilonValue};
        }
        else
        {
            opponentPtr = new TTT::RandomOpponent{opponentSide};
        }

        std::unordered_map<TTT::BoardStatus, int> resultsCounter;

        if(testResultsPlot)
        {
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Win, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Draw, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Lose, 0));
        }

        indicators::ProgressSpinner spinner {
                option::PostfixText{"Testing agent"},
                option::ForegroundColor{Color::yellow},
                option::SpinnerStates{std::vector<std::string>{"⠈", "⠐", "⠠", "⢀", "⡀", "⠄", "⠂", "⠁"}},
                option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
        };

        const auto episodeCallback = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex){
            if(testResultsPlot)
            {
                const auto boardStatus = TTT::Utils::GetBoardStatus(trainedAgent.GetAgentId(), aGameplayHistory.back());
                ++resultsCounter[boardStatus];
            }

            spinner.set_progress(100*(anEpisodeIndex+1)/static_cast<float>(iterationsCount));
        };

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(trainedAgent),
                *opponentPtr,
                iterationsCount,
                trainedAgent.GetLearningSettings().myIsAgentFirstToMove,
                episodeCallback);

        spinner.set_option(option::ForegroundColor{Color::green});
        spinner.set_option(option::PrefixText{"✔"});
        spinner.set_option(option::ShowSpinner{false});
        spinner.set_option(option::ShowPercentage{false});
        spinner.set_option(option::PostfixText{"Agent tested successfully!"});
        spinner.mark_as_completed();

        indicators::show_console_cursor(true);

        if(testResultsPlot)
        {
            PlotCumulativeGameResults(resultsCounter);
        }
    });

    CLI11_PARSE(cli, argc, argv);
}