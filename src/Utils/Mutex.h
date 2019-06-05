#ifndef MUTEX_H
#define MUTEX_H

#include <mutex>
#include <atomic>

#define LOCK_GUARD(locking_mutex) std::lock_guard<std::mutex> g(locking_mutex); Q_UNUSED(g)

#endif // MUTEX_H
