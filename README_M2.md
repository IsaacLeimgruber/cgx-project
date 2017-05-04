# Milestone 2 - Texturing and water

<img src="http://lkieliger.ch/docs/pictures/cgx/fullRender.jpg"/>

## Contents:

1. Texturing
2. Skybox
3. Enhanced water behaviour
4. Shadowmaps
5. Infinite terrain generation
6. Geometry shaders
7. Work distribution

## 1. Texturing

First, we imported the textures in our terrain.h, and passed them to the fshader in a way similar to what we learnt in the labs.

Our last implementation used to color our terrain depending on its height by using vec3 constants such as GRASS_COLOR, ROCK_COLOR, etc. We simply replaced the declaration of our colors by an assignement of texture.
`vec3 GRASS_COLOR = vec3(r, g, b);`
became
`vec3 GRASS_COLOR = 255.0 * texture(grass_texture, uv).rgb;`
Which displayed a nice grass texture where a green color used to be. Doing the same for other colors, we obtained a full-textured terrain.

## 2. Skybox

We started by declaring the skybox vertices that correspond to the vertices constituing each face of the skybox. Since we have 6 faces, and that each face need 6 vertices (to define the two primitive triangles), we end up with 36 vertices.

Then we loaded the 6 skybox textures, one for each face.

Finally, to display the skybox, we modify the view matrix used to render the skybox so that it does not induces a translation. This can be done by truncating the original view matrix into a 3x3 matrix and then building back a new 4x4 matrix with only the bottom left component set to one. We also disable depth writing so that the skybox appears behind all other objects in our scene.


## 3. Enhanced water behaviour

<img src="http://lkieliger.ch/docs/pictures/cgx/waveOnShore.jpg"/>

To add some dynamism to our scene, we decided to animate the water. The animation is composed of  the following elements:

First, we use a sum of 5 sinusoïdal waves to alter the height of the vertices forming the water mesh. The waves can be easily parametrized in the fragment shader as we took care of separating the wave computation algorithm from the actual values used to compute the result. The parameters that can be modified are the amplitude, the wavelength, the speed, the sharpness and also the direction of each wave.

Because we use 5 sinusoïds to generate the waves, the wave equation can easily (while being a bit cumbersome) be differentiated to give analytic results for the surface normal computation. Therefore, we compute the wave and its normal at the same time.

However, due to discretization of the grid mesh used the represent the water, there is a limit to the wavelength of the waves we can represent using this technique. This is where the next step comes into play.

The calculated normals are passed on to the fragment shader. In this shader, we sample a normal map which is then used to alter the original wave normal according to the value returned by the texture sampling. This allows us to display very fine details and ripples on the surface of the water.

The final normal, consisting of the original wave normal and the additional precomputed normal, is used to compute a sampling offset for the reflection texture. This way we can simulate the effect the waves have on the portion of the reflection that is visible.

Finally, we decided mimic the behavior of the waves near the shoreline by increasing the their amplitude when they meet cliffs and beaches.

#### 3.1 Optimization
Because computing waves can be expensive, we extended our tesselation algorithm to the wave mesh and added an important condition on it:

All the water fragment below the terrain surface are simply dropped for obvious reasons. This allows for nice computation time savings.


## 4. Shadowmaps

<img src="http://lkieliger.ch/docs/pictures/cgx/mountainShadows.jpg"/>

We compute the rendering of the terrain through the view of the light source and only store the depth of each fragment in a depth texture.

<img src="http://lkieliger.ch/docs/pictures/cgx/depthTexture.png"/>

Because of the limited resolution of the depth texture, ugly artifacts appeared on our scene:

1. Black and white stripes almost everywhere due to the discretization of the depth value of each fragment used to compute the depth texture.
2. Aliasing when the light rays hit the terrain with a large incidence angle.
3. Aliasing at the edges of the shadows.

<img src="http://lkieliger.ch/docs/pictures/cgx/noPCF.jpg"/>

We reduced the impact of those artifacts in the following way:

