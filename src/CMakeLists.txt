cmake_minimum_required(VERSION 3.1)
project(tictactoe-rl)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_language(C CXX)

# append custom preprocessor definition when using Debug targets
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG_FLAG")

# Required to break Boost dependency in Cereal
set(SKIP_PERFORMANCE_COMPARISON ON)

# external dependencies
add_subdirectory(external/cereal)
add_subdirectory(external/cli11)
add_subdirectory(external/sciplot)
add_subdirectory(external/indicators)

# reinforcement learning base utils
add_subdirectory(rl)

# tic tac toe
add_subdirectory(tic-tac-toe)

add_executable(tictactoe-rl main.cpp)

target_link_libraries(tictactoe-rl PRIVATE TTT)
target_link_libraries(tictactoe-rl PRIVATE CLI11)
target_link_libraries(tictactoe-rl PRIVATE sciplot)
target_link_libraries(tictactoe-rl PRIVATE indicators)