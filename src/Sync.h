#ifndef SYNC_H
#define SYNC_H

#include <mutex>
#include <queue>
#include <string>

extern std::queue<std::string> entryQueue;
extern bool isDisplayingEntry;
extern std::mutex entryMutex;

void ProcessQueue();

#endif  // SYNC_H
