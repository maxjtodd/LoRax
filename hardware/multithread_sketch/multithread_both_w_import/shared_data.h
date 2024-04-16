#ifndef SHARED_DATA_H
#define SHARED_DATA_H

#include <queue>
#include <ctime>
#include <mutex>

// Define a data structure
struct messageData {
    int id;
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


#endif
