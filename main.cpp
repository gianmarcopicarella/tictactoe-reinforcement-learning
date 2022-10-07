// TicTacToeRL.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <random>
#include <cmath>
#include <functional>
#include <fstream>
#include <string>
#include <set>

enum class Player
{
    Cross = 1,
    Nought
};

enum class BoardStatus
{
    Intermediate,
    Draw,
    Win,
    Lose,
};

std::string BoardToString(const uint32_t aBoard)
{
    static const char symbols[3] = { ' ', 'x', 'o' };
    constexpr auto firstWritableCharacterIndex = 15;

    std::string board
            {
                    " --- --- ---\n"
                    "|   |   |   |\n"
                    " --- --- ---\n"
                    "|   |   |   |\n"
                    " --- --- ---\n"
                    "|   |   |   |\n"
                    " --- --- ---\n"
            };

    board[firstWritableCharacterIndex] = symbols[(aBoard >> 16) & 0x3];
    board[firstWritableCharacterIndex + 4] = symbols[(aBoard >> 14) & 0x3];
    board[firstWritableCharacterIndex + 8] = symbols[(aBoard >> 12) & 0x3];

    board[firstWritableCharacterIndex + 27] = symbols[(aBoard >> 10) & 0x3];
    board[firstWritableCharacterIndex + 31] = symbols[(aBoard >> 8) & 0x3];
    board[firstWritableCharacterIndex + 35] = symbols[(aBoard >> 6) & 0x3];

    board[firstWritableCharacterIndex + 54] = symbols[(aBoard >> 4) & 0x3];
    board[firstWritableCharacterIndex + 58] = symbols[(aBoard >> 2) & 0x3];
    board[firstWritableCharacterIndex + 62] = symbols[aBoard & 0x3];

    return board;
}

BoardStatus GetBoardStatus(const Player aMovingPlayer, const uint32_t aBoard)
{
    constexpr uint32_t checkOffsets[5] = { 18, 16, 8, 0, 0 };

    const auto tl = ((aBoard >> 16) & 0x3);
    const auto cl = ((aBoard >> 10) & 0x3);
    const auto bl = ((aBoard >> 4) & 0x3);

    const auto tc = ((aBoard >> 14) & 0x3);
    const auto cc = ((aBoard >> 8) & 0x3);
    const auto bc = ((aBoard >> 2) & 0x3);

    const auto tr = ((aBoard >> 12) & 0x3);
    const auto cr = ((aBoard >> 6) & 0x3);
    const auto br = (aBoard & 0x3);

    const auto movingPlayerId = static_cast<uint32_t>(aMovingPlayer);
    const auto otherPlayerId = (~movingPlayerId) & 0x3;

    const uint32_t verticalShiftIndex = (tl > 0 && tl == cl && cl == bl) |
                                        ((tc > 0 && tc == cc && cc == bc) << 1) |
                                        ((tr > 0 && tr == cr && cr == br) << 2);

    assert((
                   verticalShiftIndex == 0 ||
                   verticalShiftIndex == 1 ||
                   verticalShiftIndex == 2 ||
                   verticalShiftIndex == 4) && "More than one vertical Tris found!");

    const auto verticalCheck = ((aBoard >> checkOffsets[verticalShiftIndex]) & 0x3);

    if (verticalCheck == movingPlayerId)
    {
        return BoardStatus::Win;
    }
    else if (verticalCheck == otherPlayerId)
    {
        return BoardStatus::Lose;
    }

    const uint32_t horizontalShiftIndex = (tl > 0 && tl == tc && tc == tr) |
                                          ((cl > 0 && cl == cc && cc == cr) << 1) |
                                          ((bl > 0 && bl == bc && bc == br) << 2);

    assert((
                   horizontalShiftIndex == 0 ||
                   horizontalShiftIndex == 1 ||
                   horizontalShiftIndex == 2 ||
                   horizontalShiftIndex == 4) && "More than one horizontal Tris found!");

    const auto horizontalCheck = ((aBoard >> checkOffsets[horizontalShiftIndex]) & 0x3);

    if (horizontalCheck == movingPlayerId)
    {
        return BoardStatus::Win;
    }
    else if (horizontalCheck == otherPlayerId)
    {
        return BoardStatus::Lose;
    }

    const uint32_t diagonalsShiftIndex = (tl > 0 && tl == cc && cc == br) |
                                         ((tr > 0 && tr == cc && cc == bl) << 1);

    assert((
                   diagonalsShiftIndex >= 0 &&
                   diagonalsShiftIndex < 4));

    const auto diagonalsCheck = ((aBoard >> checkOffsets[diagonalsShiftIndex]) & 0x3);

    if (diagonalsCheck == movingPlayerId)
    {
        return BoardStatus::Win;
    }
    else if (diagonalsCheck == otherPlayerId)
    {
        return BoardStatus::Lose;
    }

    if (tl > 0 && cl > 0 && bl > 0 && tc > 0 && cc > 0 && bc > 0 && tr > 0 && cr > 0 && br > 0)
    {
        return BoardStatus::Draw;
    }

    return BoardStatus::Intermediate;
}

