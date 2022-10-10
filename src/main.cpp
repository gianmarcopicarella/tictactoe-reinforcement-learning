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
#include <indicators/block_progress_bar.hpp>

#include <matplot/matplot.h>

void PlotData(const std::unordered_map<TTT::BoardStatus, int>& someResultsCount, const std::vector<float>& someCumulativeRewards)
{
    using namespace matplot;

    tiledlayout(2, 1);

    gcf()->width(800);
    gcf()->height(800);
    gcf()->reactive_mode();

    auto top = nexttile();

    bar(top, std::vector {
            someResultsCount.find(TTT::BoardStatus::Win)->second,
            someResultsCount.find(TTT::BoardStatus::Draw)->second,
            someResultsCount.find(TTT::BoardStatus::Lose)->second });

    top->title("Episodes' results");
    top->x_axis().ticklabels({"Wins", "Draws", "Loses"});
    top->y_axis().label("N. Episodes");

    auto bottom = nexttile();

    auto crfPlot = plot(bottom, someCumulativeRewards);

    bottom->title("Cumulative Reward Function");
    bottom->x_axis().ticklabels({"N. Episodes"});
    bottom->y_axis().label("Cumulative Reward");

    crfPlot->line_width(1.f);


    show();
}

int main(int argc, char **argv)
{
    using namespace indicators;
    show_console_cursor(false);

    CLI::App cli;

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

    // Train
    auto trainingOption = cli.add_flag("-t", agentSettings.myIsTraining, "Train a new agent");
    cli.add_option("-i", iterationsCount, "Number of iterations");

    std::vector<float> rewards;
    auto rewardsOption = cli.add_option("-r", rewards, "Reward values (Win, Draw, Lose, Intermediate)");

    rewardsOption->expected(4);
    rewardsOption->needs(trainingOption);

    std::vector<float> learningSettings;
    auto learningSettingsOption = cli.add_option("-l", learningSettings, "Learning settings (Gamma, Epsilon, EpsilonDecay, LearningRate)");

    learningSettingsOption->expected(4);
    learningSettingsOption->check(CLI::Range(0.f,1.f));
    learningSettingsOption->needs(trainingOption);

    auto isAgentNought { false };
    auto shouldPlot { false };

    auto agentSideOption = cli.add_flag("--nought", isAgentNought, "Agent side is nought");
    auto agentDelayOption = cli.add_flag("--delay", agentSettings.myIsAgentDelayed, "Delay first agent move");

    cli.add_flag("--plot", shouldPlot, "Plot cumulative reward and episodes' results");

    agentSideOption->needs(trainingOption);
    agentDelayOption->needs(trainingOption);

    std::string agentPath;
    auto agentPathOption = cli.add_option("--path", agentPath, "Agent save/load path");

    agentPathOption->required();

    auto epsilonValue { 0.0f };
    auto epsilonOptimalParam = cli.add_option("--optimal", epsilonValue, "Select epsilon-optimal opponent (Default equals to random)");

    epsilonOptimalParam->check(CLI::Range(0.f,1.f));

    BlockProgressBar cliProgressBar {
            option::BarWidth{80},
            option::Start{"["},
            option::End{"]"},
            option::ForegroundColor{Color::white},
            option::FontStyles{std::vector<FontStyle>{FontStyle::bold}}
    };

    cli.callback([&]() {
        const auto opponentSide = isAgentNought ? TTT::Player::Cross : TTT::Player::Nought;
        const auto agentSide = isAgentNought ? TTT::Player::Nought : TTT::Player::Cross;

        TTT::TicTacToeQLearner* agentPtr;
        RL::Agent<TTT::Player, uint32_t, uint32_t>* opponentPtr;

        if(epsilonOptimalParam->empty())
        {
            opponentPtr = new TTT::RandomOpponent{opponentSide};
        }
        else
        {
            opponentPtr = new TTT::EpsilonOptimalOpponent{opponentSide, epsilonValue};
        }

        if(agentSettings.myIsTraining)
        {
            cliProgressBar.set_option(option::PostfixText{"Training agent"});

            if(!rewards.empty())
            {
                agentSettings.myStaticScores[TTT::BoardStatus::Win] = rewards[0];
                agentSettings.myStaticScores[TTT::BoardStatus::Draw] = rewards[1];
                agentSettings.myStaticScores[TTT::BoardStatus::Lose] = rewards[2];
                agentSettings.myStaticScores[TTT::BoardStatus::Intermediate] = rewards[3];
            }

            agentPtr = new TTT::TicTacToeQLearner { agentSide, agentSettings };
        }
        else
        {
            cliProgressBar.set_option(option::PostfixText{"Testing agent"});

            std::ifstream deserializePath(agentPath);
            assert(deserializePath.is_open() && "Failed to open the deserialization stream");

            {
                cereal::JSONInputArchive jsonArchive(deserializePath);
                agentPtr = new TTT::TicTacToeQLearner{};
                jsonArchive(*agentPtr);
            }

            deserializePath.close();

            agentPtr->SetTrainingMode(false);
        }

        std::unordered_map<TTT::BoardStatus, int> resultsCounter;
        std::vector<float> cumulativeRewards;

        const auto episodeCallback = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex){
            if(shouldPlot)
            {
                const auto boardStatus = TTT::Utils::GetBoardStatus(agentPtr->GetAgentId(), aGameplayHistory.back());
                const auto reward = agentPtr->GetLearningSettings().myStaticScores.find(boardStatus)->second;

                cumulativeRewards.emplace_back(cumulativeRewards.back() + reward);
                ++resultsCounter[boardStatus];
            }
            cliProgressBar.set_progress(100*(anEpisodeIndex+1)/static_cast<float>(iterationsCount));
        };

        if(shouldPlot)
        {
            cumulativeRewards.reserve(iterationsCount+1);
            cumulativeRewards.emplace_back(0.f);

            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Win, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Draw, 0));
            resultsCounter.insert(std::make_pair(TTT::BoardStatus::Lose, 0));
        }

        TTT::Utils::Simulate(
                static_cast<TTT::TicTacToeQLearner::Base::Base&>(*agentPtr),
                *opponentPtr,
                iterationsCount,
                !agentPtr->GetLearningSettings().myIsAgentDelayed,
                episodeCallback);

        cliProgressBar.set_option(option::PostfixText {"Done ✔"});
        cliProgressBar.mark_as_completed();

        if(shouldPlot)
        {
            PlotData(resultsCounter, cumulativeRewards);
        }

        if(agentSettings.myIsTraining)
        {
            std::ofstream serializeStream(agentPath);
            assert(serializeStream.is_open() && "Failed to open the serialization stream");

            {
                cereal::JSONOutputArchive archive(serializeStream);
                archive(CEREAL_NVP(*agentPtr));
            }

            serializeStream.close();
        }

        assert(agentPtr != nullptr && opponentPtr != nullptr && "Agent or Opponent pointers cannot be nullptr");

        delete agentPtr;
        delete opponentPtr;

    });

    CLI11_PARSE(cli, argc, argv);
    indicators::show_console_cursor(true);
}