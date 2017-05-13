# the exercise name is nothing else than the directory
get_filename_component(EXERCISENAME ${CMAKE_CURRENT_LIST_DIR} NAME)
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.h")

# these files are copied only once
file(GLOB TEXTURES
    grid/grid_texture.tga
    water/waterNormalMap.tga
    water/waterNormalMap2.tga
    water/waterNormalMap2_256.tga
    water/waterNormalMap_custom.tga
    water/waterNormalMap_perlinNoStretch.tga
    water/waterNormalMap_perlinStretch.tga
    water/waterNormalMap2_perlinStretch.tga
    water/waterNormalMap3_perlinStretch.tga
    terrain/textures/grass512.tga
    terrain/textures/rock512.tga
    terrain/textures/rock512_2.tga
    terrain/textures/rock512_3.tga
    terrain/textures/rock512_4.tga
    terrain/textures/sand256.tga
    terrain/textures/snow512.tga
    skyDome/cloud1024.tga
    skybox/bigSky/left.png
    skybox/bigSky/right.png
    skybox/bigSky/top.png
    skybox/bigSky/bottom.png
    skybox/bigSky/front.png
    skybox/bigSky/back.png)
copy_files_once(${TEXTURES})

# list all the files you want to copy everytime
# you build (ie, you want the shaders there)
file(GLOB SHADERS
    terrain/terrain_vshader.glsl
    terrain/terrain_fshader.glsl
    terrain/terrain_tcshader.glsl
    terrain/terrain_teshader.glsl
    terrain/shadow/terrain_vshader_shadow.glsl
    terrain/shadow/terrain_fshader_shadow.glsl
    terrain/shadow/terrain_tcshader_shadow.glsl
    terrain/shadow/terrain_teshader_shadow.glsl
    terrain/debug/terrain_vshader_debug.glsl
    terrain/debug/terrain_fshader_debug.glsl
    terrain/debug/terrain_tcshader_debug.glsl
    terrain/debug/terrain_teshader_debug.glsl
    terrain/debug/terrain_gshader_debug.glsl
    skybox/skybox_fshader.glsl
    skybox/skybox_vshader.glsl
    skydome/skydome_fshader.glsl
    skydome/skydome_vshader.glsl
    skydome/skyPlane_fshader.glsl
    skydome/skyPlane_vshader.glsl
    screenquad/screenquad_vshader.glsl
    screenquad/screenquad_fshader.glsl
    blurquad/blurquad_vshader.glsl
    blurquad/blurquad_fshader.glsl
    perlin/perlin_vshader.glsl
    perlin/perlin_fshader.glsl
    water/water_vshader.glsl
    water/water_fshader.glsl
    water/water_tcshader.glsl
    water/water_teshader.glsl
    water/debug/water_vshader_debug.glsl
    water/debug/water_fshader_debug.glsl
    water/debug/water_tcshader_debug.glsl
    water/debug/water_teshader_debug.glsl
    water/debug/water_gshader_debug.glsl)
    deploy_shaders_to_build_dir(${SHADERS})

add_executable(${EXERCISENAME} ${SOURCES} ${HEADERS} ${SHADERS})
target_link_libraries(${EXERCISENAME} ${COMMON_LIBS})
