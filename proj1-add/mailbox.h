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
    
#include <linux/list.h>    
    
    typedef struct linkedMsg{
        //from https://isis.poly.edu/kulesh/stuff/src/klist/
        struct list_head list;
        char* msg;
    };

    

typedef struct mailbox{
    //id for mailbox
    int id;
    //is it encrypted?
    int encrypted;
    //the message
    
    //is it fifo or not?
    int islifo;
    
    //contains the list of messages
    linkedMsg myMsgs;  
        
};

mailbox test;




#ifdef __cplusplus
}
#endif

#endif /* MAILBOXE_H */

