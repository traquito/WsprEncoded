add_executable(TestWspr TestWspr.cpp)
target_include_directories(TestWspr PRIVATE .)
add_test(NAME TestWspr COMMAND TestWspr)

add_executable(TestWsprChannelMap TestWsprChannelMap.cpp)
target_include_directories(TestWsprChannelMap PRIVATE .)
add_test(NAME TestWsprChannelMap COMMAND TestWsprChannelMap)

add_executable(TestWsprMessageRegularType1 TestWsprMessageRegularType1.cpp)
target_include_directories(TestWsprMessageRegularType1 PRIVATE .)
add_test(NAME TestWsprMessageRegularType1 COMMAND TestWsprMessageRegularType1)

add_executable(TestWsprUtl TestWsprUtl.cpp)
target_include_directories(TestWsprUtl PRIVATE .)
add_test(NAME TestWsprUtl COMMAND TestWsprUtl)

