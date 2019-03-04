cmake_minimum_required(VERSION 3.0)
project (DeltaBetaCLEngine)

set(SFML_DIR C:/Libraries/SFML-2.5.1/lib/cmake/SFML)
set(SFML_LIB_DIR C:/Libraries/SFML-2.5.1/lib)
set(SFML_STATIC_LIBRARIES TRUE)
set(SFML_INCLUDE_DIR C:/Libraries/SFML-2.5.1/include)
set(SDK_DIRECTORY "C:/Program Files (x86)/Microsoft SDKs")
set(GLM_DIR C:/Libraries/glm)
set(GLCL_DIR C:/Libraries/GLCL)
set(ANTTWEAKBAR_DIR C:/Libraries/AntTweakBar)

find_package(SFML 2.5 COMPONENTS system window graphics REQUIRED)

include_directories(${SDK_DIRECTORY}/Windows/v7.1/Include)
include_directories(${ANTTWEAKBAR_DIR}/include)
include_directories(${GLM_DIR})
include_directories(${GLCL_DIR})

add_subdirectory(src)

target_include_directories(DeltaBetaCLSources PUBLIC
  src
)


if(WIN32)
  add_executable(DeltaBetaCLEngine WIN32 src/Main.cpp src/Resource.rc assets/icon.ico)
  set_source_files_properties(icon.ico Resource.rc PROPERTIES LANGUAGE RC)
else()
  add_executable(DeltaBetaCLEngine Main.cpp)
endif()

add_custom_command(TARGET DeltaBetaCLEngine PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory
                       ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:DeltaBetaCLEngine>)


target_compile_definitions(DeltaBetaCLEngine PRIVATE SFML_STATIC)
target_link_libraries(DeltaBetaCLEngine
  sfml-system
  sfml-window
  sfml-graphics
  ${ANTTWEAKBAR_DIR}/lib/AntTweakBar.lib
  ${GLCL_DIR}/lib/x86/OpenCL.lib
  ${GLCL_DIR}/lib/x86/glew32.lib
  ${GLCL_DIR}/lib/x86/glut32.lib
  winmm
  gdi32
)
