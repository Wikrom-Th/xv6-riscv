//Thamchareonkit Wikrom 72075605
//Prime Number Calculator with Dataflow Model

#include "kernel/types.h"
#include "user/user.h"

int count; 

int main(int argc, char **argv) {
    int* primes;

    if(argc < 2) {
        printf("Usage: %s generation_amount\n", argv[0]);
        return 0;
    }

    count = atoi(argv[1]);

    primes = malloc(sizeof(int) * count);

    //first values to get the calculation going
    primes[0] = 2;
    primes[1] = 3;

    printf("%d\n", 2);
    printf("%d\n", 3);

    int curr_count = 2;
    int curr_num = 3;

    //flag to check whether to add the number to the array or not
    int is_prime;

    while(curr_count < count) {

        is_prime = 1;
        curr_num += 2; 

        for(int i = 0; i < curr_count; i++) {
            if (curr_num % primes[i]==0) {
                is_prime = 0;
                break;
            }
        }

        if(is_prime) {
            primes[curr_count] = curr_num;
            printf("%d\n", curr_num);
            curr_count++;
        }
    }

    return 0;
}