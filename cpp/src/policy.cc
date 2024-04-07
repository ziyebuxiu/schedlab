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
// // 优先级队列，每个优先级一个队列
// vector<vector<Event::Task>> priorityQueues;

// // IO等待队列
// vector<Event::Task> ioWaitQueue;

// // 记录任务和其当前优先级的映射
// unordered_map<int, int> taskPriorityMap;
// unordered_map<int, Event::Task> taskId2Event;

// // cpu任务队列
// map<int, Event::Task> TaskQueue;
// // io任务队列
// map<int, Event::Task> TaskIO;

// // 当前时间
// int cur_time = -1;

// int remainTime(Event::Task eve)
// { // 任务剩余时间
//   return eve.deadline - cur_time;
// }

// struct Comparator
// {
//   bool operator()(Event::Task a, Event::Task b)
//   { // 比较任务优先级
//     if (a.priority == Event::Task::Priority::kHigh)
//     {
//       if (b.priority == Event::Task::Priority::kLow)
//         return remainTime(a) * 1.1 <= remainTime(b) * 0.9;
//       else
//         return remainTime(a) <= remainTime(b);
//     }
//     if (b.priority == Event::Task::Priority::kHigh)
//     {
//       if (a.priority == Event::Task::Priority::kLow)
//         return remainTime(a) * 0.9 <= remainTime(b) * 1.1;
//       else
//         return remainTime(a) < remainTime(b);
//     }
//     return remainTime(a) < remainTime(b);
//   }
// };

// struct Cmp{
//   bool operator()(Event::Task a, Event::Task b){
//     return remainTime(a) < remainTime(b);
//   }
// };

// void removeTaskFromCPUQueue(int taskId)
// {
//   for (auto &queue : priorityQueues)
//   { // 遍历每个优先级队列
//     auto it = remove_if(queue.begin(), queue.end(),
//                              [taskId](const Event::Task &task)
//                              { return task.taskId == taskId; });
//     if (it != queue.end())
//     {                               // 如果找到了任务
//       queue.erase(it, queue.end()); // 从队列中移除任务
//       break;                        // 任务移除后退出循环
//     }
//   }
// }
// void removeTaskFromIOQueue(int taskId)
// {
//   auto it = remove_if(ioWaitQueue.begin(), ioWaitQueue.end(),
//                            [taskId](const Event::Task &task)
//                            { return task.taskId == taskId; });
//   if (it != ioWaitQueue.end())
//   {                                           // 如果在IO等待队列中找到任务
//     ioWaitQueue.erase(it, ioWaitQueue.end()); // 从队列中移除任务
//   }
// }
// // 记录当前正在进行IO的任务ID
// // int currentIOTaskId = -1;
// // 选择下一个CPU任务
// int selectNextCPUTask(int currentIOTaskId)
// {
//   for (auto &queue : priorityQueues)
//   {
//     sort(queue.begin(), queue.end(), Cmp());
//     for (int i = 0; i < queue.size(); i++)
//     {
//       if (queue[i].taskId != currentIOTaskId)
//         return queue[i].taskId;
//     }
//   }
//   return -1; // 如果没有任务，返回-1
// }

// // 选择下一个IO任务
// int selectNextIOTask(int currentIOTaskId)
// {
//   if (!ioWaitQueue.empty())
//   {
//     sort(ioWaitQueue.begin(), ioWaitQueue.end(), Cmp());
//     for (int i = 0; i < ioWaitQueue.size(); i++)
//     {
//       if (ioWaitQueue[i].taskId == currentIOTaskId)
//         return currentIOTaskId;
//       else
//         return ioWaitQueue[i].taskId;
//     }
//   }

//   return -1; // 如果没有IO任务，返回-1
// }

