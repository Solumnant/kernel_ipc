/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */




#include <errno.h>
#include <stdlib.h>

//#include "mailbox_containers.h"
#include "mailbox.h"
#include "systemcalls.h"
#include "helpers.h"






static LIST_HEAD(mailList);
static long numBoxes=0;




//creates a new empty mailbox with ID id, if it does not already exist,
//and returns 0. The queue should be flagged for encryption if the enable_crypt
//option is set to anything other than 0. If enable_crypt is set to zero, then
//the key parameter in any functions including it should be ignored. The lifo
//parameter controls what direction the messages are retrieved in. If this
//parameter is 0, then the messages should be stored/retrieved in FIFO order
//(as a queue). If it is non-zero, then the messages should be stored
//in LIFO order (as a stack).

long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {
    //TODO LOCK
    mailbox * aMailbox;
    //does the id already exist?

     
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            //a mailbox with said id exists already, f-off
            return EADDRINUSE;
        }
    }

    mailbox * myMailbox;

    myMailbox = malloc(sizeof(mailbox));
    if (myMailbox == NULL) {
        //something went wrong...
        //no new mailbox...
        return ENOMEM;
    }

    myMailbox->encrypted = enable_crypt;
    myMailbox->id = id;
    myMailbox->islifo = lifo;
    //init the msglist
    INIT_LIST_HEAD(&myMailbox->myMsgs);
    
    //I think this initializes the linked list...
    INIT_LIST_HEAD(&myMailbox->list);
    //I think this adds the new mailbox to the existing mailbox
    list_add(&myMailbox->list, &mailList);
    numBoxes++;
    
    return 0;


}

//removes mailbox with ID id, if it is empty, and returns 0. If the mailbox is
//not empty, this system call should return an appropriate error and not
//remove the mailbox.

long remove_mbox_421(unsigned long id) {
    //TODO LOCK

    //have I found the mailbox in question?
    //int found = 0;
    mailbox *aMailbox, * tempBox;

    list_for_each_entry_safe(aMailbox, tempBox, &mailList, list) {
        //is this the one?
        if (aMailbox->id == id) {
            //do we have more messages?            
            if (list_empty(&aMailbox->myMsgs) ) {
                return EADDRINUSE;
            }
            //no more msg
            else {
                //remove the mailbox
                list_del(&aMailbox->list);
                free(aMailbox);
                numBoxes--;
                return 0;
            }
        }
    }

    //mailbox doesn't exist
    return EADDRNOTAVAIL;

}

//returns the number of existing mailboxes.

long count_mbox_421(void) {
    return numBoxes;

}

// returns a list of up to k mailbox IDs in the user-space variable mbxes.
//It returns the number of IDs written successfully to mbxes on success and an
//appropriate error code on failure.

//TODO *-*-*-*-*-*-*-*-*-*-*- malloc? pointers changing? 
long list_mbox_421(unsigned long *mbxes, long k) {
    if(mbxes==NULL){
        //you passed a null pointer, fuck off
        return EINVAL;
    }
    
    unsigned long tmpMbxes=malloc(k*sizeof(unsigned long));
    if(tmpMbxes==NULL){
        return ENOMEM;
    }
    
    long count=0;
    mailbox * aMailbox;
    list_for_each_entry(aMailbox, &mailList, list){
        tmpMbxes[count]=aMailbox->id;
        count++;
        //not going to think about counting this and being off by one,
        //wastes at most 3 spaces, nothing considering the buffer
        //if(count-3>=buffer){
          //  buffer*=2;
            //mbxes=realloc(mbxes, buffer* sizeof(unsigned long));
        //}
    }
    
    //TODO:copy my list to user space
    
    
    return count;
}

// encrypts the message msg (if appropriate), adding it to the already existing
//mailbox identified. Returns the number of bytes stored (which should be equal
//to the message length n) on success, and an appropriate error code on failure.
//Messages with negative lengths shall be rejected as invalid and cause an
//appropriate error to be returned.

long send_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {
    //is msg null? 
    if(msg==NULL){
        return EINVAL;
    }
    
    
    mailbox * aMailbox;
    //first find
    
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            
            //TODO: we can put the msg into a container now,
            //not going to waste memory if the mailbox doesn't exist...
            
            //treat as lifo
            if(aMailbox->islifo){
                
            }
            else{//its fifo
                
            }
            //found and all is right.
            return 0;
        }
    }
    
    
    //if not exists throw error    
    return EADDRNOTAVAIL;

}


//copies up to n characters from the next message in the mailbox id to the
//user-space buffer msg, decrypting with the specified key (if appropriate),
//and removes the entire message from the mailbox (even if only part of the
//message is copied out). Returns the number of bytes successfully copied
//(which should be the minimum of the length of the message that is stored and
//n) on success or an appropriate error code on failure.

long recv_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {
    //is msg null? 
    if(msg==NULL){
        return EINVAL;
    }
    
    
    mailbox * aMailbox;
    //first find
    
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            
            //treat as lifo
            if(aMailbox->islifo){
                
            }
            else{//its fifo
                
            }
            //found and all is right.
            return 0;
        }
    }
    
    
    //if not exists throw error    
    return EADDRNOTAVAIL;
    
    

}

//performs the same operation as recv_msg_421() without removing the message
//from the mailbox.

long peek_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {

    //is msg null? 
    if(msg==NULL){
        return EINVAL;
    }
    
    
    mailbox * aMailbox;
    //first find
    
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            
            //treat as lifo
            if(aMailbox->islifo){
                
            }
            else{//its fifo
                
            }
            //found and all is right.
            return 0;
        }
    }
    
    
    //if not exists throw error    
    return EADDRNOTAVAIL;
}


//returns the number of messages in the mailbox id on success or an appropriate
//error code on failure.

long count_msg_421(unsigned long id) {
    mailbox *aMailbox;
    
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            
            
            //found and all is right.
            return mailbox.numMessages;
        }
    }
    
    return EADDRNOTAVAIL;

}

//returns the lenth of the next message that would be returned by calling
//recv_msg_421() with the same id value (that is the number of bytes in the next
//message in the mailbox). If there are no messages in the mailbox, this should
//return an appropriate error value.

long len_msg_421(unsigned long id) {   
    
    
    mailbox * aMailbox;
    //first find
    
    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            
            //treat as lifo
            if(aMailbox->islifo){
                
            }
            else{//its fifo
                
            }
            //found and all is right.
            return 0;
        }
    }
    
    
    //if not exists throw error    
    return EADDRNOTAVAIL;

}


