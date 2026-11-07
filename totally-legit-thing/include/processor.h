/**
 * Windows 11 Processor Management & Topology - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 */

#ifndef _PROCESSOR_RECONSTRUCTED_H_
#define _PROCESSOR_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define MAXIMUM_PROCESSORS_PER_GROUP 64
#define MAXIMUM_PROCESSOR_GROUPS     4

typedef struct _PROCESSOR_GROUP_INFO {
    uint8_t  MaximumProcessorCount;
    uint8_t  ActiveProcessorCount;
    uint8_t  Reserved[6];
    uint64_t ActiveProcessorMask;
} PROCESSOR_GROUP_INFO, *PPROCESSOR_GROUP_INFO;

typedef enum _PROCESSOR_EFFICIENCY_CLASS {
    ProcessorEfficiencyCore = 0,     /* E-Core */
    ProcessorPerformanceCore = 1     /* P-Core */
} PROCESSOR_EFFICIENCY_CLASS;

typedef struct _PROCESSOR_CORE_TOPOLOGY {
    uint32_t ProcessorIndex;
    uint16_t Group;
    uint8_t  EfficiencyClass;
    uint8_t  NumaNode;
    uint64_t CoreQuantum;
} PROCESSOR_CORE_TOPOLOGY, *PPROCESSOR_CORE_TOPOLOGY;

/* Processor Management Prototypes */
void KeInitializeProcessorTopology(uint32_t TotalProcessors);
uint32_t KeGetActiveProcessorCount(uint16_t GroupNumber);
uint8_t KeGetProcessorEfficiencyClass(uint32_t ProcessorIndex);

#endif /* _PROCESSOR_RECONSTRUCTED_H_ */
