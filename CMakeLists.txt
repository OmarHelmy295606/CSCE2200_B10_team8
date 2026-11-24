cmake_minimum_required(VERSION 3.10)
project(BooleanMinimizerBDD CXX)
set(CMAKE_CXX_STANDARD 17)

# the program
add_executable(bfm
    main.cpp
    src/Expression.cpp
    src/BooleanFunctionMinimizer.cpp
    src/ds_Bitset.cpp
    src/BDDEngine.cpp
    src/BDDStorage.cpp
    src/BDDCount.cpp
)

# the tests
add_executable(test_ds_Bitset tests/test_ds_Bitset.cpp src/ds_Bitset.cpp)
add_executable(test_ds_Heap tests/test_ds_Heap.cpp)
add_executable(test_ds_stack tests/test_ds_stack.cpp)
add_executable(test_Expression tests/test_Expression.cpp src/Expression.cpp src/ds_Bitset.cpp)
add_executable(test_BFM tests/test_BFM.cpp src/BooleanFunctionMinimizer.cpp src/ds_Bitset.cpp)
add_executable(test_BDDCount tests/test_BDDCount.cpp src/BDDCount.cpp src/BDDStorage.cpp src/ds_Bitset.cpp
)