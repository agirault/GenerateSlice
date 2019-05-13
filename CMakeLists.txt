cmake_minimum_required(VERSION 3.3 FATAL_ERROR)

project(GenerateSlice)

find_package(VTK COMPONENTS
  vtkCommonCore
  vtkCommonDataModel
  vtkImagingCore
  vtkInteractionStyle
  vtkRenderingCore
  vtkRenderingFreeType
  vtkRenderingOpenGL2 REQUIRED)

message (STATUS "VTK_VERSION: ${VTK_VERSION}")
if (VTK_VERSION VERSION_LESS "8.90.0")
  # old system
  include(${VTK_USE_FILE})
  add_executable(GenerateSlice MACOSX_BUNDLE GenerateSlice.cxx )
  target_link_libraries(GenerateSlice PRIVATE ${VTK_LIBRARIES})
else ()
  # include all components
  add_executable(GenerateSlice MACOSX_BUNDLE GenerateSlice.cxx )
  target_link_libraries(GenerateSlice PRIVATE ${VTK_LIBRARIES})
  # vtk_module_autoinit is needed
  vtk_module_autoinit(
    TARGETS GenerateSlice
    MODULES ${VTK_LIBRARIES}
    )
endif ()