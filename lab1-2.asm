AREA Data, DATA, READWRITE
n     DCD 6
ArrayA DCD -10, 11, 20, 50, -20, -3
ArrayB DCD 0, 0, 0, 0, 0, 0       ; 用來存儲排序後的數字的陣列
ArrayC DCD 0, 0, 0, 0, 0, 0       ; 用來存儲排序後的數字的陣列
    
    AREA Array, CODE, READONLY
    ENTRY
start
    LDR R0, =ArrayA  ; 載入 ArrayA 的地址
    LDR R1, =ArrayB  ; 載入 ArrayB 的地址
    LDR R12, =ArrayC ; 載入 ArrayC 的地址
    LDR R2, =n        ; 載入陣列中元素的數量
    LDR R2, [R2]      ; 載入 n 的值
    BL  copy_ArrayB  ; 調用函數從 ArrayA 複製陣列
    BL  copy_ArrayC  ; 調用函數從 ArrayA 複製陣列到 ArrayC
    LDR R0, =ArrayB  ; 現在將 R0 指向 ArrayB 以進行排序
    BL  signedSort    ; 用冒泡排序函數
    LDR R0, =ArrayC  ; 現在將 R0 指向 ArrayC 以進行排序
    BL  unsignedSort  ; 調用冒泡排序函數
    b bsort_skip      ; BREAKPOINT!!!設在這裡
stop

; 執行冒泡排序的函數
; 輸入: R0 = 要排序的陣列地址, R2 = 陣列大小
signedSort
    MOV R5, R2        ; 將大小複製到 R5
    SUB R5, R5, #1    ; 為循環條件遞減大小
    MOV R9, R0        ; 備份 R0 到 R9
signedSort_outer_loop
    MOV R3, R9        ; 每次迭代重置 R3 到陣列的開始
    MOV R4, R5        ; 重置 R4 為內循環的 R5
signedSort_inner_loop
    LDR R6, [R3]      ; 載入當前元素
    LDR R7, [R3, #4]  ; 載入下一個元素
    CMP R6, R7        ; 比較元素
    BLT signedSort_skip    ; 如果順序正確，跳過交換
    STR R6, [R3, #4]  ; 交換元素
    STR R7, [R3]
signedSort_skip
    ADD R3, R3, #4    ; 移動到下一個元素
    SUBS R4, R4, #1   ; 遞減循環計數器
    BNE signedSort_inner_loop
    SUBS R5, R5, #1   ; 遞減外循環計數器
    BNE signedSort_outer_loop
    BX LR             ; 從函數返回
    
unsignedSort
    MOV R5, R2        ; 將大小複製到 R5
    SUB R5, R5, #1    ; 為循環條件遞減大小
    MOV R9, R0        ; 備份 R0 到 R9
unsignedSort_outer_loop
    MOV R3, R9        ; 每次迭代重置 R3 到陣列的開始
    MOV R4, R5        ; 重置 R4 為內循環的 R5
unsignedSort_inner_loop
    LDR R6, [R3]      ; 載入當前元素
    LDR R7, [R3, #4]  ; 載入下一個元素
    CMP R6, R7        ; 比較元素
    STRCS R6, [R3, #4]; 若條件滿足則交換元素
    STRCS R7, [R3]
unsignedSort_skip
    ADD R3, R3, #4    ; 移動到下一個元素
    SUBS R4, R4, #1   ; 遞減循環計數器
    BNE unsignedSort_inner_loop
    SUBS R5, R5, #1   ; 遞減外循環計數器
    BNE unsignedSort_outer_loop
    BX LR             ; 從函數返回

; 從 ArrayA 到 ArrayB 複製陣列的函數
; 輸入: R0 = 源陣列, R1 = 目標陣列, R2 = 陣列大小
copy_ArrayB
    MOV R9, R0        ; 備份 R0 到 R9
    MOV R10, R1       ; 備份 R1 到 R10
    MOV R11, R2       ; 備份 R2 到 R11
copy_loop
    LDR R3, [R9], #4  ; 從源載入單詞並更新地址
    STR R3, [R10], #4 ; 將單詞存儲到目標並更新地址
    SUBS R11, R11, #1 ; 遞減計數器
    BNE copy_loop     ; 如果計數器不為零則繼續循環
    BX LR             ; 從函數返回

; 從 ArrayA 到 ArrayC 複製陣列的函數
copy_ArrayC
    MOV R9, R0        ; 備份 R0 到 R9
    MOV R10, R12      ; 備份 R12 到 R10
    MOV R11, R2       ; 備份 R2 到 R11
copy_loopB
    LDR R3, [R9], #4  ; 從源載入單詞並更新地址
    STR R3, [R10], #4 ; 將單詞存儲到目標並更新地址
    SUBS R11, R11, #1 ; 遞減計數器
    BNE copy_loopB    ; 如果計數器不為零則繼續循環
    BX LR             ; 從函數返回
    
    END

    END