std::vector<uint32_t> GenerateMoves(const Player aPlayerToMove, const uint32_t aCurrentBoard)
{
    assert(GetBoardStatus(aPlayerToMove, aCurrentBoard) == BoardStatus::Intermediate &&
           "Cannot generate moves from a full board");

    std::vector<uint32_t> moves;

    for (auto positionIndex = 0; positionIndex < 18; positionIndex += 2)
    {
        if (((aCurrentBoard >> positionIndex) & 0x3) == 0)
        {
            const auto modifiedBoard = aCurrentBoard | (static_cast<uint32_t>(aPlayerToMove) << positionIndex);
            moves.push_back(modifiedBoard);
        }
    }

    return moves;
}

void RecursiveBoardsGeneration(const Player anAgentPlayer, const Player aPlayerToMove, const uint32_t aCurrentBoard, std::set<uint32_t>& someOutValidBoards)
{
    for (const auto& modifiedBoard : GenerateMoves(aPlayerToMove, aCurrentBoard))
    {
        const auto boardStatus = GetBoardStatus(anAgentPlayer, modifiedBoard);

        if (aPlayerToMove == anAgentPlayer)
        {
            someOutValidBoards.insert(modifiedBoard);
        }

        if (boardStatus == BoardStatus::Intermediate)
        {
            const auto nextPlayerToMove = static_cast<Player>((~static_cast<uint32_t>(aPlayerToMove)) & 0x3);
            RecursiveBoardsGeneration(anAgentPlayer, nextPlayerToMove, modifiedBoard, someOutValidBoards);
        }
    }
}

void GenerateBoards(const Player anAgentPlayer, std::set<uint32_t>& someOutValidBoards)
{
    constexpr auto startingBoard = 0x00000000;

    RecursiveBoardsGeneration(anAgentPlayer, Player::Cross, startingBoard, someOutValidBoards);

    // Check that the total number of legal boards is correct
    std::set<uint32_t> fullBoardsSpace;

    RecursiveBoardsGeneration(anAgentPlayer, Player::Cross, startingBoard, fullBoardsSpace);

    // Compute the other player
    const auto otherPlayer = static_cast<Player>((~static_cast<uint32_t>(anAgentPlayer)) & 0x3);

    RecursiveBoardsGeneration(otherPlayer, Player::Cross, startingBoard, fullBoardsSpace);

    assert(fullBoardsSpace.size() == 5477 && "The number of generated boards is not correct");

    // Check that the total number of end games is correct
    const auto endgamesCount = std::count_if(fullBoardsSpace.begin(), fullBoardsSpace.end(), [&](const auto aBoard) {
        return GetBoardStatus(Player::Cross, aBoard) != BoardStatus::Intermediate;
    });

    assert(endgamesCount == 958 && "The number of generated end game boards is not correct");
}

void SerializeBoards(const std::string& aPath, const std::set<uint32_t>& someValidBoards)
{
    std::ofstream file;

    file.open(aPath);

    assert(file.is_open() && "Cannot open file");

    for (const auto& board : someValidBoards)
    {
        file << board << "," << static_cast<int>(GetBoardStatus(Player::Cross, board)) << std::endl;
    }

    file.close();
}

// --------------------------------------------------------
template<typename ActionStatus>
struct BaseLearningSettings
{
    int myTrainEpisodesCount = 0;
    int myTestingEpisodesCount = 0;
    float myLearningRate = 0.0f;
    bool myIsTraining = false;

    using StaticScoresMap = std::unordered_map<ActionStatus, float>;

    StaticScoresMap myStaticScores;
};

template<typename ActionStatus>
struct QLearningSettings : public BaseLearningSettings<ActionStatus>
{
    float myGamma = 0.0f;
    float myGreedyEpsilon = 0.0f;
    float myGreedyEpsilonDecay = 0.0f;
};

