add_executable(even_flop_tests
  card_test.cpp
  hand_test.cpp
  deck_test.cpp
  equity_test.cpp
  search_test.cpp
)
target_link_libraries(even_flop_tests PRIVATE even_flop_core GTest::gtest_main)
target_compile_options(even_flop_tests PRIVATE -Wall -Wextra)

include(GoogleTest)
gtest_discover_tests(even_flop_tests)
