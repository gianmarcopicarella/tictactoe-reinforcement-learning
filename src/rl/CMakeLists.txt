set(RL_PUBLIC_PATH ${CMAKE_CURRENT_LIST_DIR}/public)

add_library(RL INTERFACE)
target_include_directories(RL INTERFACE ${RL_PUBLIC_PATH})
target_link_libraries(RL INTERFACE cereal)