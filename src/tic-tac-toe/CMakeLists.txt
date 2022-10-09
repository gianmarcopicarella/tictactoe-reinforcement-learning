set(TTT_PUBLIC_PATH public)
set(TTT_PRIVATE_PATH private)

file(   GLOB_RECURSE
        TTT_HDR
        ${TTT_PUBLIC_PATH}/*.h
        ${TTT_PRIVATE_PATH}/*.h)

file(   GLOB_RECURSE
        TTT_IMPL
        ${TTT_PUBLIC_PATH}/*.cpp
        ${TTT_PRIVATE_PATH}/*.cpp)


add_library(TTT STATIC ${TTT_HDR} ${TTT_IMPL})

target_link_libraries(TTT PRIVATE RL)

target_include_directories(TTT PUBLIC ${TTT_PUBLIC_PATH})
target_include_directories(TTT PRIVATE ${TTT_PRIVATE_PATH})