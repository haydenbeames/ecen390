file(GLOB cSources *.c)
file(GLOB cxxSources *.cpp)

add_library(zybo ${cSources} ${cxxSources})
