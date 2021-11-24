import cv2

cap = cv2.VideoCapture(0)
cap.set(cv2.CAP_PROP_BUFFERSIZE, 1)
cap.set(cv2.CAP_PROP_FOURCC, cv2.VideoWriter_fourcc('M', 'J', 'P', 'G'))

while True:
    _, frame = cap.read()
    blurred = cv2.GaussianBlur(frame, (7, 7), 0)
    gray = cv2.cvtColor(blurred, cv2.COLOR_BGR2GRAY)
    _, thres = cv2.threshold(gray, 150, 255, cv2.THRESH_BINARY|cv2.THRESH_OTSU)
    contours, hierarchy = cv2.findContours(thres, cv2.RETR_CCOMP, cv2.CHAIN_APPROX_SIMPLE)
    hierarchy = hierarchy[0]
    for i,cnt in enumerate(contours):
        if hierarchy[i][2] < 0 and hierarchy[i][3] < 0:
            cv2.drawContours(frame, [cnt], 0, (0, 255, 0), 3)
    cv2.imshow("Camera Frame", frame)
    cv2.imshow("Debug", thres)
    if cv2.waitKey(1) > 0: break

cap.release()
cv2.destroyAllWindows()