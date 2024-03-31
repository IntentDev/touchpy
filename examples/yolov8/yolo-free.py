from ultralytics import YOLO

model = YOLO("models/yolov8s-pose.pt",verbose=False)

results = model(source=0, show=True, conf=0.3, save=True)
result = next(results)

keypoints = result.keypoints.cpu().numpy()
# print(keypoints["data"][0])