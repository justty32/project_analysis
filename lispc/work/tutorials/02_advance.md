# LISP/c 進階教學 (02)：指標、結構與宏的威力

本章將帶領你掌握 LISP/c 中最具威力的部分：複雜的 C 指標、結構、強大的模板系統，以及對高效能運算的支持。

## 1. 指標 (Pointers)

在 Lisp 中宣告指標有兩種主要方式。

### 變數宣告中的指標
*   **C:** `int *p = NULL;`
*   **Lispsy:** `(var (pt p) int !null)`
    *   使用 `pt` 來修飾變數名稱。
*   **雙重指標:** `char **argv` 寫作 `((pt argv 2) char)`。

### 型別中的指標
*   **C:** `void* ptr;`
*   **Lispsy:** `(var ptr (t* void))`

### 指標操作
*   **取位址 (&):** `(&x)` 或 `(addr x)`。
*   **解引用 (*):** `(*p)` 或 `(ptr p)`。
*   **結構存取 (->):** `(slot p member)`。

---

## 2. 陣列與結構 (Arrays & Structs)

### 陣列
*   **C:** `int arr[10];`
*   **Lispsy:** `(var (arr arr 10) int)`
*   **存取:** `([]arr i)` 或 `(nth arr i)`。

### 結構 (Structs)
*   **C 定義:**
    ```c
    struct point {
        int x;
        int y;
    };
    ```
*   **Lispsy 定義:**
    ```lisp
    (struct point (
        x
        y))
    ```

---

## 3. 模板系統 (Templates)

這是 LISP/c 超越傳統 C 預處理器的核心功能。它允許你寫出「會生成程式碼」的程式碼。

### 實作 Generic 函數
*   **Lispsy:**
    ```lisp
    (template swap-gen (typ)
        (func (sym/add swap- typ) void (((pt a) typ) ((pt b) typ))
            (var tmp typ (* a))
            (= (* a) (* b))
            (= (* b) tmp)))

    ;; 自動產生兩個 C 函數
    (swap-gen int)   ;; void swap_int(int *a, int *b)
    (swap-gen float) ;; void swap_float(float *a, float *b)
    ```

---

## 4. 高效能運算支援 (HPC Support)

LISP/c 針對並行運算（Parallel Computing）內建了許多關鍵字與別名。

### Pthreads 範例
你可以直接使用 `pthread/create` 等名稱，它們會正確編譯成 C 的 `pthread_create`。
```lisp
(headers pthread)
(main
    (var (arr threads !nthreads) pthread-t)
    (@pthread/create (addr ([]threads i)) !null (addr func) !null))
```

### CUDA 核心宣告
使用 `cuda/global` 或 `cuda/device` 來定義 GPU 核心：
*   **Lispsy:**
    ```lisp
    (cuda/global kernel void (((pt data) float))
        (var i int thread/idx/x) ;; threadIdx.x
        (= ([]data i) (* ([]data i) 2.0f)))
    ```

---

## 5. 總結：開發心法

1.  **善用縮寫：** 熟練後多用 `@`、`[]`、`->` 等縮寫。
2.  **變數命名：** 習慣 `kebab-case`（`my-var`），產出的 C 會是標準的 `snake_case`（`my_var`）。
3.  **大寫符號：** 所有的 `#define` 和關鍵字常數請加 `!`。
4.  **括號哲學：** 只要記住「括號內第一個字是動作」，你就能輕鬆閱讀任何 LISP/c 代碼。
