#include <mio/all.h>
#include <gtest/gtest.h> 

namespace {

TEST (ThreadGroup, parallel_run) { 
  mio::ThreadGroup& thrgrp = mio::local_thread_group();
  thrgrp.set_parallel_num(4);
  std::vector<int> vec(4);

  mio::parallel_run([&](int i){
    vec[i] = i*2;
  }, thrgrp);

  EXPECT_EQ(0, vec[0]);
  EXPECT_EQ(2, vec[1]);
  EXPECT_EQ(4, vec[2]);  
  EXPECT_EQ(6, vec[3]);  
}

TEST (ThreadGroup, parentgroup) {
  mio::ThreadGroup& thrgrp = mio::local_thread_group();
  thrgrp.set_parallel_num(4);

  mio::parallel_run([&](int i){
    EXPECT_EQ(&thrgrp, mio::ThreadGroup::parent_group());
  }, thrgrp);

  EXPECT_EQ(NULL, mio::ThreadGroup::parent_group());
}

TEST (ThreadGroup, barrier) {
  mio::ThreadGroup thrgrp = mio::ThreadGroup(4);
  std::vector<int> vec(4);

  mio::parallel_run([&](int i){
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
    vec[i] = i*2;
    if (mio::ThreadGroup::thread_id() == 1) {
      EXPECT_EQ(0, vec[0]); 
      EXPECT_EQ(2, vec[1]); 
      EXPECT_EQ(0, vec[2]); 
      EXPECT_EQ(0, vec[3]); 
    }
  }, thrgrp); 

  for (int i = 0; i < 4; i++) vec[i] = 0;

  mio::parallel_run([&](int i){
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * i));
    vec[i] = i*2;
    mio::parallel_run_barrier_wait();
    if (mio::ThreadGroup::thread_id() == 1) {
      EXPECT_EQ(0, vec[0]);
      EXPECT_EQ(2, vec[1]);
      EXPECT_EQ(4, vec[2]);
      EXPECT_EQ(6, vec[3]);
    }
  }, thrgrp);

}

TEST (ThreadGroup, parallel_run_range) {
  mio::ThreadGroup thrgrp = mio::ThreadGroup(8);
  int vecsz = 107;
  std::vector<int> vec(vecsz);
 
  mio::parallel_run_range(vecsz,
    [&](int i, int begin, int end){
    for (int k = begin; k < end; k++) {
      vec[k] = k*2;
    }
  }, thrgrp);

  for (size_t i = 0; i < vec.size(); i++) {
    EXPECT_EQ(i*2, vec[i]);
  }
}

TEST (ThreadGroup, parallel_run_dynamic) {
  mio::ThreadGroup thrgrp = mio::ThreadGroup(8);
  int vecsz = 107;
  std::vector<int> vec(vecsz);

  mio::parallel_run_dynamic(vecsz,
    [&](int tid, int idx){
      vec[idx] = idx*2;
  }, thrgrp);

  for (size_t i = 0; i < vec.size(); i++) {
    EXPECT_EQ(i*2, vec[i]);
  }
}

}

