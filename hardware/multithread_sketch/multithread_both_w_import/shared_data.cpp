#include "shared_data.h"
#include <mutex>
#include <string.h>



// global instance of message data queues
messageDataQueue messageDataQueue_toLora;
messageDataQueue messageDataQueue_toBT;

// global LL
LL *ack_list = new LL;

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
Node::Node(char* id) : messageID(id), next(nullptr) {}
LL::LL() : head(nullptr) {}

// Insert new node into the ack linked list
void LL::insert(char* id) {
  Node* newNode = new Node(id);
  newNode->next = head;
  head = newNode;
}

// remove node from ack linked list given an attribute
void LL::remove(char* id) {
  Node* curr = head;
  Node* prev = nullptr;

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

// check if node exists in list
bool LL::contains(char* id) {

  Node* curr = head;

  while(curr != nullptr) {
    if (strcmp(curr->messageID, id) == 0) {return true;}
    curr = curr->next;
  }

  return false;

}
