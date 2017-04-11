# Milestone 1 - Procedural terrain generation

#Contents:

1. Perlin noise generation and variations
2. Normal map from noise texture
3. Height and angle based colouring of terrain
4. First person view camera
5. Tesselation shader
6. Work distribution

##Notes
For your convenience, please note the effect of the following inputs:

- Keys W,A,S,D: Respectively move the camera forward, backward, left and right
- Shift and spacebar: Respectively move the camera downward and upward.
- Keys G/H: Decrease or increase the zoom used to sample the heightmap to generate terrain elevation
- Key F: Activate/deactivate wireframe rendering mode.

## 1. Perlin noise generation and variations
## 2. Normal map from noise texture
One of the main disadvantage with flat shading is that it requires to compute the normal for each fragment.
Such computations can be very inefficient because when multiple fragments share the same primitive, they will
have the same normal and we will compute multiple times the same value.

Therefore, we choose to compute once and for all the terrain normals and store them in a texture that will
then be sampled by the fragment shader.

We built a new class which takes as input a Perlin noise texture and output the corresponding normal map, 
stored in a texture.

The additional advantage that this technique offers is that when used with the tesselation shader, the shading 
can still leverage the normal map to simulate a high level of detail, even if the underlying mesh is at low
resolution.

## 3. Height and angle based colouring of terrain
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

**Dont forget that you can toggle the wireframe rendering mode by using the F key !**

### 5.1 Tesselation Control Shader
The control shader receives patches of 4 vertices and specifies the outer and inner levels of tesselation.

The inner tesselation levels are simply the average of the corresponding outer levels. That is, the horizontal 
inner tesselation level is the average of the outer levels on the left and right edges and the vertical inner
tesselation level is the average of the outer levels on the top and bottom edges.

The outer tesselation levels are computed based on the distance between the vertices of the edge and the camera.
We take the mean camera distance of each pair of vertices per quad edge and use it to interpolate the 
appropriate tesselation level.
To this end we specified a maximum/minimum tesselation level and a maximum/minimum distance at which the tesselation
becomes active.

Finally, we cull the patches that are outside the camera view frustrum to save computation time.
 
### 5.2 Tesselation Evaluation Shader
The evaluation shader receives all generated primitives. It first compute the primitive attributes such as
the 3D position or 2D texture coordinates by interpolating the values of the control points (the vertices)
forming the patch based on the gl_TessCoord value, which indicates the position of the generated primitive
inside the patch.

We then sample the heightmap to define the height of each vertex and the normal map to get the normal at the
vertex position.

Finally we perform model, view and projection transforms on each vertex and apply some diffuse lighting on it.

## Work distribution
- Isaac Leimgruber:
- Julien Harbulot: 
- Leandro Kieliger: Tesselation shader, normal map computation.