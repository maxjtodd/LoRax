#include "shared_data.h"
#include <mutex>



// global instance of message data queues
messageDataQueue messageDataQueue_toLora;
messageDataQueue messageDataQueue_toBT;

// global ack_LL
ack_LL *ack_list = new ack_LL;

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

// Initialization functions
ack_Node::ack_Node(char* id) : messageID(id), next(nullptr) {}
ack_LL::ack_LL() : head(nullptr) {}

// Insert new node into the ack linked list
void ack_LL::insert(char* id) {
  ack_Node* newNode = new ack_Node(id);
  newNode->next = head;
  head = newNode;
}

// remove node from ack linked list given an attribute
void ack_LL::remove(char* id) {
  ack_Node* curr = head;
  ack_Node* prev = nullptr;

  // traverse list, find node to remove
  while (curr != nullptr && curr->messageID != id) {
      prev = curr;
      curr = curr->next;
  }

  // if node is found, remove
  if (curr != nullptr) {
      if (prev == nullptr) {
          // if the node to remove is head
          head = curr->next;
      } else {
          // if node is not head
          prev->next = curr->next;
      }

      delete curr;
  }
}