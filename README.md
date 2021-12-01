# Marker
Implement and play with marker detection.

Most knowledge is from book "Augmented Reality: Principles and Practice".

------

### Marker Detection

The marker I'm using is simply like this:  

<img src="marker.png" width="200" alt="marker">

1. Convert image to gray scale on GPU compute shader  
   Dot product of image color with `(0.299, 0.587, 0.114)`  
   Optionally, I also have applied a simple blur to make image stable  

2. Convert to binary image by thresholding on GPU compute shader  
   Threshold value from mipmap top level (automatically averaged), or manually configure  

3. Trace closed contour (only one) on CPU  
   I'm using Theo Pavlidis' Algorithm  
   (OpenCV uses a different algorithm which is not suitable for my case)  
   Thresholds are applied to filter out small and non-rectangular contours  

4. Fit quadrilateral to the closed contour and get 4 corners  
   Following the algorithm mentioned in Chapter 4  
   (OpenCV uses Ramer–Douglas–Peucker algorithm, which is redundant for my case)

### Pose Estimation

With marker corners, we can estimate the pose from homography.  

1. Prepare pairs of vector `p` (corner pos in screen space) and `q` (corner pos in object space)  
   `q` is by default:
   ```
   (0,1,0)  (1,1,0)
   (0,0,0)  (1,0,0)
   ```
   with z set to zero, and that is why we can use homography  

2. Prepare 3x3 matrix `H` where:
   ```
   cross(p, Hq) = 0
   ```

3. Rewrite the equation to:
   ```
   A h = 0
   ```
   where `A` is 2N*9 matrix and `h` is vector of size 9 from `H`  
   N is 4 in our case  

4. Run SVD on the equation:
   ```
   A = U D V^T
   ```
   Get vector `h` (last column of `V`) and reassemble `H`  

5.  