template <typename AgentId, typename Action, typename State>
struct Agent
{
    Agent() = delete;
    Agent(const AgentId& anAgentId) : myId(anAgentId) {}

    const AgentId& GetAgentId() const { return myId; }
    virtual Action GetNextAction(const State& aCurrentState) = 0;

protected:
    AgentId myId;
};

template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
struct LearningPolicy : public Agent<AgentId, State, Action>
{
    using Base = Agent<AgentId, State, Action>;

    LearningPolicy() = delete;
    LearningPolicy(const AgentId& anAgentId, const LearningSettings& aLearningSettings) : Base(anAgentId), myLearningSettings(aLearningSettings)
    {
        static_assert(std::is_base_of<BaseLearningSettings<ActionStatus>, LearningSettings>::value,
                      "[LearningPolicy]: LearningSettings type is not a child type of BaseLearningSettings");
    }

    const LearningSettings& GetLearningSettings() const { return myLearningSettings; }

    void SetTrainingMode(bool aTrainingFlag)
    {
        myLearningSettings.myIsTraining = aTrainingFlag;
    }

    virtual void Update(const std::vector<State> aGameplayHistory) = 0;

protected:
    LearningSettings myLearningSettings;
};


template<typename AgentId, typename State, typename Action, typename LearningSettings, typename ActionStatus>
struct GreedyLearner : public LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus>
{
    using Base = LearningPolicy<AgentId, State, Action, LearningSettings, ActionStatus>;

public:
    GreedyLearner() = delete;

    GreedyLearner(const AgentId& anAgentId, const LearningSettings& aLearningSettings) :
            Base(anAgentId, aLearningSettings)
    {}

    Action GetNextAction(const State& aCurrentState)
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        Action result;

        if (Base::myLearningSettings.myIsTraining)
        {
            std::uniform_real_distribution<> uniFltDistr(0.f, 1.f);

            if (uniFltDistr(rng) < Base::myLearningSettings.myGreedyEpsilon)
            {
                result = ExplorationJob(aCurrentState);

            }
            else
            {
                result = GreedyJob(aCurrentState);
            }

            Base::myLearningSettings.myGreedyEpsilon *= (1.f - Base::myLearningSettings.myGreedyEpsilonDecay);
        }
        else
        {
            result = GreedyJob(aCurrentState);
        }

        return result;
    }

protected:
    using ActionValueScoresMap = std::unordered_map<Action, float>;

    ActionValueScoresMap myActionValueScores;

    std::function<Action(const State&, const std::mt19937&)> myExploratoryAction;
    std::function<Action(const State&, const std::mt19937&)> myGreedyAction;

    virtual Action ExplorationJob(const State& aCurrentState) const = 0;
    virtual Action GreedyJob(const State& aCurrentState) const = 0;
};


struct TicTacToeQLearner : public GreedyLearner<Player, uint32_t, uint32_t, QLearningSettings<BoardStatus>, BoardStatus>
{
    using Base = GreedyLearner<Player, uint32_t, uint32_t, QLearningSettings<BoardStatus>, BoardStatus>;

public:
    TicTacToeQLearner() = delete;

    TicTacToeQLearner(const Player& anAgentId, const QLearningSettings<BoardStatus>& aLearningSettings) :
            Base(anAgentId, aLearningSettings)
    {
        std::set<uint32_t> validBoardStates;
        GenerateBoards(myId, validBoardStates);

        for (auto boardState : validBoardStates)
        {
            const auto boardScore = myLearningSettings.myStaticScores[GetBoardStatus(myId, boardState)];
            myActionValueScores.insert(std::make_pair(boardState, boardScore));
        }
    }

