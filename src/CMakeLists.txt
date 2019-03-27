add_library(DeltaBetaCLSources
  Engine.h
  Engine.cpp

  Utilities.h
  Utilities.cpp

  OpenCL.h
  CLFunction.h
  CLRender.h
  CLRender.cpp

  Camera.h
  Camera.cpp

  PhysicsEngine.h
  PhysicsEngine.cpp

  World.h
  World.cpp

  BVH.h
  BVH.cpp
)

add_compile_definitions(SFML_STATIC)