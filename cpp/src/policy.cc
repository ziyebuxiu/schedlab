#include "policy.h"
#include<iostream>
#include <map>
#include <unordered_map>
#include <queue>
using namespace std;

// 假设系统有固定数量的优先级队列
// const int kNumPriorityLevels = 3;  // 例如，三个优先级级别
// 优先级队列，每个优先级一个队列
std::vector<std::queue<Event::Task>> priorityQueues;

// IO等待队列
std::queue<Event::Task> ioWaitQueue;

// 记录任务和其当前优先级的映射
std::unordered_map<int, int> taskPriorityMap;
std::unordered_map<int, Event::Task> taskId2Event;

// 选择下一个CPU任务
int selectNextCPUTask() {
    for (auto &queue : priorityQueues) {
        if (!queue.empty()) {
            Event::Task task = queue.front();
            queue.pop();
            return task.taskId;
        }
    }
    return -1; // 如果没有任务，返回-1
}

// 选择下一个IO任务
int selectNextIOTask() {
    if (!ioWaitQueue.empty()) {
        Event::Task task = ioWaitQueue.front();
        ioWaitQueue.pop();
        return task.taskId;
    }
    return -1; // 如果没有IO任务，返回-1
}

int remainTime(Event eve){//任务剩余时间
  return eve.task.deadline - eve.task.arrivalTime;
}
bool cmp(Event a, Event b){//比较任务优先级
  if(a.task.priority == Event::Task::Priority::kHigh){
    if(b.task.priority == Event::Task::Priority::kLow)
      return true;
    else
      return remainTime(a) < remainTime(b);
  }
  if(b.task.priority == Event::Task::Priority::kHigh){
    if(a.task.priority == Event::Task::Priority::kLow)
      return false;
    else
      return remainTime(a) < remainTime(b);
  }
  return remainTime(a) < remainTime(b);
}
// 任务完成或时间片耗尽时降低任务的优先级
void demoteTask(int taskId) {
    int priority = taskPriorityMap[taskId];
    if (priority < priorityQueues.size() - 1) {
        // 从当前优先级队列中移除任务
        std::queue<Event::Task> &currentQueue = priorityQueues[priority];
        std::queue<Event::Task> tempQueue;
        while (!currentQueue.empty()) {
            Event::Task tempTask = currentQueue.front();
            currentQueue.pop();
            if (tempTask.taskId != taskId) {
                tempQueue.push(tempTask);
            }
        }
        // 除去指定任务后，恢复其他任务到原队列
        std::swap(currentQueue, tempQueue);

        // 将任务移动到下一个优先级队列
        priority++;
        priorityQueues[priority].push(taskId2Event[taskId]);
        taskPriorityMap[taskId] = priority;
    }
}


Action policy(const std::vector<Event>& events, int current_cpu,
              int current_io) {
  for(int i=0;i<events.size();i++){
    taskId2Event[events[i].task.taskId] = events[i].task;
  }
  for (const auto& event : events) {
    queue<Event::Task> tmp;
        switch (event.type) {
            case Event::Type::kTimer:
                // 时间片耗尽，降低当前CPU任务的优先级
                if (current_cpu != -1) {
                    demoteTask(current_cpu);
                }
                break;
            case Event::Type::kTaskArrival:
                tmp.push(event.task);
                // 新任务到达，放入最高优先级队列
                priorityQueues.push_back(tmp);
                taskPriorityMap[event.task.taskId] = 0;
                break;
            case Event::Type::kIoRequest:
                // 当前CPU任务请求IO，移入IO等待队列
                if (current_cpu == event.task.taskId) {
                    ioWaitQueue.push(event.task);
                }
                break;
            case Event::Type::kIoEnd:
                // IO完成，将任务放回合适的优先级队列
                priorityQueues[taskPriorityMap[event.task.taskId]].push(event.task);
                break;
            case Event::Type::kTaskFinish:
                // 任务完成，从映射中移除
                taskPriorityMap.erase(event.task.taskId);
                break;
        }
    }

    // 选择下一个CPU和IO任务
    Action action;
    action.cpuTask = selectNextCPUTask();
    action.ioTask = selectNextIOTask();

    return action;
}
