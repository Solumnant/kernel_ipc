/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */



#include <linux/kernel.h>
#include <linux/cred.h>
#include <linux/uaccess.h>
#include <linux/spinlock.h>

#include <linux/errno.h>
//#include <stdlib.h>
//#include <stdbool.h>
//#include <iso646.h>
#include <linux/string.h>
#include <linux/slab.h>

//#include "string.h"


//#include "mailbox_containers.h"
#include "mailbox.h"
//#include "systemcalls.h"
//#include "helpers.h"






static LIST_HEAD(mailList);
static long numBoxes = 0;
static rwlock_t rw_list=__RW_LOCK_UNLOCKED(rw_list);
//rw_list=RW_LOCK_UNLOCKED;
//rwlock_init();

//returns 0 on success, error on failure
//places new msg into msg. use for encryption and unencryption

long encryption(unsigned char* msg, long msgLen, long key) {
    //getting the padding

    //memcpy(outmsg, msg, msgLen * sizeof (unsigned char));
    long i = 0;
    unsigned char * cKey;
    cKey = (unsigned char *) &key;
    
    for (i = 0; i < msgLen; i++) {
        msg[i] = msg[i]^cKey[i % 4];
    }


    //memcpy(msg, outmsg, msgLen);
    //free(outmsg);

    return 0;

}

//some helpers to save autocomplete time
//locks and unlocks highest tier struct

//locks the upper level list for reading

void lockListRead(void) {
    read_lock(&rw_list);
}
//locks the upper level list for writing

void lockListWrite(void) {
    write_lock(&rw_list);
}

void unlockListRead(void) {
    read_unlock_bh(&rw_list);
}

void unlockListWrite(void) {
    write_unlock_bh(&rw_list);
}

//use this to lock for reading something from a mailbox

void lockRead(mailbox * myBox) {
    read_lock(&myBox->rw_Mail);
}
//use this to lock for writing something to a mailbox

void lockWrite(mailbox * myBox) {
    write_lock(&myBox->rw_Mail);
}
//use this to unlock a mailbox after r/w

void unlockRead(mailbox * myBox) {
    read_unlock_bh(&myBox->rw_Mail);
}

void unlockWrite(mailbox * myBox) {
    write_unlock_bh(&myBox->rw_Mail);
}




//creates a new empty mailbox with ID id, if it does not already exist,
//and returns 0. The queue should be flagged for encryption if the enable_crypt
//option is set to anything other than 0. If enable_crypt is set to zero, then
//the key parameter in any functions including it should be ignored. The lifo
//parameter controls what direction the messages are retrieved in. If this
//parameter is 0, then the messages should be stored/retrieved in FIFO order
//(as a queue). If it is non-zero, then the messages should be stored
//in LIFO order (as a stack).

//needs to create object, then wlock and search. else takes time in lock to do 
//non sensitive stuff. free is cheap, right?

asmlinkage long sys_create_mbox_421(unsigned long id, int enable_crypt, int lifo) {
    mailbox * aMailbox;
    mailbox * myMailbox;
    if (current_uid().val != 0) {
        return EPERM;
    }

    //creating it now so I don't take time from the lock


    myMailbox = kmalloc(sizeof (mailbox), GFP_KERNEL);
    if (myMailbox == NULL) {
        //something went wrong...
        //no new mailbox...
        return ENOMEM;
    }

    myMailbox->numMessages = 0;
    myMailbox->encrypted = enable_crypt;
    myMailbox->id = id;
    myMailbox->islifo = lifo;
    //init the msglist
    INIT_LIST_HEAD(&myMailbox->myMsgs);

    //I think this initializes the linked for more msgs...
    INIT_LIST_HEAD(&myMailbox->list);

    myMailbox->rw_Mail = __RW_LOCK_UNLOCKED(myMailbox->rw_Mail);



    //*************lock*************

    //does the id already exist?

    //reading the list
    lockListWrite();

    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            unlockListWrite();
            //it exists, destroy what I made
            kfree(myMailbox);
            //a mailbox with said id exists already, f-off
            return EADDRINUSE;
        }
    }
    //now need to commit work

    //I think this adds the new mailbox to the existing mailbox
    list_add(&myMailbox->list, &mailList);
    numBoxes++;

    //*********************unlock*******
    unlockListWrite();
    return 0;


}

//removes mailbox with ID id, if it is empty, and returns 0. If the mailbox is
//not empty, this system call should return an appropriate error and not
//remove the mailbox.

