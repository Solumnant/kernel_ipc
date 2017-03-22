/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   systemcalls.h
 * Author: Ben Kittner
 *
 * Created on March 20, 2017, 9:37 PM
 */

#ifndef SYSTEMCALLS_H
#define SYSTEMCALLS_H

#ifdef __cplusplus
extern "C" {
#endif

 long create_mbox_421(unsigned long id, int enable_crypt, int lifo);
 

 long remove_mbox_421(unsigned long id);
 
 long count_mbox_421(void);

 long list_mbox_421(unsigned long *mbxes, long k);
 
 long send_msg_421(unsigned long id, unsigned char *msg, long n,
         unsigned long key);
 
 long recv_msg_421(unsigned long id, unsigned char *msg, long n, 
        unsigned long key);
 
 long peek_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key);
 
 long count_msg_421(unsigned long id);
 
 long len_msg_421(unsigned long id);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEMCALLS_H */

