#ifndef _MIO_COMMON_ALL_H_
#define _MIO_COMMON_ALL_H_


#include <climits>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <arpa/inet.h>
#include <errno.h>
#include <linux/types.h>
#include <malloc.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio_ext.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <condition_variable>
#include <deque>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <valarray>
#include <vector>
#include <random>

#include <boost/functional/hash.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/lockfree/stack.hpp>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <mpi.h>
#include <zmq.h>
#include <yaml-cpp/yaml.h>

#include "macros.h"
#include "failure.h"
#include "common.h"
#include "VirtualObject.h"
#include "Spinlock.h"
#include "Semaphore.h"
#include "Barrier.h"
#include "WaitGroup.h"
#include "ManagedThread.h"
#include "ThreadGroup.h"

#include "config.h"
#include "random.h"
#include "string.h"
#include "shell.h"
#include "fs.h"

#include "Archive.h"
#include "mpi.h"

#endif

