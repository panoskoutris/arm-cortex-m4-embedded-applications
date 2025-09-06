    AREA |.text|, CODE, READONLY
    EXPORT addmod7
    IMPORT mod7_result

addmod7 PROC 
    PUSH {r4-r6, lr}          ; Save used registers

    MOVS r1, #0               ; r1 = sum
    MOVS r5, #10              ; r5 = constant 10
    MOVS r6, #7               ; r6 = constant 7

    MOV r4, r0                ; In case hash < 9 return hash

    ;; if (hash > 9)
    CMP r0, #9               	
    BLT after_if              ; skip if hash <= 9

hash_greater_zero
    CMP r0, #0               ; hash = 0
    BLE mod7_loop            ; end loop if hash <= 0

    ;; sum = sum + hash % 10
    UDIV r2, r0, r5           ; r2 = hash / 10
    MLS  r3, r2, r5, r0       ; r3 = hash - (r2 * 10) = hash % 10
    ADD  r1, r1, r3           ; sum += r3
    MOV  r0, r2               ; hash = hash / 10
    B hash_greater_zero

mod7_loop
    CMP r1, #9
    BLE after_if              ; exit loop if sum <= 9

    
    
    UDIV r2, r1, r6           ; sum = sum / 7;
    MLS  r4, r2, r6, r1       ; rest = sum % 7;
    MOV  r1, r2
    B mod7_loop

after_if 
    ;; store result to memory
    LDR r1, =mod7_result
    STR r4, [r1]

    MOV r0, r4                ; return value in r0
    POP {r4-r6, lr}
    BX lr
    ENDP

    END



