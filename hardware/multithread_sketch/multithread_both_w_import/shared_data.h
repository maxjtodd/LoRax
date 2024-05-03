#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <queue>
#include <ctime>
#include <mutex>

// messageData - functions as the data packet
struct messageData {
    int message_type; 
        /* types:
            new message : 0x01,
            contact ping: 0x02,
            ack of mess.: 0x03
        */
    char* message_id;
        /* to ensure unqiueness across devices, a messageID will contain two components
            1. a message counter, incremeneted everytime a message is sent
            2. the mac address of the sending device
            ex. 'MESSAGE_COUNT'_'ESP_MAC' (int) 
        */
    char* message_dest;
        /*
        When sending message to specific device, we use the MAC addr transfered during 
        a contact ping
        */
    int size;
    char* value;
  //  time_t messageSentTime; 
};

// Declare queue of messageData's
struct messageDataQueue {
    std::queue<messageData> queue;
    std::mutex mtx; 
};

// global instance of message data
extern messageDataQueue messageDataQueue_toLora;
extern messageDataQueue messageDataQueue_toBT;

// QUEUE OPS

// Push / Pop
messageData getMessageData(messageDataQueue& mdq);
void pushMessageData(messageDataQueue& mdq,  messageData& data);

// LINKED LIST

struct Node {
    char* messageID;
    messageData message;
    Node* next;

    Node(char* id, messageData mess);
};

class LL {
    public: 
        Node* head;
        LL();
        void insert(char* id, messageData mess);
        void remove (char* id);
        bool contains(char* id);
        void traverse() const;
};

#endif
