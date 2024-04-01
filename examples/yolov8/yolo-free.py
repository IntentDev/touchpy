from ultralytics import YOLO

model = YOLO("models/yolov8x-pose.pt")

# results = model(source=0, show=True, conf=0.3, save=True)
# result = next(results)

# keypoints = result.keypoints.cpu().numpy()
# # print(keypoints["data"][0])

model.export(format='engine', verbose=True)