    void Update(const std::vector<uint32_t> aGameplayHistory)
    {
        const auto boardStatus = GetBoardStatus(myId, aGameplayHistory.back());

        assert(boardStatus != BoardStatus::Intermediate);

        const auto isLastMoveFromAgent = boardStatus == BoardStatus::Win ||
                                         (boardStatus == BoardStatus::Draw && myId == Player::Cross);

        if (!isLastMoveFromAgent)
        {
            // Update last move from agent
            const auto& agentMove = aGameplayHistory[aGameplayHistory.size() - 2];

            const auto reward = myLearningSettings.myStaticScores[boardStatus]/* myActionValueScores[aGameplayHistory.back()]*/;

            myActionValueScores[agentMove] += myLearningSettings.myLearningRate * (reward - myActionValueScores[agentMove]);
        }

        const auto startingMoveIndex = isLastMoveFromAgent ?
                                       aGameplayHistory.size() - 3 : aGameplayHistory.size() - 4;

        for (int moveIndex = startingMoveIndex; moveIndex > -1; moveIndex -= 2)
        {
            const auto& nextState = aGameplayHistory[moveIndex + 1];

            const auto nextAgentMoves = GenerateMoves(myId, nextState);

            assert(nextAgentMoves.size() > 0);

            const uint32_t maxValueMove = *std::max_element(nextAgentMoves.begin(), nextAgentMoves.end(), [&](auto& m1, auto& m2) {
                assert(myActionValueScores.find(m1) != myActionValueScores.end());
                assert(myActionValueScores.find(m2) != myActionValueScores.end());

                return myActionValueScores.find(m1)->second < myActionValueScores.find(m2)->second;
            });

            const auto agentMove = aGameplayHistory[moveIndex];

            assert(myActionValueScores.find(agentMove) != myActionValueScores.end());

            myActionValueScores[agentMove] += myLearningSettings.myLearningRate *
                                              (myLearningSettings.myGamma * myActionValueScores[maxValueMove] - myActionValueScores[agentMove]);
        }
    }

    void SerializeActionValueScoresMap(const std::string& aPath)
    {
        std::ofstream outStream{ aPath };

        assert(outStream.is_open());

        for (auto& actionValueEntry : myActionValueScores)
        {
            outStream << actionValueEntry.first << ", " << actionValueEntry.second << std::endl;
        }

        outStream.close();
    }

protected:
    uint32_t ExplorationJob(const uint32_t& aCurrentState) const
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextAgentMoves = GenerateMoves(myId, aCurrentState);

        assert(nextAgentMoves.size() > 0);

        std::uniform_int_distribution<> uniIntDistr(0, nextAgentMoves.size() - 1);

        return nextAgentMoves[uniIntDistr(rng)];
    }

    uint32_t GreedyJob(const uint32_t& aCurrentState) const
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextAgentMoves = GenerateMoves(myId, aCurrentState);

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
};

struct RandomOpponent : public Agent<Player, uint32_t, uint32_t>
{
    using Base = Agent<Player, uint32_t, uint32_t>;

    RandomOpponent(const Player& aTrainerId) : Base(aTrainerId) {}

    virtual uint32_t GetNextAction(const uint32_t& aCurrentState)
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        const auto nextMoves = GenerateMoves(myId, aCurrentState);

        assert(nextMoves.size() > 0);

        std::uniform_int_distribution<> uniIntDistr(0, nextMoves.size() - 1);

        return nextMoves[uniIntDistr(rng)];
    }
};

struct EpsilonOptimalOpponent : public Agent<Player, uint32_t, uint32_t>
{
    using Base = Agent<Player, uint32_t, uint32_t>;

    EpsilonOptimalOpponent(const Player& aTrainerId, float aRandomEpsilon) : Base(aTrainerId), myRandomEpsilon(aRandomEpsilon) {}

    virtual uint32_t GetNextAction(const uint32_t& aCurrentState)
    {
        static std::random_device dev;
        static std::mt19937 rng(dev());

        auto nextMoves = GenerateMoves(myId, aCurrentState);

        assert(nextMoves.size() > 0);

        std::uniform_real_distribution<> uniFltDistr(0.f, 1.f);

        if (uniFltDistr(rng) < myRandomEpsilon)
        {
            std::uniform_int_distribution<> uniIntDistr(0, nextMoves.size() - 1);
            return nextMoves[uniIntDistr(rng)];
        }
        else
        {
            const auto nextPlayer = static_cast<Player>((~static_cast<uint32_t>(myId)) & 0x3);

            std::vector<std::pair<uint32_t, int>> nextMovesScores;

            for (const auto nextMove : nextMoves)
            {
                nextMovesScores.push_back(std::make_pair(nextMove, TicTacToeMinimax(nextMove, nextPlayer)));
            }

            std::sort(nextMovesScores.begin(), nextMovesScores.end(), [](auto& p1, auto& p2) {
                return p1.second > p2.second;
            });

            const auto equalMinMovesCount = std::count_if(nextMovesScores.begin(), nextMovesScores.end(), [&](const auto& p) {
                return nextMovesScores[0].second == p.second;
            });

            std::uniform_int_distribution<> uniIntDistr(0, equalMinMovesCount - 1);

            return nextMovesScores[uniIntDistr(rng)].first;
        }
    }

private:
    float myRandomEpsilon;

