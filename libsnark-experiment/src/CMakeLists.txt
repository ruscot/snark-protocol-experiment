include_directories(.)

add_executable(
  snark_polynomial_in_clear

  snark_polynomial_in_clear.cpp
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