from ultralytics import YOLO

model = YOLO("yolov8s-pose.pt",verbose=True)

results = model(source=0, show=True, conf=0.3, save=True)