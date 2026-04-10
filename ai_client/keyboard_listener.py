import configparser
from queue import Queue
from pynput.keyboard import Key, Listener
import time
"""
鍵盤監聽器 (Keyboard Listener)

此模組負責監聽系統鍵盤事件，並偵測複雜的按鍵行為（如長按、連擊與組合鍵）。
偵測結果會封裝成一個 Tuple 放入 key_signal_queue 中。

輸出信號格式 (key_signal_queue):
    (signal_type, combo_name, modifiers_dict)
    - signal_type (str): 行為類型。
        - "single_tap": 單次點擊（在 threshold 內放開）。
        - "double_tap": 雙擊。
        - "triple_tap": 三連擊（或更多）。
        - "long_press_hold": 長按發生（按下時長超過 long_press_threshold）。
        - "long_press_release": 長按後的放開動作。
    - combo_name (str): 組合鍵名稱字串。
        - 例如: "ctrl + alt + x", "shift + s", "f12"。
        - 可以用 KeyboardListener.parse_combo_name(combo_name) 來解析成["ctrl", "alt", "x"]。
    - modifiers_dict (dict): 目前修飾鍵的狀態快照。
        - 例如: {"alt": True, "ctrl": False, "shift": False}。

行為邏輯說明:
    1. 組合鍵生成: 程式會根據按下主鍵時，Alt/Ctrl/Shift 的狀態自動串接成 "ctrl + x" 這種格式。
    2. 連擊判定: 相同 combo_name 在 double_tap_threshold 秒內連續釋放，會增加連擊次數。
    3. 長按判定: 按住任意主鍵超過 long_press_threshold 秒，會視為長按。
"""

class KeyboardListener:
    def __init__(self, config: configparser.ConfigParser, key_signal_queue: Queue):
        self._key_signal_queue = key_signal_queue
        self._listener: Listener | None = None
        self._modifiers = {"alt": False, "ctrl": False, "shift": False}
        # 讀取設定值
        self.double_tap_threshold = config.getfloat('keyboard', 'double_tap_threshold', fallback=0.3)
        self.long_press_threshold = config.getfloat('keyboard', 'long_press_threshold', fallback=0.5)
        # 狀態管理
        # key_state 儲存格式: { key_object: (combo_name, start_time) }
        self._key_state = {}           
        self._last_release_times = {}  # { combo_name: timestamp }
        self._tap_counts = {}          # { combo_name: count }
    
    def start(self):
        self._listener = Listener(on_press=self._on_press, on_release=self._on_release)
        self._listener.daemon = True
        self._listener.start()

    def stop(self):
        if self._listener:
            self._listener.stop()

    def parse_combo_name(combo_name: str) -> list[str]:
        """
        解析組合鍵名稱字串，將其轉換為按鍵列表。
        
        範例:
        "ctrl + alt + x" -> ["ctrl", "alt", "x"]
        "shift + s"      -> ["shift", "s"]
        "f12"            -> ["f12"]
        "ctrl + +"       -> ["ctrl", "+"] (處理加號鍵的情況)
        """
        if not combo_name:
            return []
        # 使用 " + " (包含前後空格) 作為分隔符，可以精確區分修飾鍵與加號鍵本身
        return combo_name.split(" + ")

    def _on_press(self, key):
        # 1. 先更新修飾鍵狀態 (這會影響後續 combo_name 的生成)
        self._update_modifiers(key, pressed=True)
        
        # 2. 如果這是一個功能性按鍵 (Alt/Ctrl/Shift)，我們通常不把它當作組合鍵的主體
        if self._is_modifier_key(key):
            return

        # 3. 獲取當前按鍵組合的唯一名稱 (例如: "ctrl + alt + x")
        key_name = self._get_key_name(key)
        if not key_name:
            return
            
        combo_name = self._generate_combo_name(key_name)

        # 4. 僅在第一次按下時記錄，避免重複觸發
        if key not in self._key_state:
            self._key_state[key] = (combo_name, time.time())

    def _on_release(self, key):
        # 1. 嘗試從狀態中找出該按鍵當初按下的 combo 名稱與時間
        state = self._key_state.pop(key, None)
        
        # 2. 更新修飾鍵狀態 (在處理完主鍵後才更新，確保邏輯一致)
        self._update_modifiers(key, pressed=False)

        if state is None:
            return

        combo_name, start_time = state
        now = time.time()
        duration = now - start_time

        # --- 判斷邏輯 ---
        if duration >= self.long_press_threshold:
            self._handle_long_press(combo_name)
        else:
            self._handle_taps(combo_name, now)

    def _handle_long_press(self, combo_name):
        """處理長按行為"""
        # 這裡的 modifiers 已經由 combo_name 字串包含了，所以直接回傳 combo_name
        self._key_signal_queue.put(("long_press_hold", combo_name, self._modifiers.copy()))
        self._key_signal_queue.put(("long_press_release", combo_name, self._modifiers.copy()))
        
        self._tap_counts[combo_name] = 0
        self._last_release_times[combo_name] = 0

    def _handle_taps(self, combo_name, now):
        """處理連擊行為"""
        last_release = self._last_release_times.get(combo_name, 0)
        
        # 如果這次鬆開距離上次該 combo 鬆開的時間在閾值內，計數累加
        if (now - last_release) < self.double_tap_threshold:
            self._tap_counts[combo_name] = self._tap_counts.get(combo_name, 0) + 1
        else:
            self._tap_counts[combo_name] = 1

        count = self._tap_counts[combo_name]
        self._last_release_times[combo_name] = now
        
        signal_map = {1: "single_tap", 2: "double_tap"}
        signal = signal_map.get(count, "triple_tap")
        
        # 傳遞 combo_name 作為 key 資訊
        self._key_signal_queue.put((signal, combo_name, self._modifiers.copy()))

    def _generate_combo_name(self, key_name):
        """根據目前的修飾鍵狀態生成組合鍵名稱"""
        parts = []
        if self._modifiers["ctrl"]: parts.append("ctrl")
        if self._modifiers["alt"]: parts.append("alt")
        if self._modifiers["shift"]: parts.append("shift")
        
        # 如果該鍵本身就是修飾鍵，避免重複顯示 (例如不想出現 "alt + alt")
        if key_name not in parts:
            parts.append(key_name)
            
        return " + ".join(parts)

    def _is_modifier_key(self, key):
        """判斷是否為修飾鍵本身"""
        return key in (Key.alt_l, Key.alt_r, Key.ctrl_l, Key.ctrl_r, Key.ctrl, Key.shift, Key.shift_l, Key.shift_r)

    def _update_modifiers(self, key, pressed: bool):
        """統一更新修飾鍵狀態"""
        if key in (Key.alt_l, Key.alt_r): self._modifiers["alt"] = pressed
        elif key in (Key.ctrl_l, Key.ctrl_r, Key.ctrl): self._modifiers["ctrl"] = pressed
        elif key in (Key.shift, Key.shift_r, Key.shift_l): self._modifiers["shift"] = pressed

    def _get_key_name(self, key):
        """標準化按鍵名稱輸出"""
        try:
            if hasattr(key, "name") and key.name:
                return key.name.lower()
            if hasattr(key, "char") and key.char:
                return key.char.lower()
        except Exception:
            pass
        return str(key).lower()