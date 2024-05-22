FILE(GLOB_RECURSE sources portgen_static.cpp portgen_dynamic.cpp)

ADD_LIBRARY(PortGen OBJECT ${sources})
