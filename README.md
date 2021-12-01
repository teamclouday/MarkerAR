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
   Thresholds are applied to filter out small and non-rectangular contours  

4. Fit quadrilateral to the closed contour and get 4 corners  
   Following the algorithm mentioned in Chapter 4  

### Pose Estimation

With marker corners, we can estimate the pose from homography.  
