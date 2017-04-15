# the exercise name is nothing else than the directory
get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# these files are copied only once
file(GLOB TEXTURES
    grid/grid_texture.tga)
copy_files_once(${TEXTURES})

# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
  grid/grid_vshader.glsl
  grid/grid_fshader.glsl
  grid/grid_tcshader.glsl
  grid/grid_teshader.glsl
  screenquad/screenquad_vshader.glsl
  screenquad/screenquad_fshader.glsl
  perlin/perlin_vshader.glsl
  perlin/perlin_fshader.glsl
  normalmap/normalmap_vshader.glsl
  normalmap/normalmap_fshader.glsl
  water/water_vshader.glsl
  water/water_fshader.glsl
  water/water_tcshader.glsl
  water/water_teshader.glsl
  water/water_vshader_debug.glsl
  water/water_fshader_debug.glsl
  water/water_tcshader_debug.glsl
  water/water_teshader_debug.glsl
  water/water_gshader_debug.glsl)
deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})