    int TicTacToeMinimax(const uint32_t aBoard, const Player aPlayer)
    {
        switch (GetBoardStatus(myId, aBoard))
        {
            case BoardStatus::Win: return 1;
            case BoardStatus::Draw: return 0;
            case BoardStatus::Lose: return -1;
        }

        const auto nextPlayer = static_cast<Player>((~static_cast<uint32_t>(aPlayer)) & 0x3);

        if (aPlayer == myId)
        {
            auto  value = std::numeric_limits<int>::min();

            for (const auto nextMove : GenerateMoves(aPlayer, aBoard))
            {
                value = std::max(value, TicTacToeMinimax(nextMove, nextPlayer));
            }

            return value;
        }
        else
        {
            auto  value = std::numeric_limits<int>::max();

            for (const auto nextMove : GenerateMoves(aPlayer, aBoard))
            {
                value = std::min(value, TicTacToeMinimax(nextMove, nextPlayer));
            }

            return value;
        }
    }
};

template <typename LearningSettings>
void Simulate(LearningPolicy<Player, uint32_t, uint32_t, LearningSettings, BoardStatus>& aLearningAgent,
              Agent<Player, uint32_t, uint32_t>& aTrainerAgent,
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
        Player player = aFirstMoveFromLearnerFlag ? aLearningAgent.GetAgentId() : aTrainerAgent.GetAgentId();

        while (GetBoardStatus(player, board) == BoardStatus::Intermediate)
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
            player = static_cast<Player>((~static_cast<uint32_t>(player)) & 0x3);
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
    QLearningSettings<BoardStatus> QLSettings;

    QLSettings.myGamma = 0.9f;
    QLSettings.myGreedyEpsilon = 0.3f;
    QLSettings.myGreedyEpsilonDecay = 0.000001f;
    QLSettings.myLearningRate = 0.5f;

    QLSettings.myStaticScores.insert(std::make_pair(BoardStatus::Win, 1.f));
    QLSettings.myStaticScores.insert(std::make_pair(BoardStatus::Draw, 0.f));
    QLSettings.myStaticScores.insert(std::make_pair(BoardStatus::Lose, -1.f));
    QLSettings.myStaticScores.insert(std::make_pair(BoardStatus::Intermediate, 0.5f));

    QLSettings.myTrainEpisodesCount = 100000;
    QLSettings.myTestingEpisodesCount = 100000;
    QLSettings.myIsTraining = true;

    // Create the learning agent
    TicTacToeQLearner learningAgent{ Player::Cross, QLSettings };

    // Create the random opponent
    RandomOpponent randomOpponent{ Player::Nought };

    // Create the mixed random-optimal opponent with custom probability
    EpsilonOptimalOpponent optimalOpponent{ Player::Nought, 0.3f };

    // Create a custom episode callback
    int win = 0, draw = 0, lose = 0;

    const auto episodeCallback = [&](const std::vector<uint32_t>& aGameplayHistory, int anEpisodeIndex) {
        switch (GetBoardStatus(learningAgent.GetAgentId(), aGameplayHistory.back()))
        {
            case BoardStatus::Win:
                win += 1;
                break;;
            case BoardStatus::Lose:
                lose += 1;
                break;
            case BoardStatus::Draw:
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
            static_cast<TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<EpsilonOptimalOpponent::Base&>(optimalOpponent),
            true,
            episodeCallback);

    // Testing Phase
    std::cout << "\n!!! TESTING PHASE !!!" << std::endl;

    learningAgent.SetTrainingMode(false);

    // Serialize
    learningAgent.SerializeActionValueScoresMap("policy.txt");

    win = 0, draw = 0, lose = 0;

    Simulate(
            static_cast<TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<EpsilonOptimalOpponent::Base&>(optimalOpponent),
            true,
            episodeCallback);

    std::cout <<
              (win / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Wins\n" <<
              (draw / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Draws\n" <<
              (lose / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Loses\n\n";

    // Random

    win = 0, draw = 0, lose = 0;

    Simulate(
            static_cast<TicTacToeQLearner::Base::Base&>(learningAgent),
            static_cast<EpsilonOptimalOpponent::Base&>(randomOpponent),
            true,
            episodeCallback);

    std::cout <<
              (win / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Wins\n" <<
              (draw / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Draws\n" <<
              (lose / static_cast<float>(QLSettings.myTestingEpisodesCount)) << "% Loses\n";
}