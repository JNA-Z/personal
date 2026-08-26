from maix import camera, display, image, nn, app, time, uart, pinmap, err, http

MODEL_PATH = "/root/models/car_ballV2_yolo11n_416.mud"

UART_DEV = "/dev/ttyS0"
UART_BAUD = 115200
CONF_TH = 0.1
IOU_TH = 0.45
VISUAL_EVERY = 1
DEBUG_EVERY = 30
DETECT_EVERY_N = 1
CROP_H_RATIO = 1.0
DETECT_H_RATIO = 0.18
DETECT_Y_OFFSET = -12  # negative = up, positive = down (pixels)

# ---- 微分预测补偿 (削弱高速运动下的视觉滞后) ----
KD = 0.04                # 微分系数 
VEL_EMA_ALPHA = 0.5      # 速度 EMA 平滑系数 (0-1, 越大越灵敏)

err.check_raise(pinmap.set_pin_function("A16", "UART0_TX"), "set A16 UART0_TX failed")
err.check_raise(pinmap.set_pin_function("A17", "UART0_RX"), "set A17 UART0_RX failed")
serial = uart.UART(UART_DEV, UART_BAUD)

detector = nn.YOLO11(model=MODEL_PATH, dual_buff=True)

AI_W = detector.input_width()
AI_H = detector.input_height()

CAM_W = 640
CAM_H = 360
scale_x = CAM_W / AI_W
scale_y = CAM_H / AI_H

try:
    cam = camera.Camera(CAM_W, CAM_H, detector.input_format(), buff_num=1)
except Exception:
    cam = camera.Camera(CAM_W, CAM_H, detector.input_format())

disp = display.Display()

RECORD_HTML ='''
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>MaixCAM 钢珠录制</title>
<style>
html,body{margin:0;background:#111;color:#fff;font-family:Arial,sans-serif}
.bar{position:sticky;top:0;z-index:10;padding:10px;background:#1b1b1b;display:flex;gap:10px;align-items:center;flex-wrap:wrap}
button{font-size:18px;padding:10px 18px;border:0;border-radius:6px;cursor:pointer}
button:disabled{opacity:.45}
#startBtn{background:#18a058;color:white}#stopBtn{background:#d03050;color:white}
#playBtn{background:#2080f0;color:white}#downloadBtn{background:#f0a020;color:black}
#status{font-size:18px}.stage{padding:10px}#src{display:none}
canvas,video{width:min(96vw,1280px);height:auto;display:block;background:#000}
video{margin-top:10px}
</style>
</head>
<body>
<div class="bar">
<button id="startBtn">开始录制</button>
<button id="stopBtn" disabled>停止录制</button>
<button id="playBtn" disabled>回放</button>
<button id="downloadBtn" disabled>下载视频</button>
<span id="status">准备就绪</span>
</div>
<div class="stage">
<img id="src" src="/stream">
<canvas id="canvas"></canvas>
<video id="playback" controls></video>
</div>
<script>
const img=document.getElementById("src"),canvas=document.getElementById("canvas"),ctx=canvas.getContext("2d");
const startBtn=document.getElementById("startBtn"),stopBtn=document.getElementById("stopBtn");
const playBtn=document.getElementById("playBtn"),downloadBtn=document.getElementById("downloadBtn");
const statusText=document.getElementById("status"),playback=document.getElementById("playback");
let recorder=null,chunks=[],videoBlob=null,videoUrl=null,timer=null,startTime=0;
let canvasTrack=null;
function drawLoop(){
 if(img.complete&&img.naturalWidth>0){
  if(canvas.width!==img.naturalWidth||canvas.height!==img.naturalHeight){
   canvas.width=img.naturalWidth;canvas.height=img.naturalHeight;
  }
  ctx.drawImage(img,0,0,canvas.width,canvas.height);
  if(canvasTrack&&canvasTrack.readyState==="live")canvasTrack.requestFrame();
 }
 requestAnimationFrame(drawLoop);
}
drawLoop();
function startTimer(){
 startTime=Date.now();
 timer=setInterval(()=>{statusText.innerText="正在录制 "+((Date.now()-startTime)/1000).toFixed(1)+" 秒";},100);
}
function stopTimer(){if(timer){clearInterval(timer);timer=null;}}
startBtn.onclick=()=>{
 if(!canvas.captureStream){alert("请使用 Chrome 或 Edge 浏览器");return;}
 chunks=[];videoBlob=null;playback.removeAttribute("src");playback.load();
 let canvasStream=canvas.captureStream(0);
 canvasTrack=canvasStream.getVideoTracks()[0];
 try{recorder=new MediaRecorder(canvasStream,{mimeType:"video/webm;codecs=vp8"});}
 catch(e){recorder=new MediaRecorder(canvasStream);}
 recorder.ondataavailable=e=>{if(e.data&&e.data.size>0)chunks.push(e.data);};
 recorder.onstop=()=>{
  stopTimer();canvasTrack=null;videoBlob=new Blob(chunks,{type:"video/webm"});
  if(videoUrl)URL.revokeObjectURL(videoUrl);
  videoUrl=URL.createObjectURL(videoBlob);playback.src=videoUrl;
  startBtn.disabled=false;stopBtn.disabled=true;playBtn.disabled=false;downloadBtn.disabled=false;
  statusText.innerText="录制完成";
 };
 recorder.start();startTimer();
 startBtn.disabled=true;stopBtn.disabled=false;playBtn.disabled=true;downloadBtn.disabled=true;
};
stopBtn.onclick=()=>{if(recorder&&recorder.state==="recording")recorder.stop();};
playBtn.onclick=()=>{if(videoUrl){playback.currentTime=0;playback.play();}};
downloadBtn.onclick=()=>{if(!videoUrl)return;const a=document.createElement("a");a.href=videoUrl;a.download="ball_test.webm";a.click();};
</script>
</body>
</html>
'''

