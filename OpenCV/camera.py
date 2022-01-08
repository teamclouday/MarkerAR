# this script helps collect the camera matrix
import os
import sys
import glob
import cv2
import pickle
import numpy as np

# chessboard: https://www.mrpt.org/downloads/camera-calibration-checker-board_9x7.pdf
# reference: https://docs.opencv.org/4.x/dc/dbb/tutorial_py_calibration.html

# termination criteria
criteria = (cv2.TERM_CRITERIA_EPS + cv2.TERM_CRITERIA_MAX_ITER, 30, 0.001)

# overwrite these values for your chess board
BOARD_ROWS = 9
BOARD_COLS = 7

# prepare object points, like (0,0,0), (1,0,0), (2,0,0) ....,(6,5,0)
objp = np.zeros((BOARD_ROWS*BOARD_COLS, 3), np.float32)
objp[:,:2] = np.mgrid[0:BOARD_ROWS, 0:BOARD_COLS].T.reshape(-1,2)

# Arrays to store object points and image points from all the images.
objpoints = [] # 3d point in real world space
imgpoints = [] # 2d points in image plane.
images = glob.glob(os.path.join("samples", "*.jpg"))
if len(images) <= 0:
    print("Now sample images found!")
    sys.exit(-1)

for fname in images:
    img = cv2.imread(fname)
    gray = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    # Find the chess board corners
    ret, corners = cv2.findChessboardCorners(gray, (BOARD_ROWS, BOARD_COLS), None)
    # If found, add object points, image points (after refining them)
    if ret == True:
        objpoints.append(objp)
        corners2 = cv2.cornerSubPix(gray, corners, (11,11), (-1,-1), criteria)
        imgpoints.append(corners)
        # Draw and display the corners
        cv2.drawChessboardCorners(img, (BOARD_ROWS, BOARD_COLS), corners2, ret)
        cv2.imshow('Sample Image', img)
        cv2.waitKey(0)

# Now calibrate camera
ret, mtx, dist, rvecs, tvecs = cv2.calibrateCamera(objpoints, imgpoints, gray.shape[::-1], None, None)
with open("camera.pkl", "wb") as outFile:
    pickle.dump([mtx, dist, rvecs, tvecs], outFile)
print("Camera calibrated! {}".format(ret))

# Estimate re-projection error
mean_error = 0
for i in range(len(objpoints)):
    imgpoints2, _ = cv2.projectPoints(objpoints[i], rvecs[i], tvecs[i], mtx, dist)
    error = cv2.norm(imgpoints[i], imgpoints2, cv2.NORM_L2) / len(imgpoints2)
    print("Image {} error: {}".format(images[i], error))
    mean_error += error
print( "Total re-projection error: {} (the lower the better)".format(mean_error/len(objpoints)))

cv2.destroyAllWindows()