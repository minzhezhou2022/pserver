#include <mio/all.h>
#include <gtest/gtest.h> 

namespace {
TEST (Spinlock, unlockable) { 
  mio::Spinlock splock;
  
  splock.lock();
  splock.unlock();

}
}



