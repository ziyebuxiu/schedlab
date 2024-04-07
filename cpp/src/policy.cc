#include "policy.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

map<int, Event::Task> TaskQueue;
map<int, Event::Task> TaskIO;

// kHigh,kLow各一个map
vector<map<int, Event::Task>> priorityTaskQueue(2);
vector<map<int, Event::Task>> priorityTaskIO(2);

int cur_time = -1;

// 从CPU任务队列移除任务
void removeTaskFromCPUQueue(map<int, Event::Task> *CPUQueue, int taskId)
{
  for (auto iter = CPUQueue->begin(); iter != CPUQueue->end(); iter++)
  {
    if (iter->second.taskId == taskId)
    {
      CPUQueue->erase(iter);
      break;
    }
  }
}

// 从IO任务队列移除任务
void removeTaskFromIOQueue(map<int, Event::Task> *IOQueue, int taskId)
{
  for (auto iter = IOQueue->begin(); iter != IOQueue->end(); iter++)
  {
    if (iter->second.taskId == taskId)
    {
      IOQueue->erase(iter);
      break;
    }
  }
}

int selectNextIOTask1(int current_io)
{

  if (current_io == 0)
  {
    if (!TaskIO.empty())
    {
      int flag = 0;
      Event::Task tmp;
      for (auto iter = TaskIO.begin(); iter != TaskIO.end(); iter++)
      {
        if (iter->first > cur_time) //&& iter->second.priority == Event::Task::Priority::kHigh
        {
          tmp = iter->second;
          flag = 1;
          break;
        }
      }
      if (!flag) // 全超过截止时间
        tmp = TaskIO.begin()->second;
      // tmp.taskId = 0;

      return tmp.taskId;
    }
  }
  return current_io;
}

int selectNextIOTask2(int current_io)
{

  if (current_io == 0)
  {
    if (!priorityTaskIO[0].empty())
    {
      int flag = 0;
      Event::Task tmp;
      for (auto iter = priorityTaskIO[0].begin(); iter != priorityTaskIO[0].end(); iter++)
      {
        if (iter->first > cur_time) //&& iter->second.priority == Event::Task::Priority::kHigh
        {
          tmp = iter->second;
          flag = 1;
          break;
        }
      }
      if (!flag) // 全超过截止时间
        tmp = priorityTaskIO[0].begin()->second;
      // tmp.taskId = 0;

      return tmp.taskId;
    }
    else
    {
      int flag = 0;
      Event::Task tmp;
      for (auto iter = priorityTaskIO[1].begin(); iter != priorityTaskIO[1].end(); iter++)
      {
        if (iter->first > cur_time) //&& iter->second.priority == Event::Task::Priority::kHigh
        {
          tmp = iter->second;
          flag = 1;
          break;
        }
      }
      if (!flag) // 全超过截止时间
        tmp = priorityTaskIO[1].begin()->second;
      // tmp.taskId = 0;

      return tmp.taskId;
    }
  }
  return current_io;
}

int selectNextCPUTask()
{

  int flag = 0;
  Event::Task tmp;
  for (auto iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
  {
    if (iter->first > cur_time)
    {
      flag = 1;
      tmp = iter->second;
      break;
    }
  }

  if (!flag)
    tmp = TaskQueue.begin()->second;

  return tmp.taskId;
}

Action policy(const std::vector<Event> &events, int current_cpu,
              int current_io)
{
  Action choose;
  choose.cpuTask = current_cpu;
  choose.ioTask = current_io;
  for (const auto &event : events)
  {
    int priority = (event.task.priority == Event::Task::Priority::kHigh) ? 0 : 1;
    int tId = event.task.taskId;
    switch (event.type)
    {
    case Event::Type::kTimer:
      cur_time = event.time;
      break;

    case Event::Type::kTaskArrival:
      TaskQueue[event.task.deadline] = event.task;
      priorityTaskQueue[priority][event.task.deadline] = event.task;
      break;

    case Event::Type::kTaskFinish:
      removeTaskFromCPUQueue(&TaskQueue, tId);
      removeTaskFromCPUQueue(&priorityTaskQueue[priority], tId);
      break;

    case Event::Type::kIoRequest:
      TaskIO[event.task.deadline] = event.task;
      priorityTaskIO[priority][event.task.deadline] = event.task;
      removeTaskFromCPUQueue(&TaskQueue, tId);
      removeTaskFromCPUQueue(&priorityTaskQueue[priority], tId);
      break;

    case Event::Type::kIoEnd:
      TaskQueue[event.task.deadline] = event.task;
      priorityTaskQueue[priority][event.task.deadline] = event.task;
      removeTaskFromIOQueue(&TaskIO, tId);
      removeTaskFromIOQueue(&priorityTaskIO[priority], tId);
      break;

    default:
      break;
    }
  }

  // 选择io任务
  choose.ioTask = selectNextIOTask2(current_io);

  // 选择cpu任务
  choose.cpuTask = selectNextCPUTask();

  return choose;
}
