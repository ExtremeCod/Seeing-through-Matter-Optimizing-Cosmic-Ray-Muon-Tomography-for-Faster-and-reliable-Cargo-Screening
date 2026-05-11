#          //|                                                    
#         // |                                                    
#        //  |  ||  ______  ||  ______       _______              
#       //   |  || /        || /      \    //       \    ||      /
#      //    |  ||/         ||/        \             |   ||     / 
#     ///////|  ||          ||         |             |   ||    /  
#    //      |  ||          ||         |    _________|   ||   /   
#   //       |  ||          ||         |  //         \   ||  /    
#  //        |  ||          ||         |  ||         |   || /     
# //         |  ||          ||         |  \\_________/\  ||/       

cmake_minimum_required(VERSION 4.1 FATAL_ERROR)

project(Proj)

find_package(Geant4 REQUIRED ui_all vis_all)

include(${Geant4_USE_FILE})

file(GLOB sources ${PROJECT_SOURCE_DIR}/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/*.hh)

file(GLOB MACRO_FILES
    "*.mac"
)

file(COPY ${MACRO_FILES} DESTINATION ${PROJECT_BINARY_DIR})

add_executable(sim sim.cc ${sources} ${headers})
target_link_libraries(sim ${Geant4_LIBRARIES})

add_custom_target(Proj DEPENDS sim)