// // 任务完成或时间片耗尽时降低任务的优先级
// void demoteTask(int taskId, int currentIOTaskId)
// {
//   if (taskId == currentIOTaskId)
//     return;
//   int priority = taskPriorityMap[taskId];
//   // queue<Event::Task> emptyQueue;
//   // priorityQueues.push_back(emptyQueue);

//   // 从当前优先级队列中移除任务
//   vector<Event::Task> &currentQueue = priorityQueues[priority];
//   vector<Event::Task> tempQueue;
//   for (auto it = currentQueue.begin(); it != currentQueue.end(); ++it)
//   {
//     if (it->taskId != taskId)
//     {
//       tempQueue.push_back(*it);
//     }
//   }
//   // 除去指定任务后，恢复其他任务到原队列
//   currentQueue.swap(tempQueue);

//   // 将任务移动到下一个优先级队列
//   priority++;
//   if (priority >= priorityQueues.size())
//     priorityQueues.resize(priority + 1);
//   priorityQueues[priority].push_back(taskId2Event[taskId]);

//   // 更新任务的优先级信息
//   taskPriorityMap[taskId] = priority;
//   for (int i = 0; i < priorityQueues.size(); i++)
//   {
//     sort(priorityQueues[i].begin(), priorityQueues[i].end(), Cmp());
//   }
// }

// Action policy(const std::vector<Event> &events, int current_cpu,
//               int current_io)
// {
//   Action action;
//   action.cpuTask = current_cpu;
//   action.ioTask = current_io;

//   for (const auto &event : events)
//   {
//     switch (event.type)
//     {
//     case Event::Type::kTimer:
//       // 时间片耗尽，降低当前CPU任务的优先级

//       cur_time = event.time;
//       break;
//     case Event::Type::kTaskArrival:
//       // 新任务到达，放入最高优先级队列
//       TaskQueue[event.task.deadline] = event.task;
//       break;
//     case Event::Type::kTaskFinish:
//       // 任务完成，从映射中移除

//       for (map<int, Event::Task>::iterator iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
//       {
//         if (iter->second.taskId == event.task.taskId)
//         {
//           TaskQueue.erase(iter);
//           break;
//         }
//       }
//       break;
//     case Event::Type::kIoRequest:
//       // 当前CPU任务请求IO，如果当前没有, 移入IO等待队列
//       TaskIO[event.task.deadline] = event.task;
//       for (map<int, Event::Task>::iterator iter = TaskQueue.begin(); iter != TaskQueue.end(); iter++)
//       {
//         if (iter->second.taskId == event.task.taskId)
//         {
//           TaskQueue.erase(iter);
//           break;
//         }
//       }
//       break;
//     case Event::Type::kIoEnd:
//       // IO完成，将任务从io等待队列移除，并放回合适的优先级队列
//       TaskQueue[event.task.deadline] = event.task;
//       for (map<int, Event::Task>::iterator iter = TaskIO.begin(); iter != TaskQueue.end(); iter++)
//       {
//         if (iter->second.taskId == event.task.taskId)
//         {
//           TaskIO.erase(iter);
//           break;
//         }
//       }
//       break;
//     }
//   }

//   // 选择下一个CPU和IO任务

//   if (current_io == 0)
//   {
//     map<int, Event::Task>::iterator next;
//     if (!TaskIO.empty())
//     {

//       for (next = TaskIO.begin(); next != TaskIO.end(); next++)
//       {
//         if (next->first > cur_time)
//           break;
//       }
//     }
//     if (next == TaskIO.end())
//       next = TaskIO.begin();
//     action.ioTask = next->second.taskId;
//   }

//   map<int, Event::Task>::iterator nextCPU;
//   for(nextCPU = TaskQueue.begin(); nextCPU != TaskQueue.end(); nextCPU++){

//     if(nextCPU->first > cur_time)
//       break;
//   }
//   if(nextCPU == TaskQueue.end())
//     nextCPU = TaskQueue.begin();
//   action.cpuTask = nextCPU->second.taskId;
//   return action;
// }
