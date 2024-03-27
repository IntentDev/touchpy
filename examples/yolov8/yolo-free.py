#from ultralytics import YOLO
import torch 
print(torch.__version__)
print(torch.cuda.is_available())
#model = YOLO("yolov8m-pose.pt",verbose=True)

#results = model(source=0, show=True, conf=0.3, save=True)