asmlinkage long sys_remove_mbox_421(unsigned long id) {
    mailbox *aMailbox, * tempBox;
    if (current_uid().val != 0) {
        return EPERM;
    }


    //have I found the mailbox in question?
    //int found = 0;



    lockListWrite(); //access and delete

    list_for_each_entry_safe(aMailbox, tempBox, &mailList, list) {
        //is this the one?
        if (aMailbox->id == id) {
            lockWrite(aMailbox);
            //do we have more messages?            
            if (aMailbox->numMessages > 0) {
                //don't do anything: it has stuff in it
                unlockWrite(aMailbox);
                unlockListWrite();
                return EBADSLT;
            }//no more msg
            else {
                //remove the mailbox
                list_del(&aMailbox->list);
                kfree(aMailbox);
                numBoxes--;

                //don't need to unlock the mailbox
                //cause it doesn't exist anymore
                unlockListWrite();
                return 0;
            }
        }
    }

    unlockListWrite();
    //mailbox doesn't exist
    return EADDRNOTAVAIL;

}

//returns the number of existing mailboxes. easiest thing ever
//going to lock it for good practice: if it's writing it might not be accurate
//though there isn't really a promise it could ever be accurate in multithread

asmlinkage long sys_count_mbox_421(void) {
    long tempNum;
    lockListRead();
    tempNum = numBoxes;
    unlockListRead();
    return tempNum;

}

// returns a list of up to k mailbox IDs in the user-space variable mbxes.
//It returns the number of IDs written successfully to mbxes on success and an
//appropriate error code on failure.

//TODO *-*-*-*-*-*-*-*-*-*-*- malloc? pointers changing? 

asmlinkage long sys_list_mbox_421(unsigned long *mbxes, long k) {
    long copyRtrn;
    unsigned long * tmpMbxes;
    long count;
    mailbox * aMailbox;
    count = 0;


    if (mbxes == NULL) {
        //you passed a null pointer, fuck off
        return EFAULT;
    }

    tmpMbxes = kmalloc(k * sizeof (unsigned long), GFP_KERNEL);
    if (tmpMbxes == NULL) {
        return ENOMEM;
    }



    lockListRead();

    list_for_each_entry(aMailbox, &mailList, list) {
        //don't need to lock the mailboxes themselves cause they can't 
        //add or remove mailboxes

        tmpMbxes[count] = aMailbox->id;
        count++;
        //not going to think about counting this and being off by one,
        //wastes at most 3 spaces, nothing considering the buffer
        //if(count-3>=buffer){
        //  buffer*=2;
        //mbxes=realloc(mbxes, buffer* sizeof(unsigned long));
        //}
    }
    unlockListRead();

    //TODO:copy my list to user space


    copyRtrn = copy_to_user(mbxes, tmpMbxes, count);
    //some problem
    if (copyRtrn != 0) {
        kfree(tmpMbxes);
        return EFAULT;
    }





    kfree(tmpMbxes); //done with my temp var, so delete it    
    return count;
}

// encrypts the message msg (if appropriate), adding it to the already existing
//mailbox identified. Returns the number of bytes stored (which should be equal
//to the message length n) on success, and an appropriate error code on failure.
//Messages with negative lengths shall be rejected as invalid and cause an
//appropriate error to be returned.

asmlinkage long sys_send_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {
    mailbox * aMailbox;
    unsigned char* kMsg;
    msgList * myMsglist;
    long copyRtrn;


    if (n < 0) {
        return EBADMSG;
    }
    //is msg null? 
    if (msg == NULL) {
        return EFAULT;
    }


    kMsg = kmalloc(n * (sizeof (unsigned char)), GFP_KERNEL);
    if (kMsg == NULL) {
        //free(kMsg);
        return ENOMEM;
    }

    myMsglist = kmalloc(sizeof (msgList), GFP_KERNEL);
    if (myMsglist == NULL) {
        kfree(kMsg);
        return ENOMEM;
    }


    copyRtrn = copy_from_user(kMsg, msg, n);
    //some problem
    if (copyRtrn != 0) {
        kfree(kMsg);
        kfree(myMsglist);
        return EFAULT;
    }



    myMsglist->msg = kMsg;
    myMsglist->msgLen = n;
    INIT_LIST_HEAD(&myMsglist->list);




    //first find

    lockListRead();

    list_for_each_entry(aMailbox, &mailList, list) {
        //this means found
        if (aMailbox->id == id) {

            //sending message now
            lockWrite(aMailbox);
            if (aMailbox->encrypted) {
                //long returnVar;
                encryption(kMsg, n, key);
            }

            //treat as lifo
            if (aMailbox->islifo) {
                list_add(&myMsglist->list, &aMailbox->myMsgs);
            } else {//its fifo
                list_add_tail(&myMsglist->list, &aMailbox->myMsgs);
            }
            aMailbox->numMessages++;

            //found and all is right.
            unlockWrite(aMailbox);
            unlockListRead();
            return 0;
        }
    }


    //didn't find
    kfree(kMsg);
    kfree(myMsglist);
    unlockListRead();
    //if not exists throw error    
    return EADDRNOTAVAIL;

}


//copies up to n characters from the next message in the mailbox id to the
//user-space buffer msg, decrypting with the specified key (if appropriate),
//and removes the entire message from the mailbox (even if only part of the
//message is copied out). Returns the number of bytes successfully copied
//(which should be the minimum of the length of the message that is stored and
//n) on success or an appropriate error code on failure.

