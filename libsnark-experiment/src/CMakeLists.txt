include_directories(.)

set(MySourcesList "snark_polynomial_in_clear.cpp" "snark_tests.cpp" "main.cpp")

add_executable(
  snark_polynomial_in_clear ${MySourcesList}
)
target_link_libraries(
  snark_polynomial_in_clear
  
  snark
)
target_include_directories(
  snark_polynomial_in_clear
  
  PUBLIC
  ${DEPENDS_DIR}/libsnark
  ${DEPENDS_DIR}/libsnark/depends/libfqfft
  ${DEPENDS_DIR}/libsnark/depends/libff
)
