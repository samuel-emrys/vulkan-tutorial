# Find external dependencies
find_package(VulkanHeaders REQUIRED)
find_package(Vulkan REQUIRED)
find_package(vulkan-validationlayers REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

file(
  GLOB_RECURSE
  LIB_SOURCES
  CONFIGURE_DEPENDS
  "*.cpp")
file(
  GLOB_RECURSE
  LIB_HEADERS
  CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/include/*.h")

# Remove main.cpp from LIB_SOURCES so that we can create a library
list(REMOVE_ITEM LIB_SOURCES ${MAIN})

# Create a library out of our source files
#add_library(${PROJECT_LIB} STATIC ${LIB_SOURCES})
#target_include_directories(${PROJECT_LIB} PUBLIC ${PROJECT_SOURCE_DIR}/include)
#set_target_properties(${PROJECT_LIB} PROPERTIES PUBLIC_HEADER "${LIB_HEADERS}")
#target_link_libraries(
#  ${PROJECT_LIB}
#  PUBLIC vulan-headers::vulkan-headers glfw glm::glm)

# Create an executable with main, and link our library
add_executable(${PROJECT_EXE} ${MAIN})
target_link_libraries(${PROJECT_EXE}
  PUBLIC 
  Vulkan::Vulkan
  vulkan-headers::vulkan-headers
  vulkan-validationlayers::vulkan-validationlayers
  glfw
  glm::glm
)

# Rename the output executable to be ${PROJECT} instead of ${PROJECT_EXE}
set_target_properties(${PROJECT_EXE} PROPERTIES OUTPUT_NAME ${PROJECT})

include(GNUInstallDirs)
install(
    #TARGETS ${PROJECT_LIB} ${PROJECT_EXE}
  TARGETS ${PROJECT_EXE}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
