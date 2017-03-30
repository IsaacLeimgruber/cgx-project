# Readme for second homework of Computer Graphics class

## Perspective projection
Creating a perspective projection matrix is almost straightforward once one has been given the appropriate formula. The only extra thinking required here came from the fact that we did not immediately know the top, bottom, left and right value of the near plane. This was easily solved with a bit of trigonometry, and from the following equation: aspect = (right-left)/(top-bottom) = right / top.

We derived:

top = tan(radians(fovy/2.0) * near
bottom = -top
right = aspect * top
left = -right

## Trackball
Projecting the point onto the trackball / hypersheet and deriving the appropriate rotation matrix was quite straightforward as the formulas were explicitly given on Khronos' wiki page and everything was already in place to keep track of the last point preceding the mouse click.

Then, we had to accumulate rotations performed by the user. Again, everything was already in place to save the previous rotation matrix upon user click. To accumulate the new rotation on top of the old ones, we simply left-multiply the old trackball matrix by the output of the drag function.

Finally, we decided to implement the zoom feature in the same fashion as the rotation. We added a new matrix in the main.cpp file which saves the current view translation when the user presses the right mouse button down. When the button is released, the new translation is accumulated on top of the previous ones by left-multiplying just like we did for the rotations.

## Triangle grid
We specified the grid as 100x100 vertices. Since we wanted the grid to be 2 units wide and to hold 100 points (and thus 99 spaces), we separated each vertex from its neighbors by a distance of 2.0 / (dim_grid - 1.0) starting from the bottom-left one. Then, for the indices values we indicated the vertices of every pair of triangles within each subsquare of the grid in the following order:

- bottom left / top right / top left
- bottom left / bottom right / top right

We let (i,j) be the coordinate of the bottom left corner. The numbering goes from bottom left to top right, that is, (i+1, j+1) is the next top right vertex. Because each line consists of 100 vertices and the indices vector is unidimensional, we can access the element at position (i, j) by writing indices[j*100 + i].

This remark lets us specify vertex indices for each triangle trough a nested for-loop. Note finally that we stop the loop one unit before the end since we are always creating triangles with coordinates one unit bigger than i and j.

## Water simulation
To generate a wave with the triangle grid, we need to modify the height of each vertex based on time and position. A sin wave can has the following parameters:

- A, the amplitude
- f, the frequency
- phi, the phase

By selecting the position as the main parameter we can give an initial, static, wave shape. By altering the phase as a function of time, we can tweak the speed at which the wave 'travels'. Moreover, if we multply the position by the frequency, we can tweak the wavelength.

Therefore, our model has the following aspect: height = A * sin( 2pi * f * pos  + phi * time).

By using uv.x + uv.y as the pos parameter, the wave will propagate along the diagonal of the grid and since we want roughly a bit more than 3 crests to be visible at any given time along the diagonal of the square we can choose something like 3/2 as a frequency.

## Water simulation bonus

To obtain a water-like surface we inspired ourselves from the idea proposed at the following address: http://http.developer.nvidia.com/GPUGems/gpugems_ch01.html

We created four sinusoïdal waves:

- A base wave of large amplitude with a slow moving front
- A wave of medium amplitude and faster moving front, slightly rotated with respect to the base wave to create some ripple effects, like if the wind was blowing a bit sideways on the base wave.
- A small wave to amplify the ripple effect, at the same angle as the medium wave
- A miniature fast-moving wave to capture the instability of water waves, at an angle which is between the base and medium wave.

We sharpened the smaller waves by elevating them by a given power.

We finally added more or less of each sine wave as a function of time, to vary the pattern created by the waves.