#include "policy.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

map<int, Event::Task> TaskQueue;
map<int, Event::Task> TaskIO;

int cur_time = -1;

Action policy(const std::vector<Event> &events, int current_cpu,
              int current_io)
{

  Action choose;
  choose.cpuTask = current_cpu;
  choose.ioTask = current_io;
  for (const auto &event : events)
  {
    switch (event.type)
    {
    case Event::Type::kTimer:
      cur_time = event.time;
      break;

    case Event::Type::kTaskArrival:
      TaskQueue.insert(map<int, Event::Task>::value_type(event.task.deadline, event.task));
      break;

    case Event::Type::kTaskFinish:
      for (auto iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
      {
        if (iter->second.taskId == event.task.taskId)
        {
          TaskQueue.erase(iter);
          break;
        }
      }
      break;

    case Event::Type::kIoRequest:
      TaskIO.insert(map<int, Event::Task>::value_type(event.task.deadline, event.task));
      for (auto iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
      {
        if (iter->second.taskId == event.task.taskId)
        {
          TaskQueue.erase(iter);
          break;
        }
      }
      break;

    case Event::Type::kIoEnd:
      TaskQueue.insert(map<int, Event::Task>::value_type(event.task.deadline, event.task));
      for (auto iter = TaskIO.begin(); iter != TaskIO.end(); iter++)
      {
        if (iter->second.taskId == event.task.taskId)
        {
          TaskIO.erase(iter);
          break;
        }
      }
      break;

    default:
      break;
    }
  }

//选择io任务
  if (current_io == 0)
  {
    if (!TaskIO.empty())
    {
      int flag = 0;
      Event::Task tmp;
      for (auto iter = TaskIO.begin(); iter != TaskIO.end(); iter++)
      {
        if (iter->first > cur_time)
        {
          tmp = iter->second;
          flag = 1;
          break;
        }
      }
      if (!flag) // 全超过截止时间
        tmp = TaskIO.begin()->second;
      
      choose.ioTask = tmp.taskId;

    }
  }
//选择cpu任务
  int flag = 0;
  Event::Task tmp;
  for(auto iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++){
    if(iter->first > cur_time){
      flag = 1;
      tmp = iter->second;
      break;
    }
  }
  
  if (!flag)
    tmp = TaskQueue.begin()->second;

  choose.cpuTask = tmp.taskId;

  return choose;
}
