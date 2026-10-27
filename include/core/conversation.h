// from p2_spec_v3
#pragma once

#include <cstddef>
#include "core/message.h"

class Conversation {
public:
    Conversation(); // default constructor for an empty conversation
    ~Conversation(); // destructor to clean up dynamically allocated memory

    Conversation(const Conversation& other);                // copy constructor
    Conversation& operator=(const Conversation& other);     // copy assignment operator

    Conversation(Conversation&& other) noexcept;            // move constructor
    Conversation& operator=(Conversation&& other) noexcept; // move assignment operator

    void append(Message m);                 // append a new message to the conversation

    std::size_t size() const noexcept;      // return the number of messages in the conversation

    const Message& at(std::size_t i) const; // return the message at the specified index

    const Message* begin() const noexcept;  // return a pointer to the first message
    const Message* end() const noexcept;    // return a pointer to the last message

private:
    Message* data_ = nullptr;   // pointer to dynamically allocated array of messages
    std::size_t size_ = 0;      // current number of messages in the conversation
    std::size_t capacity_ = 0;  // current capacity of the dynamically allocated array
};