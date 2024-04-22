#include "shared_data.h"
#include <mutex>


// global instance of message data queues
messageDataQueue messageDataQueue_toLora;
messageDataQueue messageDataQueue_toBT;

messageData getMessageData(messageDataQueue& mdq) {
  /*
    Retreive first messageData from queue
    Pop it from queue
    Returnt the retrieved messageData
  */
  std::lock_guard<std::mutex> lock(mdq.mtx); // lock mutex

  messageData data = mdq.queue.front();

  mdq.queue.pop(); // remove message from queue

  return data;
}

void pushMessageData(messageDataQueue& mdq, messageData& dataToInsert) {

  std::lock_guard<std::mutex> lock(mdq.mtx); // lock mutex

  mdq.queue.push(dataToInsert);

}