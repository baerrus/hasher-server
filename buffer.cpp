//
// Copyright (c) 2024 Vlad Troyanker
//

#include <cassert>
#include <iostream>
#include <utility>


#include "buffer.h"


Buffer make_buffer(unsigned size) {

  assert(size > 0);
  auto  trackingDeleter = [](std::vector<char>* p) 
    { // debug use only
        std::cout << "Call delete for object size =" << p->size() << '\n';
        delete p;
    };
  return std::shared_ptr<std::vector<char>>(new std::vector<char>(size));
}
