# Marker
Implement and play with marker detection.

Knowledge is from book "Augmented Reality: Principles and Practice", and online resources (commented in code).

------

### PC Version

Spiderman! 😨😁😝

<img src="Images/MarkerPC2.gif" width="600" alt="markerPC2">

------

### Marker Detection

The marker I'm using is simply like this:  

<img src="Images/marker.png" width="200" alt="marker">

1. Convert image to gray scale on GPU compute shader  
   Dot product of image color with `(0.299, 0.587, 0.114)`  
   Optionally, I also have applied a simple blur to make image stable  

2. Convert to binary image by thresholding on GPU compute shader  
   Threshold value from mipmap top level (automatically averaged), or manually configure  

3. Trace closed contour (only one) on CPU  
   I'm using Theo Pavlidis' Algorithm  
   Thresholds are applied to filter out small and non-rectangular contours  

4. Fit quadrilateral to the closed contour and get 4 corners  
   Following the algorithm mentioned in Chapter 4  
   Similar to OpenCV's Ramer–Douglas–Peucker algorithm but simpler  

5. Determine orientation by sampling near the corners

### Pose Estimation

Starting from here, you will need to calibrate your webcam and get intrinsic matrix and distortion coefficients. I use OpenCV to do this step since it is convenient. Details can be found in `OpenCV/camera.py`.

Let's say your have the 3x3 camera matrix `K`, 4 detected marker corners on camera image are `p`.  
Corners in virtual world space are `q` (usually something like `[1,1,0,1]`).  
What we are looking for is a 3x4 transformation matrix `M` (with rotation `R` and translation `t`) such that:
```
p = K M q
M = [R t]
  = [r1 r2 r3 t]
```
The fact that all corners are on a plane allows to eliminate Z axis and use homography estimation:
```
p = H q'
```
For example, if `q` is `[qx,qy,0,1]`, then `q'` can be `[qx,qy,1]`.  
Since `p'` and `Hq'` are in same direction, cross product is zero:  
```
p' x (H q') = 0
```
Expand the left side and we get the following:
```
[
   q1.x, q1.y, 1.0f, 0.0f, 0.0f, 0.0f, -p1.x*q1.x, -p1.x*q1.y, -p1.x,
   0.0f, 0.0f, 0.0f, q1.x, q1.y, 1.0f, -p1.y*q1.x, -p1.y*q1.y, -p1.y,
   .....
   q4.x, q4.y, 1.0f, 0.0f, 0.0f, 0.0f, -p4.x*q4.x, -p4.x*q4.y, -p4.x,
   0.0f, 0.0f, 0.0f, q4.x, q4.y, 1.0f, -p4.y*q4.x, -p4.y*q4.y, -p4.y,
] * 
[
   h1,h2,h3,h4,h5,h6,h7,h8,h9
] = 0

H = 
[
   h1 h2 h3
   h4 h5 h6
   h7 h8 h9
]
```
The matrix `A` on the left has size 8x9, `h` is a vector of size 9.  
Run SVD on `A` and get `U D V^T` and `h` is the last column in matrix `V`.  
Reconstruct 3x3 matrix `H`, and we need to extract `R` and `t` from it:
```
HK = K^-1 H
n = ( norm(HK.col0) + norm(HK.col1) ) / 2
t = H.col2 / n
r1 = normalize( HK.col0 )
r2 = normalize( HK.col1 )
r3 = normalize( r1 x r2 )
R = [r1 r2 r3]
M = [R t]
```
Note that there are many ways to do it. Here I illustrate a simple one I found online.  
Besides, you may also need to undistort `p` first given camera distortion coefficients (Details can be found in OpenCV source code of function `undisortPoints`).  

Finally, for any new point `b` in 3d world space, its mapping on screen will be computed from `K M b`.

Further details of my implementation can be found in `PC/src/markerpose.cpp`.

### Pose Refinement

However, with only homography estimation, the result may have large pixel error, since SVD is just an approximation of the solution of the equation above.  

Suggested by the book, I implemented Levenberg-Marquardt's algorithm to refine the matrix `M`.  

Let's say the objective is to minimize:
```
sum ( p - K M q )^2
```
This is equivalent to:
```
sum ( kp - M q )^2
kp = K^-1 p
```
where `M` is the variable to update.  
Suppose an update of `d` on `M` moves closer to the local minimum:
```
sum ( kp - M q - J d )^2
```
where `J` is the Jacobian matrix of function `M q` with respect to `M`.  
Take derivative of it and set to zero, we get the following linear equation:
```
( J^T J ) d = J^T ( kp - M q )
```
Levenberg's version:
```
( J^T J + lambda diag( J^T J ) ) d = J^T ( kp - M q )
```
where `lambda` is a damping factor adjusted at each iteration.  
Finally, iteratively solve for `d` and update `M` until max iterations or minimum error.

Further details can be found in `PC/src/markerLM.cpp`.

### Some Debugging

The above does not work perfectly in my implementation.  
When Y-axis (in OpenGL coordinate) value increases, the projected points tend to move to lower left (45 degrees). And this is independent of the camera rotation.  
After a long time of debugging, I can't figure out why, but a quick fix is the following:
```glsl
vec3 screenPos = poseM * pos;
screenPos.xy -= 0.5 * screenPos.z;
```
After projecting a point, shift its XY values by half of its Z value, and now it looks perfect!  
This bug should be caused by the transformation matrix `M`, which then traces back to homography approximation and decomposition. A lot more to learn to understand why.