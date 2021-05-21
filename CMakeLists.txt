cmake_minimum_required(VERSION 3.14)
project(basler_camera)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_VERBOSE_MAKEFILE ON)

if ("${LIBRARY_INSTALL_DESTINATION}" STREQUAL "")
   set(LIBRARY_INSTALL_DESTINATION ${CMAKE_SOURCE_DIR}/host)
endif()

if("${CUSTOM_OPENCV_BUILD_PATH}" STREQUAL "")
   find_package(OpenCV REQUIRED)
else()
   find_package(OpenCV REQUIRED PATHS ${CUSTOM_OPENCV_BUILD_PATH} NO_DEFAULT_PATH)
endif()

if (MSVC)
   set(PYLON_INCLUDE_DIRECTORIES "C:/Program Files/Basler/pylon 6/Development/include")
else()
   set(PYLON_INCLUDE_DIRECTORIES  /opt/pylon/include)
endif()

include_directories(
    include
    ${PYLON_INCLUDE_DIRECTORIES}
    ${OpenCV_INCLUDE_DIRS})

set(${PROJECT_NAME}_PUBLIC_INCLUDES include/basler_camera/BaslerCamera.hpp)

add_library(${PROJECT_NAME}
        ${${PROJECT_NAME}_PUBLIC_INCLUDES}
        src/BaslerCamera.cpp
        src/TimeMeasuring.hpp)

if(MSVC)
   set(PYLON_LIBRARIES "C:/Program Files/Basler/pylon 6/Development/lib/x64/PylonUtility_v6_1.lib"
           "C:/Program Files/Basler/pylon 6/Development/lib/x64/PylonBase_v6_1.lib"
           "C:/Program Files/Basler/pylon 6/Development/lib/x64/GenApi_MD_VC141_v3_1_Basler_pylon.lib"
           "C:/Program Files/Basler/pylon 6/Development/lib/x64/GCBase_MD_VC141_v3_1_Basler_pylon.lib")
else()
   set(PYLON_LIBRARIES -Wl,--enable-new-dtags -Wl,-rpath,/opt/pylon/lib -L/opt/pylon/lib -Wl,-E -lpylonbase -lpylonutility -lGenApi_gcc_v3_1_Basler_pylon -lGCBase_gcc_v3_1_Basler_pylon)
endif()

target_link_libraries(${PROJECT_NAME}
        ${PYLON_LIBRARIES}
        ${OpenCV_LIBS})

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${${PROJECT_NAME}_PUBLIC_INCLUDES}")

install(TARGETS ${PROJECT_NAME}
   ARCHIVE DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   LIBRARY DESTINATION ${LIBRARY_INSTALL_DESTINATION}/lib
   RUNTIME DESTINATION ${LIBRARY_INSTALL_DESTINATION}/bin
   PUBLIC_HEADER DESTINATION ${LIBRARY_INSTALL_DESTINATION}/include/${PROJECT_NAME})

add_subdirectory(test)