stream = http.JpegStreamer()
stream.set_html(RECORD_HTML)
stream.start()
CROP_W = CAM_W // 16 * 16
CROP_H = int(CAM_H * CROP_H_RATIO) // 16 * 16
crop_x = (CAM_W - CROP_W) // 2
crop_y = (CAM_H - CROP_H) // 2
crop_sx = CROP_W / AI_W
crop_sy = CROP_H / AI_H

# detection-only crop (tighter vertical range, full width)
DET_H = int(CROP_H * DETECT_H_RATIO) // 16 * 16
det_x = (CROP_W - CROP_W) // 2  # 0 — full width
det_y = (CROP_H - DET_H) // 2 + DETECT_Y_OFFSET
det_sx = CROP_W / AI_W
det_sy = DET_H / AI_H

import socket as _socket
try:
    _s = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM)
    _s.connect(("8.8.8.8", 80))
    DEVICE_IP = _s.getsockname()[0]
    _s.close()
except Exception:
    DEVICE_IP = "?.?.?.?"
del _socket

print("stream url: http://{}:{}".format(DEVICE_IP, stream.port()))
print("camera: {}x{}  crop: {}x{}  model: {}x{}".format(CAM_W, CAM_H, CROP_W, CROP_H, AI_W, AI_H))

CENTER_X = CROP_W // 2
CENTER_Y = CROP_H // 2

PIXELS_PER_CM = 16.0 * crop_sx
target_cm = -2.4

# aux red lines (cm), shorter, spread out — edit to taste
aux_targets_cm = [-11.6,9.8]

# ---- calibration: piecewise linear pixel→cm ----
# nominal cm values used for the red lines (main + aux, sorted left→right)
_calib_nominal = sorted([target_cm] + aux_targets_cm)
# real-world cm these lines represent (12 cm spacing, main=reference at 0)
_calib_real = [-12, 0, 12]
# pixel positions of each calibration line
_calib_px = [int(CENTER_X + v * PIXELS_PER_CM) for v in _calib_nominal]

fps = 0.0
fps_last_ms = time.ticks_ms()
frame_id = 0

cached_ball_info = None

# ---- 微分预测状态 ----
vel_ema = 0.0            # 速度 EMA (cm/s)
prev_x_cm = None         # 上一次 x_cm (cm), None 表示未初始化
prev_time_ms = 0          # 上一次检测到球的时间戳 (ms)

# ---- 加速度估算状态 (速度差分 → 加速度, 回传给 STM32 做前馈) ----
ACCEL_EMA_ALPHA = 0.5     # 加速度 EMA 平滑系数 (0-1, 越大越灵敏)
ACCEL_SCALE = 10.0        # 加速度编码: |a|/10 → 0~255, 覆盖 ±2550 cm/s²
accel_ema = 0.0           # 加速度 EMA (cm/s²)
prev_vel = 0.0            # 上一次速度 (cm/s)
prev_vel_ms = 0           # 上一次速度的时间戳 (ms)


def predict_x(x_cm, t_ms):
    """
    微分预测补偿: 通过历史位置变化率预估当前真实位置。
    返回 (x_pred, velocity) — x_pred 为补偿后的输出坐标, velocity 为当前速度 (cm/s)。
    """
    global vel_ema, prev_x_cm, prev_time_ms

    if prev_x_cm is None:
        prev_x_cm = x_cm
        prev_time_ms = t_ms
        vel_ema = 0.0
        return x_cm, 0.0

    dt = (t_ms - prev_time_ms) / 1000.0

    if dt <= 0.001:
        return x_cm + KD * vel_ema, vel_ema

    raw_vel = (x_cm - prev_x_cm) / dt          # cm/s
    vel_ema = vel_ema * (1.0 - VEL_EMA_ALPHA) + raw_vel * VEL_EMA_ALPHA

    prev_x_cm = x_cm
    prev_time_ms = t_ms

    x_pred = x_cm + KD * vel_ema
    return x_pred, vel_ema


