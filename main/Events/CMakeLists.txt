FILE(GLOB_RECURSE sources *.cpp *h *.hpp)

ADD_LIBRARY(Events OBJECT ${sources})
