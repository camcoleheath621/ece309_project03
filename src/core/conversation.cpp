#include "core/conversation.h"

// default constructor (empty brackets)
Conversation::Conversation() {
    data_ = nullptr;                // initialize data_ to nullptr
    size_ = 0;                      // initialize size_ to 0 
    capacity_ = 0;                  // initialize capacity_ to 0
}

// default destructor
Conversation::~Conversation() {
    delete[] data_;                 // free data_
}

// copy constructor
Conversation::Conversation(const Conversation& other) {
    size_ = other.size_;            // copy the number of stored messages
    capacity_ = other.capacity_;    // copy the allocated capacity

    // DEEP COPY: allocate seperate storage for non-empty conversation
    if (other.capacity_ != 0) {                     
        data_ = new Message[capacity_];                     // create a new Message array

        for (std::size_t i = 0; i < size_; i++) {     // copy all messages into conversation
            data_[i] = other.data_[i];                             
        }
    }

    else {                  // empty case, no storage to copy
        data_ = nullptr;    // make pointer nullptr
    }

}
// copy assignment operator
 Conversation& Conversation::operator=(const Conversation& other) {

    if (this != &other) {               // if we are not copying the same array
        
        delete[] data_;                 // delete old storage
        size_ = other.size_;            // copy the number of stored messages
        capacity_ = other.capacity_;    // copy the allocated capacity

        if (other.capacity_ != 0) {                     

            data_ = new Message[capacity_];

            for (std::size_t i = 0; i < size_; i++) {     // copy all messages into conversation
            data_[i] = other.data_[i];                             
            }
        }

        else {                  // empty case, no storage to copy
            data_ = nullptr;    // make pointer nullptr
        }

    } 
    return *this;
 }

 // move constructor
 Conversation::Conversation(Conversation&& other) noexcept {
    
    data_ = other.data_;
    size_ = other.size_;
    capacity_ = other.capacity_;

    other.data_ = nullptr;
    other.size_ = 0;
    other.capacity_ = 0;
 }

 // move assignment operator
 Conversation& Conversation::operator=(Conversation&& other) noexcept {

    if (this != &other) {           // make sure we're not moving into the same object
    
        delete[] data_;             // delete current storage

    data_ = other.data_;            // take other's data pointer
    size_ = other.size_;            // copy other's current size
    capacity_ = other.capacity_;    // copy other's current capacity

    other.data_ = nullptr;          // empty the previous object
    other.size_ = 0;                
    other.capacity_ = 0;            
    }
    
    return *this;                   // return the current conversation
 }

 // append a new message
 void Conversation::append(Message m) {
    
    if (capacity_ == 0) {                       // if conversation has no allocated storage...
        capacity_ = 1;                          // create space for the first message
        data_ = new Message[capacity_];         // allocate the initial message array
    }

    if ( capacity_ > 0 && size_ == capacity_) {         // if the current array is full...
        
        capacity_ = capacity_ * 2;                      // double its capacity

        Message* new_data = new Message[capacity_];     // allocate an array for the new capacity
        
        for (std:: size_t i = 0; i < size_; i++) {      // copy existing message into new storage
            new_data[i] = data_[i];
        }

        delete [] data_;                                // free the old message array
        data_ = new_data;                               // transfer ownership

    }

    data_[size_] = m;       // append the new message
    size_++;                // increase the message size by 1
    
 }

 // return number of messages per conversation
 std::size_t Conversation::size() const noexcept { 
    return size_;
 }

 // return the message at the specified index
 const Message& Conversation::at(std::size_t i) const {
    
    static const Message invalid_message;       // safe default in case of an invalid indedx
    
    if ( i >= size_) {             // if index is invalid
        return invalid_message;    // return blank message
    }
    
    return data_[i]; // return the valid index
 }

 // return a pointer to the first message
   const Message* Conversation::begin() const noexcept {
    return data_;
   }
    
   const Message* Conversation::end() const noexcept {
    
    if ( size_ == 0) {      // check for an empty conversation where begin() == end()
        return data_;
    }

    return data_ + size_;   // point one past the last stored message
    }
