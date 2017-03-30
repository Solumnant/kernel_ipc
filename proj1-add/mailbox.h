/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * File:   mailbox.h
 * Author: Ben Kittner
 *
 * Created on March 20, 2017, 9:06 PM
 */

#ifndef MAILBOXE_H
#define MAILBOXE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"
    //#include <linux/list.h>
#include <pthread.h>

    typedef struct msgList {
        //from https://isis.poly.edu/kulesh/stuff/src/klist/
        //makes message into a list
        struct list_head list;
        char* msg;
        long msgLen;
        pthread_mutex_t mutexMail;
    }msgList;
    
    

    typedef struct mailbox {
        //makes mailboxes into a list
        struct list_head list;

        //id for mailbox
        unsigned long id;
        //is it encrypted?
        int encrypted;
        //the message

        //is it fifo or not?
        int islifo;

        //contains the list of messages
        struct list_head myMsgs;
        
        unsigned long numMessages;


        pthread_mutex_t mutexMail;

    }mailbox;
    






#ifdef __cplusplus
}
#endif

#endif /* MAILBOXE_H */

