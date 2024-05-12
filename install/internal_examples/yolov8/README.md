# TouchPy example: YOLOv8

YOLOv8 can perform object detection, tracking, instance segmentation, image classification and pose estimation tasks.
This examples uses a live video output from TouchDesigner. 

## Requirements:

- numpy
- ultralytics
- torch
- cuda 11.8

If you already have created the `touchpy` conda environment (recommended - see README in repo root), the only thing missing is for this example is `ultralytics`, here's how to add it to your `touchpy` environment:

```
conda activate touchpy
mamba install ultralytics
```


## How to run:

1. Start `toe/yolo.toe`


2. activate touchpy environment and run the example
```
conda activate touchpy
python yolo.py
```
