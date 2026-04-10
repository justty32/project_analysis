import configparser
import io
import threading
import time
import wave
from queue import Empty, Queue

import numpy as np
import pyaudio

"""
錄音控制器 (Voice Recorder)

此模組負責處理音訊輸入，並根據 VAD (Voice Activity Detection) 邏輯自動切分音訊片段。
透過 Queue 進行非同步通訊。

可用指令 (recorder_cmd_queue):
    - "START": 開始錄音。若已在錄音中則忽略。
    - "STOP" : 停止錄音。會將目前殘餘的音訊內容 Flush 掉。

可能發出的事件 (recorder_event_queue):
    - {"event": "recording_started"}: 成功啟動錄音流程。
    - {"event": "recording_stopped"}: 成功停止錄音流程。
    - {"event": "chunk_flushed"}: 一個音訊片段（WAV 格式）已存入 audio_queue。
    - {"event": "volume", "rms": float}: 每 0.1 秒發送一次目前的音量強度（RMS 值）。
    - {"event": "error", "message": str}: 錄音過程中發生異常（如硬體被拔除、權限問題）。

音訊片段輸出 (audio_queue):
    - 放入的是包含完整 WAV 標頭的 io.BytesIO 物件。
"""


class Recorder:
    """錄音器。透過 recorder_cmd_queue 接收 START/STOP，將 WAV BytesIO 片段放入 audio_queue。

    切片邏輯（VAD 模式）：
    1. 持續錄音至少 chunk_duration 秒（預設 60 秒）
    2. 在 chunk_duration 秒後，若靜音 >= silence_seconds，則切片
    3. max_duration > 0 且錄音時長 >= max_duration → 強制切片
    每次切片後重置計時器，繼續錄音直到收到 STOP。
    """

    def __init__(
        self,
        config: configparser.ConfigParser,
        recorder_cmd_queue: Queue,
        audio_queue: Queue,
        recorder_event_queue: Queue,
    ):
        self._recorder_cmd_queue = recorder_cmd_queue
        self._audio_queue = audio_queue
        self._recorder_event_queue = recorder_event_queue

        audio = config["AUDIO"]
        self._sample_rate = int(audio.get("sample_rate", 16000))
        self._channels = int(audio.get("channels", 1))
        self._chunk_size = int(audio.get("chunk_size", 1024))
        self._chunk_duration = int(audio.get("chunk_duration", 60))
        self._silence_seconds = float(audio.get("silence_seconds", 1.5))
        self._max_duration = int(audio.get("max_duration", 0))
        self._silence_threshold = float(audio.get("silence_threshold", 300.0))

        self._thread: threading.Thread | None = None
        self._running = False

    def start(self):
        self._running = True
        self._thread = threading.Thread(target=self._worker, daemon=True, name="Recorder")
        self._thread.start()

    def stop(self):
        self._running = False

    # ── Worker ─────────────────────────────────────────────────────────

    def _worker(self):
        pa = pyaudio.PyAudio()
        try:
            stream = pa.open(
                format=pyaudio.paInt16,
                channels=self._channels,
                rate=self._sample_rate,
                input=True,
                frames_per_buffer=self._chunk_size,
            )
            self._loop(stream, pa)
        except Exception as exc:
            self._recorder_event_queue.put({"event": "error", "message": str(exc)})
        finally:
            try:
                stream.stop_stream()
                stream.close()
            except Exception:
                pass
            pa.terminate()

    def _loop(self, stream, pa):
        recording = False
        frames: list[bytes] = []
        chunk_start = 0.0
        last_sound = 0.0

        while self._running:
            # ── Process pending commands ───────────────────────────────
            try:
                cmd = self._recorder_cmd_queue.get_nowait()
                if cmd == "START" and not recording:
                    recording = True
                    frames = []
                    chunk_start = time.monotonic()
                    last_sound = time.monotonic()
                    self._recorder_event_queue.put({"event": "recording_started"})
                elif cmd == "STOP" and recording:
                    recording = False
                    self._flush(frames, pa)
                    frames = []
                    self._recorder_event_queue.put({"event": "recording_stopped"})
            except Empty:
                pass

            if not recording:
                time.sleep(0.02)
                continue

            # ── Read audio chunk ───────────────────────────────────────
            data = stream.read(self._chunk_size, exception_on_overflow=False)
            frames.append(data)
            now = time.monotonic()

            # ── VAD & Volume ──────────────────────────────────────────
            rms = _rms(data)
            if rms >= self._silence_threshold:
                last_sound = now
            
            # Send volume event periodically (e.g. every 0.1s)
            if not hasattr(self, "_last_vol_time"):
                self._last_vol_time = 0.0
            if now - self._last_vol_time >= 0.1:
                self._recorder_event_queue.put({"event": "volume", "rms": rms})
                self._last_vol_time = now

            elapsed = now - chunk_start
            silence = now - last_sound

            # ── Slice conditions ───────────────────────────────────────
            # VAD 模式：必須至少錄音 chunk_duration 秒，才開始檢查沉默
            should_slice = (
                (self._max_duration > 0 and elapsed >= self._max_duration)
                or (elapsed >= self._chunk_duration and silence >= self._silence_seconds)
            )

            if should_slice:
                self._flush(frames, pa)
                frames = []
                chunk_start = now
                last_sound = now

    # ── Helpers ────────────────────────────────────────────────────────

    def _flush(self, frames: list[bytes], pa: pyaudio.PyAudio):
        if not frames:
            return
        buf = io.BytesIO()
        with wave.open(buf, "wb") as wf:
            wf.setnchannels(self._channels)
            wf.setsampwidth(pa.get_sample_size(pyaudio.paInt16))
            wf.setframerate(self._sample_rate)
            wf.writeframes(b"".join(frames))
        buf.seek(0)
        self._audio_queue.put(buf)
        self._recorder_event_queue.put({"event": "chunk_flushed"})


def _rms(data: bytes) -> float:
    arr = np.frombuffer(data, dtype=np.int16).astype(np.float32)
    return float(np.sqrt(np.mean(arr ** 2))) if len(arr) else 0.0
