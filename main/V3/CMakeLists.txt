FILE(GLOB_RECURSE sources *.cpp *h *.hpp)

ADD_LIBRARY(V3 OBJECT ${sources})
