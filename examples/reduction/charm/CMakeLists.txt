
set(SRC
    ../ReductionCallbacks.h
    ../ReductionGraph.h
    ../ReductionGraph.cpp
    main.cpp
)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
    
add_charm_module(reduction)
add_charm_executable(reduction reduction ${SRC})
target_link_libraries(reduction ${BABELFLOW_LIBRARIES})

    
INSTALL(TARGETS reduction
        DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

