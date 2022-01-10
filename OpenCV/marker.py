import os
import pickle
import cv2
import numpy as np

# reference: https://docs.opencv.org/4.x/d7/d53/tutorial_py_pose.html

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))

# load camera matrix
camera_matrix_loaded = False
if os.path.exists("camera.pkl"):
    with open("camera.pkl", "rb") as inFile:
        mtx, dist, _, _ = pickle.load(inFile)
    camera_matrix_loaded = True

# define drawing at corners
def drawAxis(img, corners, imgpts):
    corner = tuple(corners[0].ravel())
    cv2.line(img, corner, tuple(imgpts[0].ravel()), (255,0,0), 5)
    cv2.line(img, corner, tuple(imgpts[1].ravel()), (0,255,0), 5)
    cv2.line(img, corner, tuple(imgpts[2].ravel()), (0,0,255), 5)

def drawCube(img, corners, imgpts):
    imgpts = np.int32(imgpts).reshape(-1,2)
    # draw ground floor in green
    cv2.drawContours(img, [imgpts[:4]],-1,(0,255,0),-3)
    # draw pillars in blue color
    for i,j in zip(range(4),range(4,8)):
        cv2.line(img, tuple(imgpts[i]), tuple(imgpts[j]),(255),3)
    # draw top layer in red color
    cv2.drawContours(img, [imgpts[4:]],-1,(0,0,255),3)
    return img

objp = np.array([
    [0.0, 0.0, 0.0],
    [1.0, 0.0, 0.0],
    [1.0, 1.0, 0.0],
    [0.0, 1.0, 0.0]
])
axis = np.array([
    [0.5, 0.0, 0.0],
    [0.0, 0.5, 0.0],
    [0.0, 0.0, 0.5]
])
axisCube = np.array([
    [0,0,0], [0,0.5,0], [0.5,0.5,0], [0.5,0,0],
    [0,0,-0.5],[0,0.5,-0.5],[0.5,0.5,-0.5],[0.5,0,-0.5]
])

while True:
    _, frame = cap.read()
    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    blurred = cv2.GaussianBlur(gray, (5, 5), 0)
    thres = cv2.threshold(gray, 60, 255, cv2.THRESH_OTSU)[1]
    contours, _ = cv2.findContours(thres, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)
    for i,cnt in enumerate(contours):
        # detect my custom square contour
        approx = cv2.approxPolyDP(cnt, 0.01 * cv2.arcLength(cnt, True), True)
        if len(approx) == 4 and cv2.contourArea(cnt) > 600:
            corners = np.array(approx).reshape(4, 2)
            centroid = np.sum(corners, axis=0) / 4
            
            p1, p2, p3, p4  = 0, 1, 2, 3

            p1center = np.round((centroid + corners[p1, :]) * 0.5).astype(np.int32)
            p2center = np.round((centroid + corners[p2, :]) * 0.5).astype(np.int32)
            p3center = np.round((centroid + corners[p3, :]) * 0.5).astype(np.int32)
            p4center = np.round((centroid + corners[p4, :]) * 0.5).astype(np.int32)

            if thres[p2center[1], p2center[0]] < 255:
                p1, p2, p3, p4 = p2, p3, p4, p1
            elif thres[p3center[1], p3center[0]] < 255:
                p1, p2, p3, p4 = p3, p4, p1, p2
            elif thres[p4center[1], p4center[0]] < 255:
                p1, p2, p3, p4 = p4, p1, p2, p3

            cv2.circle(frame, tuple(corners[p1, :]), 5, (255, 0, 0), -1)
            cv2.circle(frame, tuple(corners[p2, :]), 5, (0, 255, 0), -1)
            cv2.circle(frame, tuple(corners[p3, :]), 5, (0, 0, 255), -1)
            cv2.circle(frame, tuple(corners[p4, :]), 5, (255, 0, 255), -1)
            cv2.circle(frame, tuple(centroid.astype(np.int32)), 5, (0, 255, 255), -1)

            corners = corners[[p4, p3, p2, p1]].astype(np.float32)

            # Find the rotation and translation vectors.
            _, rvecs, tvecs = cv2.solvePnP(objp, corners, mtx, dist, flags=cv2.SOLVEPNP_ITERATIVE)
            # undistorted = cv2.undistortPoints(corners, mtx, dist)
            # fx = mtx[0, 0]
            # fy = mtx[1, 1]
            # cx = mtx[0, 2]
            # cy = mtx[1, 2]
            # for i in range(4):
            #     undistorted[i, 0, 0] = undistorted[i, 0, 0] * fx + cx
            #     undistorted[i, 0, 1] = undistorted[i, 0, 1] * fy + cy
            # print("="*20)
            # print(corners)
            # print("-"*20)
            # print(undistorted)

            # project 3D points to image plane
            # imgpts, jac = cv2.projectPoints(axis, rvecs, tvecs, mtx, dist)
            # drawAxis(frame, corners, np.round(imgpts).astype(np.int32))

            imgpts, jac = cv2.projectPoints(axisCube, rvecs, tvecs, mtx, dist)
            drawCube(frame, corners, np.round(imgpts).astype(np.int32))

            # break because only one expected
            break
    cv2.imshow("Debug", thres)
    cv2.imshow("Camera Frame", frame)
    if cv2.waitKey(1) > 0: break

cap.release()
cv2.destroyAllWindows()