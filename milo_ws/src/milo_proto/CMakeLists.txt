cmake_minimum_required(VERSION 3.8)
project(milo_proto)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(Protobuf REQUIRED)

# Proto source directory and files
set(PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/proto)
set(PROTO_FILES
  ${PROTO_DIR}/RoverCommand.proto
)

# Output directory for generated code
set(PROTO_GENERATED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/generated_code)
file(MAKE_DIRECTORY ${PROTO_GENERATED_DIR})

# Generate cpp from .proto files
set(PROTO_SRCS)
set(PROTO_HDRS)

foreach(PROTO_FILE ${PROTO_FILES})
  get_filename_component(PROTO_NAME ${PROTO_FILE} NAME_WE)

  set(PROTO_SRC ${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.cc)
  set(PROTO_HDR ${PROTO_GENERATED_DIR}/${PROTO_NAME}.pb.h)

  add_custom_command(
    OUTPUT ${PROTO_SRC} ${PROTO_HDR}
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
      --proto_path=${PROTO_DIR}
      --proto_path=${Protobuf_INCLUDE_DIRS}
      --cpp_out=${PROTO_GENERATED_DIR}
      ${PROTO_FILE}
    DEPENDS ${PROTO_FILE}
    COMMENT "Generating protobuf code from ${PROTO_NAME}.proto"
  )

  list(APPEND PROTO_SRCS ${PROTO_SRC})
  list(APPEND PROTO_HDRS ${PROTO_HDR})
endforeach()

# Also compile the existing timestamp.pb.cc
list(APPEND PROTO_SRCS ${PROTO_GENERATED_DIR}/google/protobuf/timestamp.pb.cc)

# Build as shared library so the linker can find it
add_library(${PROJECT_NAME} SHARED ${PROTO_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${PROTO_GENERATED_DIR}>
  $<INSTALL_INTERFACE:include>
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  ${Protobuf_LIBRARIES}
)

# Install library
install(TARGETS ${PROJECT_NAME}
  EXPORT export_${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install generated headers (preserving directory structure)
install(DIRECTORY ${PROTO_GENERATED_DIR}/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

# Export for downstream packages
ament_export_targets(export_${PROJECT_NAME} HAS_LIBRARY_TARGET)
ament_export_include_directories(include)
ament_export_libraries(${PROJECT_NAME})
ament_export_dependencies(Protobuf)

ament_package()