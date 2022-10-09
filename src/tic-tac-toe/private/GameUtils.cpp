//
// Created by Gianmarco Picarella on 08/10/22.
//

#include "GameUtils.h"

namespace TTT {
    namespace Utils {

        namespace
        {
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
        }


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
    }
}
