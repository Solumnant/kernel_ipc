/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */




#include <linux/cred.h>
    
#include <uapi/asm-generic/errno.h>
//#include <stdlib.h>
//#include <stdbool.h>
//#include <iso646.h>
#include <linux/string.h>    
    
//#include "string.h"


    //#include "mailbox_containers.h"
#include "mailbox.h"
//#include "systemcalls.h"
//#include "helpers.h"






    static LIST_HEAD(mailList);
    static long numBoxes = 0;
    static rwlock_t rw_list = rwlock_init(&rw_list);

    //returns 0 on success, error on failure
    //places new msg into msg. use for encryption and unencryption

    long encryption(unsigned char* msg, long msgLen, long key) {
        //getting the padding
        int padding;
        padding = msgLen % 4;
        padding = 4 - padding;
        char* outmsg = malloc((msgLen) * sizeof (unsigned char));
        if (outmsg == NULL) {
            return ENOMEM;
        }
        memcpy(outmsg, msg, msgLen * sizeof (unsigned char));

        unsigned char * cKey = (unsigned char *) &key;
        long i = 0;
        for (i = 0; i < msgLen; i++) {
            outmsg[i] = outmsg[i]^cKey[i % 4];
        }

        outmsg = realloc(outmsg, msgLen * sizeof (unsigned char));

        if (outmsg == NULL) {
            //wtf
            return ENOMEM;
        }
        memcpy(msg, outmsg, msgLen);
        free(outmsg);

        return 0;

    }

    //some helpers to save autocomplete time
    //locks and unlocks highest tier struct

    //locks the upper level list for reading

    void lockListRead() {
        read_lock(&rw_list);
    }
    //locks the upper level list for writing

    void lockListWrite() {
        write_lock(&rw_list);
    }
    

    void unlockListRead() {
        read_unlock_bh(&rw_list);
    }
    void unlockListWrite(){
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

    long create_mbox_421(unsigned long id, int enable_crypt, int lifo) {
        if (current_uid() != 0) {
            return EPERM;
        }

        //creating it now so I don't take time from the lock
        mailbox * myMailbox;

        myMailbox = malloc(sizeof (mailbox));
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

        myMailbox->rw_Mail = rwlock_init(&myMailbox->rw_Mail);



        //*************lock*************
        mailbox * aMailbox;
        //does the id already exist?

        //reading the list
        lockListWrite();

        list_for_each_entry(aMailbox, &mailList, list) {
            if (aMailbox->id == id) {
                unlockListWrite();
                //it exists, destroy what I made
                free(myMailbox);
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

    long remove_mbox_421(unsigned long id) {
        if (current_uid() != 0) {
            return EPERM;
        }


        //have I found the mailbox in question?
        //int found = 0;
        mailbox *aMailbox, * tempBox;


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
                    free(aMailbox);
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

    long count_mbox_421(void) {
        lockListRead();
        long tempNum = numBoxes;
        unlockListRead();
        return tempNum;

    }

    // returns a list of up to k mailbox IDs in the user-space variable mbxes.
    //It returns the number of IDs written successfully to mbxes on success and an
    //appropriate error code on failure.

    //TODO *-*-*-*-*-*-*-*-*-*-*- malloc? pointers changing? 

    long list_mbox_421(unsigned long *mbxes, long k) {
        if (mbxes == NULL) {
            //you passed a null pointer, fuck off
            return EFAULT;
        }

        unsigned long * tmpMbxes = malloc(k * sizeof (unsigned long));
        if (tmpMbxes == NULL) {
            return ENOMEM;
        }

        long count = 0;
        mailbox * aMailbox;
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

        long copyRtrn = copy_to_user(mbxes, tmpMbxes, count);
        //some problem
        if (copyRtrn != 0) {
            free(tmpMbxes);
            return EFAULT;
        }





        free(tmpMbxes); //done with my temp var, so delete it    
        return count;
    }

    // encrypts the message msg (if appropriate), adding it to the already existing
    //mailbox identified. Returns the number of bytes stored (which should be equal
    //to the message length n) on success, and an appropriate error code on failure.
    //Messages with negative lengths shall be rejected as invalid and cause an
    //appropriate error to be returned.

    //messages of length 0 are also invalid in my implementation

    long send_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key) {



        unsigned char* kMsg;

        


        kMsg = malloc(n * (sizeof (unsigned char)));
        if (kMsg == NULL) {
            //free(kMsg);
            return ENOMEM;
        }
        msgList * myMsglist = malloc(sizeof (msgList));
        if (myMsglist == NULL) {
            free(kMsg);
            return ENOMEM;
        }

        long copyRtrn = copy_from_user(kMsg, msg, n);
        //some problem
        if (copyRtrn != 0) {
            free(kMsg);
            free(myMsglist);
            return EFAULT;
        }



        myMsglist->msg = kMsg;
        myMsglist->msgLen = n;
        INIT_LIST_HEAD(&myMsglist->list);


        if (n < 0) {
            return EBADMSG;
        }
        //is msg null? 
        if (msg == NULL) {
            return EFAULT;
        }

        mailbox * aMailbox;
        //first find

        lockListRead();

        list_for_each_entry(aMailbox, &mailList, list) {
            //this means found
            if (aMailbox->id == id) {

                //sending message now
                lockWrite(aMailbox);
                if (aMailbox->encrypted) {
                    //long returnVar;
                    if (ENOMEM==encryption(kMsg, n, key)) {
                        //ran out of memory to encrypt
                        return ENOMEM;
                    }

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
        free(kMsg);
        free(myMsglist);
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

    long recv_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key) {
        if (n < 0) {
            return EBADMSG;
        }

        //is msg null? 
        if (msg == NULL) {
            return EFAULT;
        }

        long retLen;


        mailbox * aMailbox;
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

                msgList* msgStruct = list_first_entry(&aMailbox->myMsgs, msgList, list);
                unsigned char * tmpMsg = msgStruct->msg;
                //which is larger, request or msg?


                if (msgStruct->msgLen > n) {
                    retLen = n;
                } else {
                    retLen = msgStruct->msgLen;
                }



                //only do encryption up to what is needed
                encryption(tmpMsg, retLen, key);



                //TODO copy from kernel to userspace
                long copyRtrn = copy_to_user(msg, tmpMsg, n);
                //some problem
                if (copyRtrn != 0) {
                    unlockWrite(aMailbox);
                    unlockListRead();

                    return EFAULT;
                }



                //found and all is right: cleanup'
                aMailbox->numMessages--;
                list_del(&msgStruct->list);
                free(tmpMsg);
                free(msgStruct);
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

    long peek_msg_421(unsigned long id, unsigned char *msg, long n,
            unsigned long key) {

        if (n < 0) {
            return EBADMSG;
        }

        //is msg null? 
        if (msg == NULL) {
            return EFAULT;
        }

        long retLen;


        mailbox * aMailbox;
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

                msgList* msgStruct = list_first_entry(&aMailbox->myMsgs, msgList, list);
                unsigned char * tmpMsg = msgStruct->msg;

                //which is larger, request or msg?


                if (msgStruct->msgLen > n) {
                    retLen = n;
                } else {
                    retLen = msgStruct->msgLen;
                }



                //only do encryption up to what is needed
                encryption(tmpMsg, retLen, key);
                //TODO copy from kernel to userspace

                long copyRtrn = copy_to_user(msg, tmpMsg, n);
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

    long count_msg_421(unsigned long id) {
        mailbox *aMailbox;
        lockListRead();

        list_for_each_entry(aMailbox, &mailList, list) {
            if (aMailbox->id == id) {
                lockRead(aMailbox);
                long numMessages = aMailbox->numMessages;

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

    long len_msg_421(unsigned long id) {

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



