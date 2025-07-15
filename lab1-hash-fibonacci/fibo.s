    AREA |.text|, CODE, READONLY
    EXPORT fibo
    IMPORT fibo_result

fibo PROC
    PUSH {r4-r5, lr}   ;Save used registers
	
    CMP r0, #0
    BEQ return_0
    
    CMP r0, #1
    BEQ return_1
	
    ;; return fibo(n-1) + fibo(n-2)
    MOV r4, r0    ; r4 = r0 = n
    SUBS r0, #1   ; r0 = n-1
    BL fibo
    MOV r5, r0    ; r5 = fibo(n-1)
	
    MOV r0, r4    ; r0 = n
    SUBS r0, #2   ; r0 = n-2
    BL fibo
    ADD r0, r0, r5  ; r0 = fibo(n-1) + fibo(n-2)

    B ending
	
return_1
    MOV r0, #1
    B ending
	
return_0
    MOV r0, #0
    B ending

ending
    ;; store result to memory
    LDR r1, =fibo_result
    STR r0, [r1]

    POP {r4-r5, lr}    ;Load used registers
    BX lr
    ENDP

    END
