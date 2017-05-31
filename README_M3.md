# Milestone 3 - Environment elements

## Contents:

1. Project basis: Bezier Curve (with constant speed along curve)
2. Grass map for variations at grass level
3. Clouds
4. Sky variations
5. Instancing Grass
6. Yacht 3D model
7. Fog
8. Work distribution

## 1. Project basis: Bezier Curve

A recursive implementation of the Bezier Curve following the deCasteljau algorithm. The Curve is then sampled, 
the distance between each sample evaluated to allocate a proportional time for each path between two successive samples to allow
a constant speed over the curve.

## 2. Grass map

To add variations at the grass level, we used another perlin noise. If the noise at some coordinates (where there would be grass)
has a higher value than some threshold, then we draw mud instead of grass. 

## 3. Clouds


## 4. Sky variations


## 5. Instancing grass
To add realism to our terrain, we felt adding grass was a good idea. Using instancing seemed the good way.

### 5.1 Drawing one bush
Following the tutorial on GPU gems, we learnt a good way to simulate a 3D grass bush was to alpha map a texture on a quad
and combine it with 3 similar quads rotated in order to have visibility from any direction. 
Our seed for instancing is formed of 18 vertices, 6 for each quad. Constructing the bushes this way, instead of instancing 3 times
the quad and rotate it, allowed us to save a lot of precious computations, since the rotations were already define in the 
vertex coordinates.
 
### 5.2 Drawing 1000 bushes
The main point of using instancing was to give each instance an attribute to distinguish itself from other.
We first tried to give a model matrix in order to be able to rotateand scale bushes whenever, but it was too costly.
Instead, we gave the positions of the bushes in the form of a translation from the seed. In the vertex shader, 
we computed the model matrix using the translations to position the vertices. To save more computations, and since there was 
no good reason for the bushes to have different sizes, we directly scaled the vertex coordinates.

### 5.3 Animating grass
The animation of the bushes was done in a straightforward way: we select the upper vertices of the bushes using the vertexID and move them using a sine function.

## 6. Yacht 3D model

## 7. Fog

## Work distribution

**Isaac Leimgruber**
- 
**Julien Harbulot**
- 
**Leandro Kieliger**
- 
