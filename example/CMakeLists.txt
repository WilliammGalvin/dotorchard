add_executable(orchard_dot_example main.cpp)
target_link_libraries(orchard_dot_example PRIVATE orchard::dot)
set_target_properties(orchard_dot_example PROPERTIES
  OUTPUT_NAME example
  CXX_EXTENSIONS OFF)

target_compile_definitions(orchard_dot_example PRIVATE
  ORCHARD_EXAMPLE_FILE="${CMAKE_CURRENT_SOURCE_DIR}/cherry.orchard")

add_custom_target(run_example
  COMMAND orchard_dot_example ${CMAKE_CURRENT_SOURCE_DIR}/cherry.orchard
  DEPENDS orchard_dot_example
  USES_TERMINAL)
