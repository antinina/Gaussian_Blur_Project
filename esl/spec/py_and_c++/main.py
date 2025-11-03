import ctypes
import cv2
import pickle
import cvzone
import numpy as np
import os
#from numpy.ctypeslib import ndpointer

os.environ["XDG_SESSION_TYPE"] = "xcb"

cwd = os.getcwd()

filename_cpp_lib = os.path.join(cwd, 'cpplibrary.so')

cpplibrary = ctypes.CDLL(filename_cpp_lib)


cpplibrary.func.argtypes = [np.ctypeslib.ndpointer(dtype = np.uint8, flags="C_CONTIGUOUS")]
cpplibrary.func.restype = ctypes.POINTER(ctypes.c_int)        



filename_video_feed = os.path.abspath(os.path.join(cwd, '../../data/carPark.mp4'))
# Video feed
video_path = os.path.abspath(filename_video_feed)
cap = cv2.VideoCapture(video_path)

filename_park_pos =  os.path.abspath(os.path.join(cwd, '../python/CarParkPos1'))
with open(filename_park_pos, 'rb') as f:
    posList = pickle.load(f)

width, height = 90, 40


n = 0
while(n<500):  #while True: 

  if not cap.isOpened():
    print("Error: Unable to open video source.")

  if cap.get(cv2.CAP_PROP_POS_FRAMES) == cap.get(cv2.CAP_PROP_FRAME_COUNT):
      cap.set(cv2.CAP_PROP_POS_FRAMES, 0)
   
  success, img = cap.read()
  
  imgGray = cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)

 
  if not success:
      print("Error reading frame from video.")
      break



  resultpy = cpplibrary.func(imgGray)


  spaceCounter = 0
  i = 0
  for pos in posList:
    x, y = pos

    if resultpy[i] == 1:    
      spaceCounter = spaceCounter + 1 
      color = (0, 255, 0)
      thickness = 2
    else:
      color = (0, 0, 255)
      thickness = 2


    cv2.rectangle(img, pos, (pos[0] + width, pos[1] + height), color, thickness)
    i=i+1

  cvzone.putTextRect(img, f'Free: {spaceCounter}/{len(posList)}', (100, 50), scale=3,
                    thickness=5, offset=20, colorR=(0,200,0))

  cv2.imshow("Image", img)
  n+=1
  cv2.waitKey(1)


