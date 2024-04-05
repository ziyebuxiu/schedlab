#include "policy.h"
#include <iostream>
#include <map>
#include <unordered_map>
#include <queue>
#include <algorithm>
using namespace std;

// 优先级队列，每个优先级一个队列
std::vector<std::vector<Event::Task>> priorityQueues;

// IO等待队列
std::vector<Event::Task> ioWaitQueue;

// 记录任务和其当前优先级的映射
std::unordered_map<int, int> taskPriorityMap;
std::unordered_map<int, Event::Task> taskId2Event;

void removeTaskFromCPUQueue(int taskId){
  for (auto& queue : priorityQueues) {  // 遍历每个优先级队列
        auto it = std::remove_if(queue.begin(), queue.end(),
                                 [taskId](const Event::Task& task) { return task.taskId == taskId; });
        if (it != queue.end()) {  // 如果找到了任务
            queue.erase(it, queue.end());  // 从队列中移除任务
            break;  // 任务移除后退出循环
        }
    }
}
void removeTaskFromIOQueue(int taskId)
{
    auto it = std::remove_if(ioWaitQueue.begin(), ioWaitQueue.end(),
                             [taskId](const Event::Task& task) { return task.taskId == taskId; });
    if (it != ioWaitQueue.end()) {  // 如果在IO等待队列中找到任务
        ioWaitQueue.erase(it, ioWaitQueue.end());  // 从队列中移除任务
    }
}
// 记录当前正在进行IO的任务ID
// int currentIOTaskId = -1;
// 选择下一个CPU任务
int selectNextCPUTask(int currentIOTaskId)
{
  for (auto &queue : priorityQueues)
  {
    for (int i = 0; i < queue.size(); i++)
    {
      if (queue[i].taskId != currentIOTaskId)
        return queue[i].taskId;
      // if (!queue.empty())
      // {
      //   // 查看队列前端任务是否正在进行IO
      //   Event::Task task = queue.front();
      //   if (task.taskId == currentIOTaskId)
      //     continue;
      //   queue.pop();
      //   return task.taskId;
      // }
    }
  }
  return -1; // 如果没有任务，返回-1
}

// 选择下一个IO任务
int selectNextIOTask(int currentIOTaskId)
{
  for (int i = 0; i < ioWaitQueue.size(); i++)
  {
    if (ioWaitQueue[i].taskId == currentIOTaskId)
      continue;
    return ioWaitQueue[i].taskId;
  }
  // if (!ioWaitQueue.empty())
  // {
  //   Event::Task task = ioWaitQueue.front();
  //   ioWaitQueue.pop();
  //   return task.taskId;
  // }
  return -1; // 如果没有IO任务，返回-1
}

int remainTime(Event eve)
{ // 任务剩余时间
  return eve.task.deadline - eve.task.arrivalTime;
}
bool cmp(Event a, Event b)
{ // 比较任务优先级
  if (a.task.priority == Event::Task::Priority::kHigh)
  {
    if (b.task.priority == Event::Task::Priority::kLow)
      return true;
    else
      return remainTime(a) < remainTime(b);
  }
  if (b.task.priority == Event::Task::Priority::kHigh)
  {
    if (a.task.priority == Event::Task::Priority::kLow)
      return false;
    else
      return remainTime(a) < remainTime(b);
  }
  return remainTime(a) < remainTime(b);
}
// 任务完成或时间片耗尽时降低任务的优先级
void demoteTask(int taskId, int currentIOTaskId)
{
  if (taskId == currentIOTaskId)
    return;
  int priority = taskPriorityMap[taskId];
  // queue<Event::Task> emptyQueue;
  // priorityQueues.push_back(emptyQueue);

  // 从当前优先级队列中移除任务
  std::vector<Event::Task> &currentQueue = priorityQueues[priority];
  std::vector<Event::Task> tempQueue;
  for (auto it = currentQueue.begin(); it != currentQueue.end(); ++it)
  {
    if (it->taskId != taskId)
    {
      tempQueue.push_back(*it);
    }
  }
  // 除去指定任务后，恢复其他任务到原队列
  currentQueue.swap(tempQueue);

  // 将任务移动到下一个优先级队列
  priority++;
  if (priority >= priorityQueues.size())
    priorityQueues.resize(priority + 1);
  priorityQueues[priority].push_back(taskId2Event[taskId]);

  // 更新任务的优先级信息
  taskPriorityMap[taskId] = priority;
}

Action policy(const std::vector<Event> &events, int current_cpu,
              int current_io)
{
  Action action;
  action.cpuTask = current_cpu;
  action.ioTask = current_io;
  // currentIOTaskId = current_io;

  for (int i = 0; i < events.size(); i++)
  {
    taskId2Event[events[i].task.taskId] = events[i].task;
    taskPriorityMap[events[i].task.taskId] = events[i].task.priority == Event::Task::Priority::kHigh ? 0 : 1;
  }
  for (const auto &event : events)
  {
    queue<Event::Task> tmp;
    int priority = (event.task.priority == Event::Task::Priority::kHigh) ? 0 : 1;
    switch (event.type)
    {
    case Event::Type::kTimer:
      // 时间片耗尽，降低当前CPU任务的优先级
      if (current_cpu != 0)
      {
        demoteTask(current_cpu, current_io);
      }
      break;
    case Event::Type::kTaskArrival:
      // 新任务到达，放入最高优先级队列

      if (priorityQueues.size() <= priority)
      {
        priorityQueues.resize(priority + 1);
      }
      priorityQueues[priority].push_back(event.task);
      taskPriorityMap[event.task.taskId] = priority;
      break;
    case Event::Type::kIoRequest:
      // 当前CPU任务请求IO，如果当前没有, 移入IO等待队列
      // if (current_cpu == event.task.taskId)
      // {
      ioWaitQueue.push_back(event.task);
      // currentIOTaskId = event.task.taskId;
      // }
      if (current_cpu == event.task.taskId)
        removeTaskFromCPUQueue(current_cpu);
      break;
    case Event::Type::kIoEnd:
      // IO完成，将任务从io等待队列移除，并放回合适的优先级队列
      if (!ioWaitQueue.empty())
      {
        auto it = std::find_if(ioWaitQueue.begin(), ioWaitQueue.end(),
                               [&event](const Event::Task &task)
                               {
                                 return task.taskId == event.task.taskId;
                               });

        bool found = (it != ioWaitQueue.end());
        if (found)
        {
          // 任务在IO队列中找到，现在把它放回CPU队列
          priorityQueues[taskPriorityMap[event.task.taskId]].push_back(*it);
          // 现在从ioWaitQueue中移除这个任务
          ioWaitQueue.erase(it);
        }
        // currentIOTaskId = -1;
      }

      break;
    case Event::Type::kTaskFinish:
      // 任务完成，从映射中移除
      taskPriorityMap.erase(event.task.taskId);
      removeTaskFromCPUQueue(event.task.taskId);
      removeTaskFromIOQueue(event.task.taskId);
      break;
    }
  }

  // 选择下一个CPU和IO任务
  action.cpuTask = selectNextCPUTask(current_io);
  action.ioTask = selectNextIOTask(current_io);

  return action;
}
