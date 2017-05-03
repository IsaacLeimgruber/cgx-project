# Report 2
## Mandatory Parts

### Texturing
First, we imported the textures in our terrain.h, and passed them to the fshader in a way similar to what we learnt in the labs.

Our last implementation used to color our terrain depending on its height by using vec3 constants such as GRASS_COLOR, ROCK_COLOR, etc.
We simply replaced the declaration of our colors by an assignement of texture.

vec3 GRASS_COLOR = vec3(r, g, b);
became
vec3 GRASS_COLOR = 255.0 * texture(grass_texture, uv).rgb;

Which displayed a nice grass texture where a green color used to be.
Doing the same for other colors, we obtained a full-textured terrain.

### Skybox
The loadCubemap method takes a vector of "string" as parameter which contains the filenames of the textures for the respective skybox faces. The method uses the strings to bind the textures to the cubemap object.

In the Init() method, we start by declaring the skybox vertices that correspond to the vertices constituing each face of the skybox. Since we have 6 faces, and that each face need 6 vertices (to define the two primitive triangles), we end up with 36 vertices.

At last, we display our skybox, making sure we set the skybox's view matrix to be the camera's view matrix in order to never naviguate outside of the skybox (the box follows the camera, which gives the impression that the sky environment is infinite).


### Water Reflexion
