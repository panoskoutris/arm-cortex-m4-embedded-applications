#include <stdio.h>

extern int hash_result;
extern int mod7_result;
extern int fibo_result;

extern int hash(char* str);
extern int addmod7(int hash_value);
extern int fibo(int rest_value);

int main(void) {
	
    char str[20];

    // PART 1
    printf("Please give a string: ");
    scanf("%s", str);

    hash(str);  // result stored in memory
    printf("\nHash of string = %d\n", hash_result);

    // PART 2
    addmod7(hash_result);
    printf("Rest = %d\n", mod7_result);

    // PART 3
    fibo(mod7_result);
    printf("Final Result = %d\n", fibo_result);
	
    printf("\n--- Extra Test Cases ---\n");

    // Test 1: string = "C3f9"
    char test1[] = "C3f9";
    hash(test1);
    addmod7(hash_result);
    fibo(mod7_result);
    printf("\nTest 1: string = %s\n", test1);
    printf("Expected hash = 192 | Computed hash = %d\n", hash_result);
    printf("Expected rest = 5   | Computed rest = %d\n", mod7_result);
    printf("Expected Fibonacci = 5 | Computed Fibonacci = %d\n", fibo_result);

    // Test 2: string = "Zz0"
    char test2[] = "Zz0";
    hash(test2);
    addmod7(hash_result);
    fibo(mod7_result);
    printf("\nTest 2: string = %s\n", test2);
    printf("Expected hash = 813 | Computed hash = %d\n", hash_result);
    printf("Expected rest = 5   | Computed rest = %d\n", mod7_result);
    printf("Expected Fibonacci = 5 | Computed Fibonacci = %d\n", fibo_result);


    // Test 3: string = "a7k1q"
    char test3[] = "a7k1q";
    hash(test3);
    addmod7(hash_result);
    fibo(mod7_result);
    printf("\nTest 3: string = %s\n", test3);
    printf("Expected hash = 376 | Computed hash = %d\n", hash_result);
    printf("Expected rest = 2   | Computed rest = %d\n", mod7_result);
    printf("Expected Fibonacci = 1 | Computed Fibonacci = %d\n", fibo_result);

    while (1);  // Keep program alive
	
    return 0;
	
}
