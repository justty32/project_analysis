# LISP/c 基礎教學 (01)：從 C 到 Lispsy 的語法映射

本章將詳細介紹如何將 C 語言中最基礎的元素轉換為 LISP/c 語法。

## 1. 變數與型別 (Variables & Types)

在 LISP/c 中，宣告變數使用 `var` 或 `vars`。

### 基本宣告
*   **C:** `int a = 10;`
*   **Lispsy:** `(var a int 10)`

### 常見型別對照
LISP/c 提供了一些更具語意化的別名（Synonyms）：
| C 型別 | Lispsy 名稱 | 別名 |
| :--- | :--- | :--- |
| `int` | `int` | `integer` |
| `long` | `long` | `integer+` |
| `float` | `float` | `real` |
| `double` | `double` | `real+` |
| `unsigned int` | `natural` | |
| `char` | `char` | `boolean` |

### 一次宣告多個變數
*   **C:** `int x, y; float z;`
*   **Lispsy:** `(vars (x y) (z float))` (預設型別為 `int`)

---

## 2. 流程控制 (Control Flow)

### 條件判斷 (if / else)
在 Lisp 中，如果 `if` 區塊需要執行多個指令，必須使用 `progn` 包起來（類似 C 的 `{}`）。
*   **C:**
    ```c
    if (x > 0) {
        printf("Positive");
        x--;
    } else {
        printf("Non-positive");
    }
    ```
*   **Lispsy:**
    ```lisp
    (if (> x 0)
        (progn
            (@printf (str "Positive"))
            (-- x))
        (@printf (str "Non-positive")))
    ```

### 迴圈 (Loops)
*   **For 迴圈:** `(for [初始] [條件] [步進] [主體])`
    *   **C:** `for(int i=0; i<10; i++) { ... }`
    *   **Lispsy:** `(for (var i int 0) (< i 10) (++ i) ...)`
*   **While 迴圈:**
    *   **C:** `while(cond) { ... }`
    *   **Lispsy:** `(while cond ...)`

---

## 3. 函數 (Functions)

### 定義函數
語法結構：`(func [名稱] [回傳型別] ([參數清單]) [主體])`
*   **C:**
    ```c
    float square(float n) {
        return n * n;
    }
    ```
*   **Lispsy:**
    ```lisp
    (func square float ((n float))
        (return (* n n)))
    ```

### 主程式 (Main)
*   **C:** `int main(int argc, char** argv) { ... }`
*   **Lispsy:** `(main ...)` (會自動處理 `argc` 和 `argv`)

---

## 4. 預處理器 (Preprocessor)

### 標頭檔
*   **C:** `#include <stdio.h>`
*   **Lispsy:** `(header stdio)`
*   **一次包含多個:** `(headers stdio stdlib string)`

### 宏定義 (#define)
*   **C:** `#define PI 3.14`
*   **Lispsy:** `(define !pi 3.14)` (注意 `!` 會轉換成大寫 `PI`)

### 條件編譯
*   **C:** `#ifdef DEBUG ... #endif`
*   **Lispsy:** `(ifdef debug ...)`