#### 4.1 Depth bias
Introduce a bias when comparing the depth value of the currently being rendered fragment and the depth value stored in the depth map. This allows to remove most of the black and white stripes that appear on the scene.

#### 4.2 Angle dependent bias
To diminishes the aliasing effect we modify the value of the bias based on the angle made between the surface normal and the light direction. The closer the angle to 90 degrees, the higher the bias is.

While this is not a perfect solution (as aliasing effect still occurs), when it is used in conjunciton with a diffuse shadin this produces very smooth luminosity transitions even for high incidence angles.

<img src="http://lkieliger.ch/docs/pictures/cgx/shadowOnMountains.jpg"/>

#### 4.3 PCF and rotated poisson disk sampling
Aliasing at the edge can be diminished by computing the average of the depth values around a certain point. This can first be done in hardware through a method called Percentage Closer Filtering (PCF). To implement this, we have to use a shadow2D sampler instead of the usual sampler2D and also activate linear filtering and a comparison function on the depth attachment of the corresponding framebuffer.
This way, when sampling the texture the hardware actually looks at the four neighbouring pixels and return the average of the following function evaluated at each neighbour: 1 if in the shadow, 0 otherwise. Therefore the value computed for each fragment can take the values 0.0, 0.25, 0.5, 0.75 and 1.0.

While this reduce the sharpness of the aliasing effect, depth texels are still visible. To further improve the visual quality of our shadows we decided to use a technique called "Rotated poisson disk sampling". The idea behind this technique is to sample the depth texture multiple times in the fragment shader, each time introducing a random offset to the sampling position. The random offset is given by a kernel of numbers whose values are ditributed according to a Poissonian distribution. Because random number generation does not exist in glsl, we use a pseudo random number generator with the fragment position as a seed.

This produces pretty good results, as it can be seen on the folowing screenshot.

<img src="http://lkieliger.ch/docs/pictures/cgx/smoothTerrainShading.jpg"/>

## 5. Geometry shaders
From the calculations involved with computing the waves normals as well as the viewing and light directions emerged a need for visualizing vectors in the scene. Luckily, this is one of the uses cases for the geometry shaders.

We added to the water and terrain classes a new shader program which can be toggled on and off using the 'N' key. The new program compiles a geometry shader which for each vertex outputs a second vertex, displaced in the direction given by an arbitrary vector passed to it.

Below are illustrated use cases for such a shader. In the first screenshot you can see in green some of the surface normals as they were computed analytically in the TesselationEvaluation shader.

In the second screenshot, red segments illustrate the direction towards the light source and in yellow the direction of the surface normal at the given vertex, as computed by sampling the normal map texture.

<img src="http://lkieliger.ch/docs/pictures/cgx/waterNormals.jpg"/>
<img src="http://lkieliger.ch/docs/pictures/cgx/terrainNormals.jpg"/>

## 6 Infinite terrain generation
We now generate the terrain on the fly according to key presses on the arrows. To this end, the perlin noise is generated only where it needs to be, that is, at the edges of the noise texture. This avoid recomputation of already computed height points as well as the need for multiple terrain grids since the generation on a small area is fast enough to be imperceptible to the user.

The objective will be for the next milestone to couple the terrain generation with the camera location so that the user can fly-through the landscape while the terrain gets generated around him.

## 7 Further improvements
* Expand the terrain so that it looks bigger while conserving the current features.
* Implement a procedural Skybox.
* Add fog to hide terrain end.

#### 7.1 Possible optimizations
* Compute the shadow maps using multiple textures for having high shadow resolution at close range and smaller resolutions for shadow at higher ranges
* Implement the terrain quadtree as explained in the report for milestone 1.

## Work distribution

**Isaac Leimgruber**
- 10% implemented section `1. Skybox`

**Julien Harbulot**
- 30% implemented section `5. Infinite terrain generation`

**Leandro Kieliger**
- 60% implemented sections `1. Texturing`, `3. Enhanced water behaviour`, `4 Shadowmaps`, `6. Geometry shaders`
