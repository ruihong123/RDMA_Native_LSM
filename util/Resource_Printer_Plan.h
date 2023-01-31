//
// Created by ruihong on 9/24/22.
//

#ifndef TIMBERSAW_RESOURCE_PRINTER_PLAN_H
#define TIMBERSAW_RESOURCE_PRINTER_PLAN_H

#include <string>

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/times.h"
//#include "sys/vtimes.h"
#include "cassert"

#define NUMA_CORE_NUM 8
#define COMPUTE_NUMA_CORE_NUM 152
#define CPU_UTILIZATION_CACULATE_INTERVAL 10 // in miliseconds.

class Resource_Printer_PlanA {
  unsigned long long lastTotalUser[NUMA_CORE_NUM], lastTotalUserLow[NUMA_CORE_NUM], lastTotalSys[NUMA_CORE_NUM], lastTotalIdle[NUMA_CORE_NUM];

 public:
  Resource_Printer_PlanA();

  long double getCurrentValue();
  std::string getCurrentHost();
};

class Resource_Printer_PlanB {
  clock_t lastCPU, lastSysCPU, lastUserCPU;

 public:
  Resource_Printer_PlanB();
  void paramInit();
  long double current_percent;
  int numa_bind_core_num = 0;
  double getCurrentValue();
  std::string getCurrentHost();
};

#endif  // TIMBERSAW_RESOURCE_PRINTER_PLAN_H
