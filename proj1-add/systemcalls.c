/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */


//creates a new empty mailbox with ID id, if it does not already exist,
//and returns 0. The queue should be flagged for encryption if the enable_crypt
//option is set to anything other than 0. If enable_crypt is set to zero, then
//the key parameter in any functions including it should be ignored. The lifo
//parameter controls what direction the messages are retrieved in. If this
//parameter is 0, then the messages should be stored/retrieved in FIFO order
//(as a queue). If it is non-zero, then the messages should be stored
//in LIFO order (as a stack).

#include <errno.h>

#include "systemcalls.h"
#include "helpers.h"




long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {
    mailbox * myMailbox;
    myMailbox->encrypted=enable_crypt;
    myMailbox->id=id;
    myMailbox->islifo=lifo;
    myMailbox->myMsgs
    
}

//removes mailbox with ID id, if it is empty, and returns 0. If the mailbox is 
//not empty, this system call should return an appropriate error and not 
//remove the mailbox.
long remove_mbox_421(unsigned long id) {
    
    //does the mailbox exist
    //TODO
    if(0){
        
    }
    
    //do we have more messages?
    if(count_msg_421(id)!=0){
        //pretty close to whats going on...
        return EADDRINUSE;
    }
    else{
        //remove the mailbox
        
        return 0;
    }
    
    

}

//returns the number of existing mailboxes.
long count_mbox_421(void){
    
}

// returns a list of up to k mailbox IDs in the user-space variable mbxes.
//It returns the number of IDs written successfully to mbxes on success and an 
//appropriate error code on failure.
long list_mbox_421(unsigned long *mbxes, long k){
    
}

// encrypts the message msg (if appropriate), adding it to the already existing
//mailbox identified. Returns the number of bytes stored (which should be equal 
//to the message length n) on success, and an appropriate error code on failure.
//Messages with negative lengths shall be rejected as invalid and cause an 
//appropriate error to be returned.
long send_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key){
    
}


//copies up to n characters from the next message in the mailbox id to the
//user-space buffer msg, decrypting with the specified key (if appropriate),
//and removes the entire message from the mailbox (even if only part of the
//message is copied out). Returns the number of bytes successfully copied 
//(which should be the minimum of the length of the message that is stored and
//n) on success or an appropriate error code on failure.
long recv_msg_421(unsigned long id, unsigned char *msg, long n, 
        unsigned long key){
    
}

//performs the same operation as recv_msg_421() without removing the message
//from the mailbox.
long peek_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key){
    
}


//returns the number of messages in the mailbox id on success or an appropriate
//error code on failure.
long count_msg_421(unsigned long id){
    
}

//returns the lenth of the next message that would be returned by calling
//recv_msg_421() with the same id value (that is the number of bytes in the next
//message in the mailbox). If there are no messages in the mailbox, this should 
//return an appropriate error value.
long len_msg_421(unsigned long id){
    
}