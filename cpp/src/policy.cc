#include "policy.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;
#define FLAG true // for debug

map<int, Event::Task> TaskQueue;
map<int, Event::Task> TaskIO;

// kHigh,kLow各一个map
vector<map<int, Event::Task>> priorityTaskQueue(2);
vector<map<int, Event::Task>> priorityTaskIO(2);

int cur_time = -1;

int remainTime(Event::Task event)
{
  return event.deadline - cur_time;
}

bool cmp(pair<int, Event::Task> a, pair<int, Event::Task> b)
{
  if ((remainTime(a.second) < 0))
  {
    if (remainTime(b.second) > 0)
      return false;
    else
      return remainTime(a.second) < remainTime(b.second);
  }
  else
  {
    if (remainTime(b.second) < 0)
      return true;
    else
      return remainTime(a.second) < remainTime(b.second);
  }
}

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

int selectNextIOTask(int current_io, vector<pair<int, Event::Task>> Vec)
{
  if (FLAG)
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

  // if (current_io == 0)
  // {
  //   if (!TaskIO.empty())
  //   {
  //     int flag = 0;
  //     Event::Task tmp;
  //     for (auto iter = Vec.begin(); iter != Vec.end(); iter++)
  //     {
  //       if (iter->first > cur_time) //&& iter->second.priority == Event::Task::Priority::kHigh
  //       {
  //         tmp = iter->second;
  //         flag = 1;
  //         break;
  //       }
  //     }
  //     if (!flag) // 全超过截止时间
  //       tmp = Vec.begin()->second;
  //     // tmp.taskId = 0;

  //     return tmp.taskId;
  //   }
  // }
  // return current_io;
}

int selectNextCPUTask(vector<pair<int, Event::Task>> Vec)
{
  if (FLAG)
  {
    if (FLAG)
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

    // int flag = 0;
    // Event::Task tmp;
    // for (auto iter = Vec.begin(); iter != Vec.end(); iter++)
    // {
    //   if (iter->first > cur_time)
    //   {
    //     flag = 1;
    //     tmp = iter->second;
    //     break;
    //   }
    // }

    // if (!flag)
    //   tmp = Vec.begin()->second;

    // return tmp.taskId;
  }

  // int flag = 0;
  // Event::Task tmp;
  // for (auto iter = priorityTaskQueue[0].begin(); iter != priorityTaskQueue[0].end(); iter++)
  // {
  //   if (iter->first > cur_time)
  //   {
  //     flag = 1;
  //     tmp = iter->second;
  //     break;
  //   }
  // }

  // if (!flag)
  // {
  //   for (auto iter = priorityTaskQueue[1].begin(); iter != priorityTaskQueue[1].end(); iter++)
  //   {
  //     if (iter->first > cur_time)
  //     {
  //       flag = 1;
  //       tmp = iter->second;
  //       break;
  //     }
  //   }
  // }
  // if (!flag)
  //   tmp = TaskQueue.begin()->second;

  // return tmp.taskId;
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

  vector<pair<int, Event::Task>> CPUVec(TaskQueue.begin(), TaskQueue.end());
  vector<pair<int, Event::Task>> IOVec(TaskIO.begin(), TaskIO.end());

  sort(CPUVec.begin(), CPUVec.end(), cmp);
  sort(IOVec.begin(), IOVec.end(), cmp);
  // 选择io任务
  choose.ioTask = selectNextIOTask(current_io, IOVec);

  // 选择cpu任务
  choose.cpuTask = selectNextCPUTask(CPUVec);

  return choose;
}