def reset_predictor():
    """丢球时重置预测器, 避免过时数据污染下次计算。"""
    global vel_ema, prev_x_cm, prev_time_ms
    global accel_ema, prev_vel, prev_vel_ms
    vel_ema = 0.0
    prev_x_cm = None
    prev_time_ms = 0
    accel_ema = 0.0
    prev_vel = 0.0
    prev_vel_ms = 0


def update_accel(vel, t_ms):
    """由速度差分估算加速度 (cm/s²), 带 EMA 平滑。

    返回平滑后的加速度 accel_ema, 用于回传给 STM32 做加速度前馈。
    """
    global accel_ema, prev_vel, prev_vel_ms

    if prev_vel_ms == 0:
        prev_vel = vel
        prev_vel_ms = t_ms
        accel_ema = 0.0
        return 0.0

    dt = (t_ms - prev_vel_ms) / 1000.0
    prev_vel_ms = t_ms

    if dt <= 0.001:
        return accel_ema

    raw_acc = (vel - prev_vel) / dt          # cm/s²
    prev_vel = vel
    accel_ema = accel_ema * (1.0 - ACCEL_EMA_ALPHA) + raw_acc * ACCEL_EMA_ALPHA
    return accel_ema


def send_x_packet(x_cm, accel_cm_s2):
    """发送 7 字节帧: 位置 + 加速度。

    帧格式: [0xAA][0x08][sign][value][a_sign][a_value][checksum]
      sign/value   : 位置 (|x_cm|*10)
      a_sign/a_value: 加速度 (|accel|/ACCEL_SCALE), ACCEL_SCALE=10
      checksum     : 前 6 字节求和 & 0xFF
    """
    if serial is None:
        return

    x_scaled = int(round(x_cm * 10))

    if x_scaled < 0:
        sign_x = 1
        x_val = -x_scaled
    else:
        sign_x = 0
        x_val = x_scaled

    if x_val > 255:
        x_val = 255

    a_scaled = int(round(accel_cm_s2 / ACCEL_SCALE))

    if a_scaled < 0:
        sign_a = 1
        a_val = -a_scaled
    else:
        sign_a = 0
        a_val = a_scaled

    if a_val > 255:
        a_val = 255

    packet = [0xAA, 0x08, sign_x, x_val, sign_a, a_val]
    checksum = sum(packet) & 0xFF
    serial.write(bytes(packet) + bytes([checksum]))


def project_to_cm(cx, cy):
    """Piecewise-linear pixel→cm using red-line calibration points."""
    if cx <= _calib_px[0]:
        # left of first cal point: extrapolate using first segment slope
        dx = cx - _calib_px[0]
        slope_denom = _calib_px[1] - _calib_px[0]
        slope = (_calib_real[1] - _calib_real[0]) / slope_denom if slope_denom else 0
        return _calib_real[0] + dx * slope
    if cx >= _calib_px[-1]:
        # right of last cal point: extrapolate using last segment slope
        dx = cx - _calib_px[-1]
        slope_denom = _calib_px[-1] - _calib_px[-2]
        slope = (_calib_real[-1] - _calib_real[-2]) / slope_denom if slope_denom else 0
        return _calib_real[-1] + dx * slope
    # interpolate between two calibration points
    for i in range(len(_calib_px) - 1):
        if _calib_px[i] <= cx <= _calib_px[i + 1]:
            t = (cx - _calib_px[i]) / (_calib_px[i + 1] - _calib_px[i])
            return _calib_real[i] + t * (_calib_real[i + 1] - _calib_real[i])
    return 0.0


def point_at_cm(x_cm):
    px = int(CENTER_X + x_cm * PIXELS_PER_CM)
    py = CENTER_Y
    return px, py


x_cm = 0.0
x_pred = 0.0
vel = 0.0
accel = 0.0

