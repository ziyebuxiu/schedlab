#include "policy.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

map<int, Event::Task> TaskQueue;
map<int, Event::Task> TaskIO;

map<int, Event> taskId2Event;

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
// 基础版
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

// 优化1
int selectNextIOTask2(int current_io)
{
  if (current_io == 0)
  {
    if (!TaskIO.empty())
    {
      int flag = 0;
      Event::Task tmp;
      for (auto iter = TaskIO.begin(); iter != TaskIO.end(); iter++)
      {
        if (iter->first > cur_time && iter->second.priority == Event::Task::Priority::kHigh) //
        {
          tmp = iter->second;
          flag = 1;
          break;
        }
      }
      if (!flag) // 全超过截止时间
      {
        tmp = TaskIO.begin()->second;
      }

      return tmp.taskId;
    }
  }
  return current_io;
}
// 优化2
int selectNextIOTask3(int current_io)
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
          if (!priorityTaskIO[0].empty() && iter->second.priority == Event::Task::Priority::kHigh)
          {
            tmp = iter->second;
            flag = 1;
            break;
          }
          else if (iter->second.priority == Event::Task::Priority::kLow)
            continue;
        }
      }
      if (!flag) // 全超过截止时间
      {
        if (!priorityTaskIO[0].empty())
          tmp = priorityTaskIO[0].begin()->second;
        else
          tmp = TaskIO.begin()->second;
      }

      return tmp.taskId;
    }
  }
  return current_io;
}

int selectNextCPUTask1()
{
  int flag = 0;
  Event::Task tmp;
  for (auto iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
  {
    //ddl还没到
    if (iter->first > cur_time)
    {
      flag = 1;
      tmp = iter->second;
      break;
    }
  }
  //全部超时
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
  for(const auto &event:events){
    taskId2Event[event.task.taskId] = event;
  }
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
  choose.ioTask = selectNextIOTask3(current_io);

  // 选择cpu任务
  choose.cpuTask = selectNextCPUTask1();

  return choose;
}
