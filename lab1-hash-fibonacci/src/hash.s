    AREA    |.text|, CODE, READONLY
    EXPORT  hash
    IMPORT  hash_result

hash PROC
    PUSH {r4-r7, lr}      ; Save used registers

    MOVS r2, #0           ; r2 = first current (i) and after final (n) length of str 
    MOVS r3, #0           ; r3 = hash

length_func               ; Calculate length of string
    LDRB r1, [r0, r2]     ; r1 = str[i]
    CMP r1, #0            ; End of string?
    BEQ init_hash
    ADD r2, r2, #1
    B length_func

init_hash
    MOV r3, r2              ; hash = length of string

    LDR r5, =digits_table   ; Load address of digits array

    MOV r4, #0              ; r4 = i = 0

for_loop1
    CMP r4, r2              ; i < length?
    BEQ after_loop1

    LDRB r6, [r0, r4]       ; r6 = str[i]

    ;; if capital letter
    CMP r6, #'A'
    BLT check_lower

    CMP r6, #'Z'
    BGT check_lower

    ADD r7, r6, r6          ; r7 = 2 * str[i]
    ADD r3, r3, r7          ; hash += 2 * str[i]
    B end_check

check_lower
    ;; if lowercase letter
    CMP r6, #'a'
    BLT check_number

    CMP r6, #'z'
    BGT check_number

    SUB r7, r6, #'a'      ; r7 = str[i] - 'a'
    MUL r7, r7, r7        ; r7 = (str[i] - 'a')^2
    ADD r3, r3, r7        ; hash += (str[i] - 'a')^2
    B end_check

check_number
    ;; if number
    CMP r6, #'0'
    BLT end_check

    CMP r6, #'9'
    BGT end_check

    SUB r7, r6, #'0'         ; r7 = str[i] - '0'
    LDR r7, [r5, r7, LSL #2] ; r7 = digits[r7]
    ADD r3, r3, r7           ; hash += digits[r7]

end_check
    ADD r4, r4, #1
    B for_loop1

after_loop1
    ;; store result to memory
    LDR r1, =hash_result
    STR r3, [r1]

    MOV r0, r3              ; return hash in r0
    POP {r4-r7, lr}
    BX lr
    ENDP

digits_table                ; The digit table provided
    DCD 5, 12, 7, 6, 4, 11, 6, 3, 10, 23

    END

