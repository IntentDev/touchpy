import touchpy as tp
import time

import torch
import modules.utils as utils
from modules.image_filter import ImageFilter

# ── Configuration ──────────────────────────────────────────────
TD_FPS = 60
HOST_FPS = 5

REALTIME = False  # (every frame / offline)
#   Steps through every 1/TD_FPS second sequentially.
#   Host loop runs at HOST_FPS, but each iteration advances TD
#   by exactly one frame (1/TD_FPS). No frames are skipped.
#   At HOST_FPS=5 and TD_FPS=60 it takes 12 seconds of wall time
#   to render 1 second of TD time.
#
# REALTIME = True
#   Uses wall-clock elapsed time. Host loop runs at HOST_FPS.
#   TD skips ahead to match real elapsed time, so ~12 TD frames
#   are skipped per host iteration (60/5). TD time tracks wall time.
# ───────────────────────────────────────────────────────────────

class MyComp(tp.Comp):
    def __init__(self):
        super().__init__(
            flags=tp.CompFlags.EXTERNAL_TIME | tp.CompFlags.CUDA_STREAM_DEFAULT,
            fps=TD_FPS
        )
        self.layout_ready = False
        self.set_on_layout_change_callback(self.on_layout_change)

        self.device = torch.device('cuda:0')
        self.frame_count = 0

        self.imag_filter = ImageFilter(
            in_channels=3,
            out_channels=3,
            kernel_size=12,
            stride=2,
            groups=3
        ).to(self.device)

        self.alpha_tensor = torch.ones(1, 1080, 1920, device=self.device, dtype=torch.float16)

    def on_layout_change(self):
        print('layout changed:')
        print('out tops:\n', *[f"\t{name}\n" for name in self.out_tops.names])
        print('in tops:\n', *[f"\t{name}\n" for name in self.in_tops.names])

        if 'Monitortop' in self.par.names:
            self.par['Monitortop'].val = 'topIn1'

        self.out_tops[0].set_cuda_flags(tp.CudaFlags.HWC)
        self.out_tops[1].set_cuda_flags(tp.CudaFlags.RGB | tp.CudaFlags.CHW)
        self.out_tops[2].set_cuda_flags(tp.CudaFlags.RGB)

        self.layout_ready = True


comp = MyComp()
comp.load('TopChopDatIO.tox', fps=TD_FPS)
comp.start()

while not comp.layout_ready:
    if comp.frame_did_finish():
        pass
    time.sleep(0.001)

mode_label = "REALTIME" if REALTIME else "EVERY FRAME"
print(f"\nRunning EXTERNAL_TIME | TD @ {TD_FPS} fps | host @ {HOST_FPS} fps | mode: {mode_label}")
print(f"Press 'q' to quit\n")

frame_step = 1.0 / TD_FPS
host_period = 1.0 / HOST_FPS
current_time = 0.0
start_wall = time.perf_counter()

comp.start_next_frame(current_time)

running = True
while running:
    loop_start = time.perf_counter()

    if utils.check_key('q'):
        # while not comp.frame_did_finish():
        #     time.sleep(0.001)
        break

    if comp.frame_did_finish():
        t = comp.time()
        elapsed = time.perf_counter() - start_wall
        print(f"frame {comp.frame_count:4d} | td time: {t.seconds:8.3f}s | wall: {elapsed:8.3f}s")

        out_top1_tensor = comp.out_tops[0].as_tensor()
        out_top2_tensor = comp.out_tops[1].as_tensor()
        out_top3_tensor = comp.out_tops[2].as_tensor()

        if REALTIME:
            current_time = time.perf_counter() - start_wall
        else:
            current_time += frame_step

        comp.start_next_frame(current_time)

        modified_tensor = out_top1_tensor.float()
        modified_tensor[:, :, :-1] *= .5
        modified_tensor[:, :, :-1] += 100
        modified_tensor = torch.clamp(modified_tensor, 0, 255)
        modified_tensor = modified_tensor.to(torch.uint8)
        comp.in_tops[0].from_tensor(modified_tensor)

        filtered_tensor = comp.imag_filter(out_top2_tensor.unsqueeze(0)).squeeze(0)
        comp.in_tops[1].from_tensor(filtered_tensor)

        rgba_tensor = torch.cat((out_top3_tensor, comp.alpha_tensor), dim=0)
        comp.in_tops[2].from_tensor(rgba_tensor)

        comp.frame_count += 1

    # throttle host loop to HOST_FPS
    loop_elapsed = time.perf_counter() - loop_start
    sleep_time = host_period - loop_elapsed
    if sleep_time > 0:
        time.sleep(sleep_time)


comp.clear_on_layout_change_callback()
comp.stop()
comp.unload()
del out_top1_tensor, out_top2_tensor, out_top3_tensor
del modified_tensor, filtered_tensor, rgba_tensor
del comp
import gc; gc.collect()  # noqa: E702
