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
#include <sciplot/sciplot.hpp>

void PlotCumulativeGameResults(int anEpisodesCount, int aWinCounter, int aDrawCounter, int aLoseCounter)
{
    sciplot::Plot2D plot;

    plot.legend().hide();
    plot.xlabel("Win/Draw/Lose");
    plot.ylabel("N. Episodes");
    plot.yrange(0.0f, anEpisodesCount);
    plot.drawBoxes(std::vector {0, 1, 2}, std::vector {aWinCounter, aDrawCounter, aLoseCounter})
            .fillSolid()
            .fillColor("green")
            .fillIntensity(0.5);
    plot.boxWidthRelative(0.75);

    sciplot::Figure figure {{plot}};
    sciplot::Canvas canvas {{figure}};

    canvas.title("Game results distribution");
    canvas.size(800, 800);

    canvas.show();
}
void PlotCumulativeRewardFunction(const std::vector<float>& someCumulativeRewards, int anEpisodesCount)
{
    sciplot::Plot2D plot;
    plot.legend().hide();

    plot.drawPoints(sciplot::linspace(1, anEpisodesCount, anEpisodesCount), someCumulativeRewards);

    // Use the previous plots as sub-figures in a larger 2x2 figure.
    sciplot::Figure figure {{plot}};

    figure.title("Cumulative reward function");

    // Create canvas to hold figure
    sciplot::Canvas canvas {{figure}};
    canvas.size(800, 800);
    canvas.title("Cumulative reward function");
    canvas.show();
}

int main(int argc, char **argv)
{
    constexpr auto appDescription = "";
    constexpr auto appName = "Learning Tic-tac-toe with QLearning";

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

    train->add_option("-i", iterationsCount, "Number of iterations");

    std::vector<float> rewards;
    auto rewardsParam = train->add_option("--rewards", rewards, "Reward values (Win, Draw, Lose, Intermediate)");

    rewardsParam->expected(4);

    // Train settings
    auto gammaParam = train->add_option("-g", agentSettings.myGamma, "Gamma");
    auto epsilonParam = train->add_option("-e", agentSettings.myRandomEpsilon, "Epsilon");
    auto epsilonDecayParam = train->add_option("-d", agentSettings.myRandomEpsilonDecay, "Epsilon decay");
    auto learningRateParam = train->add_option("-r", agentSettings.myLearningRate, "Learning rate");

    gammaParam->check(CLI::Range(0.f,1.f));
    epsilonParam->check(CLI::Range(0.f,1.f));
    epsilonDecayParam->check(CLI::Range(0.f,1.f));
    learningRateParam->check(CLI::Range(0.f,1.f));

    auto rewardPlot {false};
    train->add_flag("--rewardPlot", rewardPlot, "Plot cumulative reward");

    auto resultsPlot{false};
    train->add_flag("--resultsPlot", resultsPlot, "Plot game results");

    auto isAgentNought { false };
    train->add_flag("--nought", isAgentNought, "Agent side is nought");

    auto isAgentDelayed { false };
    train->add_flag("--delay", isAgentDelayed, "Delay first agent move");

    agentSettings.myIsAgentFirstToMove = !isAgentDelayed;

    std::string savePath;
    auto savePathParam = train->add_option("--save", savePath, "Trained policy save path");

    savePathParam->required();

    auto epsilonValue { 0.0f };
    auto epsilonOptimalParam = train->add_option("--optimal", epsilonValue, "Select epsilon-optimal opponent (Default equals to random)");

    epsilonOptimalParam->check(CLI::Range(0.f,1.f));

    // Add Test subcommand
    auto test = app.add_subcommand("test", "Test a trained policy against an opponent");

    auto testResultsPlot {false};
    test->add_option("--resultsPlot", testResultsPlot, "Set results plot save path");

    test->add_option("-i", iterationsCount, "Number of iterations");

    std::string loadPath;
    auto loadPathParam = test->add_option("--load", loadPath, "Trained policy path");

    loadPathParam->required();
    loadPathParam->check(CLI::ExistingFile);

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

        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Win, 0));
        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Draw, 0));
        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Lose, 0));

        std::vector<float> cumulativeRewards;
        cumulativeRewards.reserve(iterationsCount+1);
        cumulativeRewards.emplace_back(0.f);

        const auto customFunc = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex){

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

        };

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(learningAgent),
                *opponentPtr,
                iterationsCount,
                learningAgent.GetLearningSettings().myIsAgentFirstToMove, customFunc);

        learningAgent.SetTrainingMode(false);

        // save agent to disk
        std::ofstream os(savePath);
        assert(os.is_open());

        {
            cereal::JSONOutputArchive archive(os);
            archive(CEREAL_NVP(learningAgent));
        }

        os.close();

        if(rewardPlot)
        {
            PlotCumulativeRewardFunction(cumulativeRewards, iterationsCount);
        }

        if(resultsPlot)
        {
            PlotCumulativeGameResults(iterationsCount, resultsCounter[TTT::BoardStatus::Win], resultsCounter[TTT::BoardStatus::Draw], resultsCounter[TTT::BoardStatus::Lose]);
        }

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

        std::unordered_map<TTT::BoardStatus, int> resultsCounter;

        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Win, 0));
        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Draw, 0));
        resultsCounter.insert(std::make_pair(TTT::BoardStatus::Lose, 0));

        const auto customFunc = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex){
            if(testResultsPlot)
            {
                const auto boardStatus = TTT::Utils::GetBoardStatus(trainedAgent.GetAgentId(), aGameplayHistory.back());
                ++resultsCounter[boardStatus];
            }
        };

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(trainedAgent),
                *opponentPtr,
                iterationsCount,
                trainedAgent.GetLearningSettings().myIsAgentFirstToMove, customFunc);

        if(testResultsPlot)
        {
            PlotCumulativeGameResults(iterationsCount, resultsCounter[TTT::BoardStatus::Win], resultsCounter[TTT::BoardStatus::Draw], resultsCounter[TTT::BoardStatus::Lose]);
        }

        std::cout << "Testing done" << std::endl;
    });

    // Parse command line params
    CLI11_PARSE(app, argc, argv);
}