while not app.need_exit():
        img = cam.read()
        frame_id += 1

        fps_now = time.ticks_ms()
        fps_dt = max(1, fps_now - fps_last_ms) / 1000.0
        fps_last_ms = fps_now
        fps = fps * 0.85 + (1.0 / fps_dt) * 0.15

        img = img.crop(crop_x, crop_y, CROP_W, CROP_H)

        if frame_id % DETECT_EVERY_N == 0:
            detect_img = img.resize(AI_W, AI_H)

            objs = detector.detect(
                detect_img,
                conf_th=CONF_TH,
                iou_th=IOU_TH,
                sort=1
            )
            del detect_img

            for obj in objs:
                obj.x = int(obj.x * crop_sx)
                obj.y = int(obj.y * crop_sy)
                obj.w = int(obj.w * crop_sx)
                obj.h = int(obj.h * crop_sy)

            # filter: only keep detections inside the vertical detection band
            if DETECT_H_RATIO < 1.0:
                band_top = det_y
                band_bot = det_y + DET_H
                objs = [o for o in objs
                        if o.y + o.h // 2 >= band_top
                        and o.y + o.h // 2 <= band_bot]

            ball = objs[0] if objs else None

            if frame_id % DEBUG_EVERY == 0:
                top_scores = ", ".join(
                    ["{:.3f}".format(objs[i].score) for i in range(min(5, len(objs)))]
                ) if len(objs) > 0 else "none"
                print("th={:.2f} top5=[{}] ball={} | raw={:.2f}cm pred={:.2f}cm vel={:.1f}cm/s acc={:.0f}cm/s2".format(
                    CONF_TH, top_scores, "found" if ball else "miss",
                    x_cm if ok else 0.0, x_pred, vel, accel if ok else 0.0
                ))

            if ball:
                cached_ball_info = {
                    "x": ball.x, "y": ball.y, "w": ball.w, "h": ball.h,
                    "cx": ball.x + ball.w / 2, "cy": ball.y + ball.h / 2,
                    "score": ball.score,
                }
            else:
                cached_ball_info = None

        ok = 0
        x_cm = 0.0
        x_pred = 0.0
        vel = 0.0
        t_now = time.ticks_ms()

        if cached_ball_info is not None:
            bi = cached_ball_info
            cx = bi["cx"]; cy = bi["cy"]
            x_cm = project_to_cm(cx, cy)
            ok = 1
            # 微分预测补偿
            x_pred, vel = predict_x(x_cm, t_now)
            # 加速度: 速度差分 + EMA 平滑, 回传给 STM32 做前馈
            accel = update_accel(vel, t_now)
            # 检测到球才发包 (位置 + 加速度)
            send_x_packet(x_pred, accel)
        else:
            # 丢球: 重置预测器, 停止发包
            # STM32 侧 200ms 无新帧即判定视觉超时 → 电机回零保持。
            # 注意: 绝不能在丢球时发 x=0, 否则 PID 会误以为球在中心。
            reset_predictor()

        if frame_id % VISUAL_EVERY == 0:
            if cached_ball_info is not None:
                bi = cached_ball_info
                img.draw_rect(bi["x"], bi["y"], bi["w"], bi["h"],
                              color=image.COLOR_GREEN, thickness=2)
                img.draw_cross(int(bi["cx"]), int(bi["cy"]),
                               color=image.COLOR_RED, size=8, thickness=2)
                img.draw_string(
                    bi["x"],
                    max(0, bi["y"] - 16),
                    "ball {:.2f}".format(bi["score"]),
                    color=image.COLOR_GREEN
                )

            # detection region outline
            img.draw_rect(det_x, det_y, CROP_W, DET_H,
                          color=image.COLOR_YELLOW, thickness=1)

            # X-axis
            img.draw_line(
                0, CENTER_Y,
                CROP_W - 1, CENTER_Y,
                color=image.COLOR_BLUE,
                thickness=2
            )

            # Y-axis — follows target position
            tx, ty = point_at_cm(target_cm)
            img.draw_line(
                tx, CENTER_Y - 35,
                tx, CENTER_Y + 35,
                color=image.COLOR_BLUE,
                thickness=2
            )

            # origin cross — follows target position
            img.draw_cross(
                tx, CENTER_Y,
                color=image.COLOR_WHITE,
                size=8, thickness=2
            )

            # target x position (long line)
            img.draw_line(
                tx, CENTER_Y - 45,
                tx, CENTER_Y + 45,
                color=image.COLOR_RED,
                thickness=2
            )

            # aux red lines (shorter, ±25)
            for acm in aux_targets_cm:
                ax, _ = point_at_cm(acm)
                img.draw_line(
                    ax, CENTER_Y - 25,
                    ax, CENTER_Y + 25,
                    color=image.COLOR_RED,
                    thickness=2
                )

            img.draw_string(
                4, 4,
                "{}:{}".format(DEVICE_IP, stream.port()),
                color=image.COLOR_YELLOW
            )
            img.draw_string(
                4, 24,
                "fps={:.1f}".format(fps),
                color=image.COLOR_YELLOW
            )
            img.draw_string(
                4, 44,
                "raw={:.2f} pred={:.2f}cm ok={}".format(x_cm, x_pred, ok),
                color=image.COLOR_WHITE
            )
            img.draw_string(
                4, 64,
                "vel={:.1f}cm/s".format(vel),
                color=image.COLOR_WHITE
            )

            disp.show(img)
            stream.write(img.to_jpeg())