asmlinkage long sys_recv_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {
    long copyRtrn;
    mailbox * aMailbox;
    msgList* msgStruct;
    unsigned char * tmpMsg;
    long retLen;

    if (n < 0) {
        return EBADMSG;
    }

    //is msg null? 
    if (msg == NULL) {
        return EFAULT;
    }





    //first find

    lockListRead(); //I'm only reading at this level

    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            lockWrite(aMailbox);

            //do we have msg to get?
            if (aMailbox->numMessages == 0) {
                //no msg
                unlockWrite(aMailbox);
                unlockListRead();
                //dne
                return EEXIST;
            }


            msgStruct = list_first_entry(&aMailbox->myMsgs, msgList, list);

            tmpMsg = msgStruct->msg;
            //which is larger, request or msg?


            if (msgStruct->msgLen > n) {
                retLen = n;
            } else {
                retLen = msgStruct->msgLen;
            }



            //only do encryption up to what is needed
            encryption(tmpMsg, retLen, key);



            //TODO copy from kernel to userspace

            copyRtrn = copy_to_user(msg, tmpMsg, n);
            //some problem
            if (copyRtrn != 0) {
                unlockWrite(aMailbox);
                unlockListRead();

                return EFAULT;
            }



            //found and all is right: cleanup'
            aMailbox->numMessages--;
            list_del(&msgStruct->list);
            kfree(tmpMsg);
            kfree(msgStruct);
            unlockWrite(aMailbox);
            unlockListRead();
            //todo: return num copied
            return retLen;
        }
    }


    unlockListRead();
    //if not exists throw error    
    return EADDRNOTAVAIL;



}

//performs the same operation as recv_msg_421() without removing the message
//from the mailbox.

asmlinkage long sys_peek_msg_421(unsigned long id, unsigned char *msg, long n,
        unsigned long key) {
    long copyRtrn;
    mailbox * aMailbox;
    msgList* msgStruct;
    unsigned char * tmpMsg;
    long retLen;

    if (n < 0) {
        return EBADMSG;
    }

    //is msg null? 
    if (msg == NULL) {
        return EFAULT;
    }



    //first find
    lockListRead(); //only reading at this level

    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {

            lockWrite(aMailbox);

            //do we have msg to get?
            if (aMailbox->numMessages == 0) {
                unlockWrite(aMailbox);
                unlockListRead();
                //dne
                return EEXIST;
            }


            msgStruct = list_first_entry(&aMailbox->myMsgs, msgList, list);

            tmpMsg = msgStruct->msg;

            //which is larger, request or msg?


            if (msgStruct->msgLen > n) {
                retLen = n;
            } else {
                retLen = msgStruct->msgLen;
            }



            //only do encryption up to what is needed
            encryption(tmpMsg, retLen, key);
            //TODO copy from kernel to userspace


            copyRtrn = copy_to_user(msg, tmpMsg, n);
            //some problem
            if (copyRtrn != 0) {
                unlockWrite(aMailbox);
                unlockListRead();

                return EFAULT;
            }


            //found and all is right.
            unlockWrite(aMailbox);
            unlockListRead();
            return retLen;
        }
    }
    unlockListRead();

    //if not exists throw error    
    return EADDRNOTAVAIL;
}


//returns the number of messages in the mailbox id on success or an appropriate
//error code on failure.

asmlinkage long sys_count_msg_421(unsigned long id) {
    long numMessages;
    mailbox *aMailbox;
    lockListRead();

    list_for_each_entry(aMailbox, &mailList, list) {
        if (aMailbox->id == id) {
            lockRead(aMailbox);

            numMessages = aMailbox->numMessages;

            //found and all is right.
            unlockWrite(aMailbox);
            unlockListRead();
            return numMessages;
        }
    }
    unlockListRead();
    return EADDRNOTAVAIL;

}

//returns the lenth of the next message that would be returned by calling
//recv_msg_421() with the same id value (that is the number of bytes in the next
//message in the mailbox). If there are no messages in the mailbox, this should
//return an appropriate error value.

asmlinkage long sys_len_msg_421(unsigned long id) {

    long msgLen;

    mailbox * aMailbox;
    msgList * tempMsglist;
    //first find
    lockListRead();

    list_for_each_entry(aMailbox, &mailList, list) {
        //found mailbox
        if (aMailbox->id == id) {
            lockRead(aMailbox);
            //do we have messages?
            if (aMailbox->numMessages == 0) {
                //no, invalid, no messeges to know the size of
                unlockWrite(aMailbox);
                unlockListRead();
                return EEXIST;
            }
            //we have a message, put it's length into a return var
            tempMsglist = list_first_entry(&aMailbox->myMsgs, msgList, list);
            msgLen = tempMsglist->msgLen;

            //found and all is right.
            unlockWrite(aMailbox);
            unlockListRead();
            return msgLen;
        }
    }


    unlockListRead();
    //if not exists throw error    
    return EADDRNOTAVAIL;

}



