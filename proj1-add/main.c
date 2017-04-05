/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.c
 * Author: Ben Kittner
 *
 * Created on March 26, 2017, 1:14 AM
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
//#include "systemcalls.h"

#define __NR_create_mbox_421 377
#define __NR_remove_mbox_421 378
#define __NR_count_mbox__421 379
#define __NR_list_mbox_421 380
#define __NR_send_msg_421 381
#define __NR_recv_msg_421 382
#define __NR_peek_msg_421 383
#define __NR_count_msg_421 384
#define __NR_len_msg_421 385

long create_mbox_421(unsigned long id, int enable_crypt, int lifo){
	return syscall(__NR_create_mbox_421, id, enable_crypt, lifo);
}

long remove_mbox_421(unsigned long id){
	return syscall(__NR_remove_mbox_421, id);
}

long count_mbox_421(void){
	return syscall(__NR_count_mbox__421);
}

long list_mbox_421(unsigned long *mbxes, long k){
	return syscall(__NR_list_mbox_421, mbxes, k );
}

long send_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key){
	return syscall(__NR_send_msg_421, id, msg, n, key);
}

long recv_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key){
	return syscall(__NR_recv_msg_421, id, msg, n, key);
}

long peek_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key){
	return syscall(__NR_peek_msg_421, id, msg, n, key);
}

long count_msg_421(unsigned long id){
	return syscall(__NR_count_msg_421, id);
}

long len_msg_421(unsigned long id){
	return syscall(__NR_len_msg_421, id);
}



/*
 * 
 */
int main(int argc, char** argv) {

	printf("%d %s\n",create_mbox_421(5,0,0), "create_mbox_421");
	printf("%d %s\n",count_mbox_421(), "count_mbox_421");
	printf("%d %s\n",count_msg_421(5), "count_msg_421");
	printf("%d %s\n",remove_mbox_421(5), "remove_mbox_421");
    


    return (EXIT_SUCCESS);
}

