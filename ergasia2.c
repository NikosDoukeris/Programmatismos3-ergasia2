#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include "ergasialib.h"
#define NUM_OF_CUSTOMERS 5
#define LIST_LEN 20
//write 1, read 0

struct product{
    char description[256];
    int price;
    int item_count;
    int item_req;
    int sold;
    int unserved;
};

int main(void){

    char a = 'a';//ack
    struct product products[LIST_LEN];
    int failed_sum=0, done_sum=0, sum=0, money_sum=0;

    for(int i=0; i<LIST_LEN; i++){
        // int -> string
        char temp_str[16];
        sprintf(temp_str, "Product %d", i+1);

        strcpy(products[i].description, temp_str);
        products[i].price = randomInt(50);
        products[i].item_count = 2;
        products[i].item_req = 0;
        products[i].sold = 0;
        products[i].unserved = 0;

        cyan();
        printf("%s\n", products[i].description);
        clear();
        printf("Price: %d\n", products[i].price);
        printf("Count: %d\n------------\n", products[i].item_count);
        sleep(1);
    }

    //pipes
    int pipes[NUM_OF_CUSTOMERS*2][2];
    for(int a=0; a<NUM_OF_CUSTOMERS*2; a++){
        if (pipe(pipes[a]) < 0) return 1;
    }

    for(int customer=0; customer<NUM_OF_CUSTOMERS; customer++){
        //pipes per customer: pipes[NUM_OF_CUSTOMERS][0,1] & pipes[NUM_OF_CUSTOMERS * 2][0,1]
        yellow();
        printf("\nCustomer %d:\n", customer+1);
        clear();

        int id = fork();
        if (id == -1) return 2;

        if (id == 0){ //child

            //close unused pipes
            close(pipes[customer][0]);
            close(pipes[customer+NUM_OF_CUSTOMERS][1]);

            read(pipes[customer+NUM_OF_CUSTOMERS][0], &a, sizeof(char));//ack

            for(int order_c=0; order_c<10; order_c++){
                sleep(1);
                int product_index = randomInt(LIST_LEN);
                write(pipes[customer][1], &product_index, sizeof(int));
                read(pipes[customer+NUM_OF_CUSTOMERS][0], &a, sizeof(char));//ack
                sum++;
            }

            write(pipes[customer][1], &a, sizeof(int));//ack

            //close unused pipes
            close(pipes[customer][1]);
            close(pipes[customer+NUM_OF_CUSTOMERS][0]);

            exit(0);
        }
        else { //parent
            //close unused pipes
            close(pipes[customer][1]);
            close(pipes[customer+NUM_OF_CUSTOMERS][0]);

            write(pipes[customer+NUM_OF_CUSTOMERS][1], &a, sizeof(char));//ack
            
            int product_num;
            for(int order_p=0; order_p<10; order_p++){
                read(pipes[customer][0], &product_num, sizeof(int));
                printf("Product id: %d\n", product_num);
                write(pipes[customer+NUM_OF_CUSTOMERS][1], &a, sizeof(char));//ack

                int temp_sum = 0;
                products[product_num].item_req += 1;
                if(products[product_num].item_count > 0){
                    //successfull order
                    //edit struct
                    products[product_num].item_count -= 1;
                    products[product_num].sold += 1;
                    temp_sum += products[product_num].price;

                    //sum
                    done_sum++;
                    money_sum += products[product_num].price;
                }else{
                    //failed order
                    products[product_num].unserved += 1;

                    //sum
                    failed_sum++;
                }
            }

            read(pipes[customer][0], &a, sizeof(char));//ack

            //close unused pipes
            close(pipes[customer][0]);
            close(pipes[customer+NUM_OF_CUSTOMERS][1]);

            wait(NULL);
        }


        usleep(500 * 1000);
    }

    blue();
    printf("\n                             ***********");
    printf("\n                             * RESULTS *");
    printf("\n                             ***********\n");
    clear();

    for(int i=0; i<LIST_LEN; i++){
        printf("%s,  ", products[i].description);
        printf("Price: %d,  ", products[i].price);
        printf("Count: %d,  ", products[i].item_count);
        printf("Requests: %d,  ", products[i].item_req);
        printf("Sold: %d,  ", products[i].sold);
        printf("Unserved: %d\n", products[i].unserved);
    }

    sum = done_sum + failed_sum;
    printf("==================================================================\n");
    printf("Orders:%d, ", sum);
    green();
    printf("Successful orders:%d, ",done_sum);
    clear();
    red();
    printf("Failed orders:%d, ",failed_sum);
    clear();
    printf("Total spent:%d\n",money_sum);
    printf("==================================================================\n");

    return 0;
}