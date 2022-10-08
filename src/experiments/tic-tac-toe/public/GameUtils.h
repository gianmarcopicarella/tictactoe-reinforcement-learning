//
// Created by Gianmarco Picarella on 08/10/22.
//

#ifndef RLEXPERIMENTS_GAMEUTILS_H
#define RLEXPERIMENTS_GAMEUTILS_H

#include <string>
#include <cstdint>
#include <set>
#include <vector>

#include "PlayerEnum.h"
#include "BoardStatusEnum.h"

namespace TTT
{
namespace Utils
{
std::string BoardToString(const uint32_t aBoard);

BoardStatus GetBoardStatus(const Player aMovingPlayer, const uint32_t aBoard);

std::vector<uint32_t> GenerateMoves(const Player aPlayerToMove, const uint32_t aCurrentBoard);

void RecursiveBoardsGeneration(const Player anAgentPlayer, const Player aPlayerToMove, const uint32_t aCurrentBoard, std::set<uint32_t>& someOutValidBoards);

void GenerateBoards(const Player anAgentPlayer, std::set<uint32_t>& someOutValidBoards);
}
}

#endif //RLEXPERIMENTS_GAMEUTILS_H
