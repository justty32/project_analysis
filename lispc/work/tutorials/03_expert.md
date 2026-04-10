# LISP/c 專家教學 (03)：C++ 支援、MPI 與命名轉換

本章介紹 LISP/c 針對現代開發環境（C++）與平行運算（MPI）的深度整合。

## 1. C++ 深度支援
LISP/c 不只是翻譯 C，它也能寫出優雅的 C++。

### 核心關鍵字
*   `(headers++ iostream)` -> 生成 `#include <iostream>` (不加 .h)。
*   `(using std)` -> 生成 `using namespace std;`。
*   `<<+` 與 `>>+`：流操作符（串接多個參數且不加多餘括號）。
*   `(new ...)` 與 `(delete[] ...)`。

### 完整 C++ 範例：Vector 類別
```lisp
(headers++ iostream)
(using std)

(class vec
    (public
        (var x float)
        (var y float)
        ;; 建構子 (cx)
        (cx ((f1 float) (f2 float)) 
            ((x f1) (y f2)))
        ;; 運算子重載 (+)
        (op + vec ((v (t& vec)))
            (var res vec)
            (= (slot res x) (+ x (slot v x)))
            (= (slot res y) (+ y (slot v y)))
            (return res))))

(main
    (v (@v1 1.0 2.0) vec)
    (v (@v2 3.0 4.0) vec)
    (v v3 vec (+ v1 v2))
    (<<+ cout (str "Result: ") (slot v3 x) (str ", ") (slot v3 y) endl)
    (return 0))
```

## 2. MPI 平行運算
LISP/c 使用「斜線語法」來對應 MPI 的複雜常數與函數。

| Lispsy 語法 | 編譯後的 C 程式碼 |
| :--- | :--- |
| `mpi/comm/world` | `MPI_COMM_WORLD` |
| `mpi/init` | `MPI_Init` |
| `mpi/success` | `MPI_SUCCESS` |

### MPI 範例
```lisp
(headers mpi stdio)
(main
    (vars (rank size))
    (@mpi/init (addr argc) (addr argv))
    (@mpi/comm/rank mpi/comm/world (addr rank))
    (@mpi/printf (str "Hello from rank %d\\n") rank)
    (@mpi/finalize))
```

## 3. 命名轉換工具 (Naming Tools)
當您需要轉換變數風格（例如 C++ 常用的 CamelCase）時，LISP/c 提供極速轉換：

*   **`=` (大寫開頭)**：`(=this-is-test)` -> `ThisIsTest`
*   **`%` (小寫開頭)**：`(%this-is-test)` -> `thisIsTest`
*   **`!` (全大寫)**：`(!my-const)` -> `MY_CONST`

## 4. 進階編譯指令
在 CLISP REPL 中，您可以使用更細緻的編譯指令：

```lisp
;; 指定編譯器與連結庫
(compile-cl-file "main.cl" :cc "nvcc" :libs "-lm -lpthread")
```
