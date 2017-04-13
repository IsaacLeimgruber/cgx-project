# Milestone 1 - Procedural terrain generation

## Contents:

1. Project basis: Perlin noise, heightmap and coloring
2. Perlin noise generation and variations
3. Normal map from noise texture
4. First person view camera
5. Tesselation shader
6. Work distribution

## 1. Project basis: Perlin noise, heightmap and coloring

Initial Perlin noise implementation following GPUGems.


Perlin noise was first implemented using the permutation table to generate pseudo-random gradients. Following the algorithm, we obtained our perlin noise. Our function has the signature perlin_nosie(float x, float y). Multiplying the x and y parameters, initially given   as uv.x and uv.y allows us to modify the frequency.

Initial OctavePerlin implementation cumulates several octaves of Perlin noise.

The implementation of the Brownian motion was mainly generating and stacking multiple perlin noises at different frequencies.

### 1.2. Height and angle based colouring of terrain

Using the normal vector allows us to calculate the slope by scalar product with the vertical vector (0, 1, 0). Now we have an alpha angle with range [-1, 1]. We can choose an arbitrary threshold. Knowing that angle 0 means our normal vector is horizontal, we have highest slope. Instead of coloring only with some grey when the slope is higher than the threshold, we found more realistic to mix the rock color with the color corresponding to the given height. The mix ratio being a function of the slope allowed us to define a smooth transition between, say, grass and rock, up to some other threshold where we directly color with rock (this seemed to imitate a natural coloring pretty well. One can think of a cliff this way: When the slope is low there is grass (less as the slope increase), but at some height, there's no grass anymore.

<img src="https://lkieliger.ch/docs/pictures/cgx/cliffMix.png">

## 2. Perlin noise improvements and variations
Since the perlin noise is at the heart of our procedural project, we took some time experiment with and better understand its complicated behaviors.

Moving on from the initial Perlin noise implementation, we tweaked the permutation array generation to make it faster (with pre-caching) and random (using C++ 11's random). From there, we implemented several fBm:

- Ridged noise: a ridged noise and fBm
- Multifractal: this is Kenton Musgrave's mutifractal algorithm
- pow fBm: this is an octave based fBm were the power of the Perlin noise is taken

We then proceeded to implement an utility that would display the different noise side by side so as to be able to compare them.
On the screenshots below: 

- OctavePerlin is on the top left corner,
- Pow-fBm is on the bottom left corner, 
- Ridged-noise is on the top right corner,
- and Multifractal (fed with an initial ridged-noise) is on the bottom right corner.

<img src="https://lkieliger.ch/docs/pictures/cgx/terrainCompil1.jpg" alt="Perlin noise variations" />

<img src="https://lkieliger.ch/docs/pictures/cgx/terrainCompil2.jpg"  alt="Perlin noise variations" />

Our work on the noise generation is not finished, yet. We are still looking for an implementation or combination of noise that will provide the most realistic results. Our plans for improvement are the following:

- Further optimize fBm generation's speed by extracting the Perlin noise in a different texture.
- Find the most appropriate random distribution for the permutation array generation.
- Settle for a noise implementation. Our favorite at the moment is a mix of Perlin noise and ridged-noise in a multifractal like fBm. Due to technical problems (Julien's computer died...) we couldn't implement it in the project, yet.
- Implement erosion.

## 3. Normal map from noise texture
One of the main disadvantages with flat shading is that it requires to compute the triangle normal for each fragment.
Such computations can be very inefficient because when multiple fragments share the same primitive, they will
have the same normal and we will compute multiple times the same value.

Therefore, we choose to compute once and for all the terrain normals and store them in a texture that will
then be sampled by the fragment shader.

We built a new class which takes as input a Perlin noise texture and output the corresponding normal map, 
stored in a texture.

The additional advantage that this technique offers is that when used with the tesselation shader, the shading 
can still leverage the normal map to simulate a high level of detail, even if the underlying mesh is at low
resolution.

<img src="https://lkieliger.ch/docs/pictures/cgx/normalMapCompil.png">

## 4. First person view camera
We followed the tutorial on learnopengl.com to implement the first person view camera. At the end of the 
tutorial the author of the website offers a class which encapsulates the behavior of the camera. As we
understood the effect of the code, we took the class from learnopengl.com and slightly adapted it for our
needs. Therefore, most of the code in this class is *NOT* our work.

## 5. Tesselation shader
The helper class was modified in order to be abe to compile two new types of shaders: the Tesselation 
Control Shader and the Tesselation Evaluation Shader.

The grid class was modified to render GL_PATCHES instead of GL_ELEMENTS, the vertices are now indexed
in squares instead of triangles and in counter clockwise order. That is, bottom left is vertex 0, 
bottom right 1, top right 2, top left 3.

### 5.1 Tesselation Control Shader
The control shader receives patches of 4 vertices and specifies the outer and inner levels of tesselation.

The inner tesselation levels are simply the average of the corresponding outer levels. That is, the horizontal 
inner tesselation level is the average of the outer levels on the left and right edges and the vertical inner
tesselation level is the average of the outer levels on the top and bottom edges.

The outer tesselation levels are computed based on the mean distance between the pair of vertices forming the edge and the camera.
We use the distance to interpolate the appropriate tesselation level.
To this end, we specified a maximum/minimum tesselation level and a maximum/minimum distance at which the tesselation
becomes active.

Finally, we cull the patches that are outside the camera view frustrum to save computation time.
 
### 5.2 Tesselation Evaluation Shader
The evaluation shader receives all generated primitives. It first compute the primitive attributes such as
the 3D position or 2D texture coordinates by interpolating the values of the control points (the vertices)
forming the patch. The interpolation is done based on the gl_TessCoord value, which indicates the position of the generated primitive with respect to its patch.

We then sample the heightmap to define the height of each vertex and the normal map to get the normal at the
vertex position.

Finally we perform model, view and projection transforms on each vertex and apply some diffuse lighting on it.

You can see the results which show four different stages of tesselation based on vertex distance to the camera.

<img src="https://lkieliger.ch/docs/pictures/cgx/tessCompil.png">

### 5.3 Further improvements
At the moment, we use a fixed patch size for the tesselation. That is, the grid is always represented with a fixed number of vertices. While this allow for a simple implementation of the tesselation shader, when patches are really far away they may end up in the same fragment after rasterization. Therefore, we would like to lower the level of detail of those sections even more.

This can be done by represening the terrain with a quadtree, where each leaf of the tree defines a region of the grid. This way, we can recursively split the quadtree into 4 child nodes, which would then represent 4 sub-patches on the grid, based on the distance to the camera. The deeper the leaf in the quadtree, the more vertices are used in the corresponding patch in the terrain.

## Work distribution

**Isaac Leimgruber**
- 33% implemented section `1. Project basis: Perlin noise, fBm, heightmap and coloring`

**Julien Harbulot**
- 33% implemented section `2. Perlin noise generation and variations`
- researched the best way to make the terrain infinite, but couldn't implement a working version for this assignment due to technical problems (computer died).

**Leandro Kieliger**
- 33% implemented sections `3. Normal map from noise texture` and `5. Tesselation shader`.
