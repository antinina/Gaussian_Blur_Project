import cv2
import pickle
import os

width, height = 90, 40

cwd = os.getcwd()
filename_park_pos = 'CarParkPos2'
pathname_park_pos = os.path.join(cwd, filename_park_pos)

try:
    with open(pathname_park_pos, 'rb') as f:
        posList = pickle.load(f)
except:
    posList = []


def mouseClick(events, x, y, flags, params):
    if events == cv2.EVENT_LBUTTONDOWN:
        posList.append((x, y))
    if events == cv2.EVENT_RBUTTONDOWN:
        for i, pos in enumerate(posList):
            x1, y1 = pos
            if x1 < x < x1 + width and y1 < y < y1 + height:
                posList.pop(i)

    with open(pathname_park_pos, 'wb') as f:
        pickle.dump(posList, f)


while True:

    image_path = os.path.abspath(os.path.join(cwd, '../../data/carPark.png'))

    img = cv2.imread(image_path)
    
    for pos in posList:
        cv2.rectangle(img, pos, (pos[0] + width, pos[1] + height), (255, 0, 255), 2)

    cv2.imshow("Image", img)
    cv2.setMouseCallback("Image", mouseClick)
    cv2.waitKey(1)
