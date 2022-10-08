cmake_minimum_required(VERSION 3.1)
project(RLExperiments)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_language(C CXX)

# Required to break Boost dependency in Cereal
set(SKIP_PERFORMANCE_COMPARISON ON)

# external dependencies
add_subdirectory(external/cereal)
add_subdirectory(external/cli11)

# reinforcement learning base utils
add_subdirectory(rl)

# tic tac toe experiment
add_subdirectory(experiments/tic-tac-toe)

add_executable(RLExperiments main.cpp)

target_link_libraries(RLExperiments PRIVATE RL)
target_link_libraries(RLExperiments PRIVATE TTT)