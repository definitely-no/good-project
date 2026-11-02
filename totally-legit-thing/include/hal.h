/**
 * Windows 11 Hardware Abstraction Layer (hal.dll) - Architectural Reconstruction
 * Reverse Engineered from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Target: extracted_kernel/hal.dll (SHA-256: 6f8ec8f2fba2ad2ea678fa8e97e6f1ee677f6b99e4868ee2e94ebf175815a5bb)
 *
 * ARCHITECTURAL FINDING:
 * In modern Windows 11 64-bit releases, hal.dll contains NO executable .text code.
 * Instead, hal.dll acts as an Export Forwarder stub module containing an .edata section
 * that directly forwards 100% of its 91 exported functions into ntoskrnl.exe.
 * This unifies kernel and HAL hardware interaction into the main NT kernel image
 * while preserving legacy driver ABI compatibility.
 */

#ifndef _HAL_FORWARDERS_H_
#define _HAL_FORWARDERS_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HAL_EXPORT_COUNT 91

/* Structure documenting each forwarder export */
typedef struct _HAL_FORWARDER_ENTRY {
    uint16_t Ordinal;
    const char *ExportName;
    const char *ForwarderTarget;
} HAL_FORWARDER_ENTRY;

/* Table of all 91 forwarder stubs from hal.dll to ntoskrnl.exe */
extern const HAL_FORWARDER_ENTRY g_HalExportTable[HAL_EXPORT_COUNT];

/* Core HAL Functions Forwarded to ntoskrnl.exe */
uint32_t HalAcpiGetTableEx(void *Context, uint32_t Signature, void *Table);
void HalAcquireDisplayOwnership(void);
void HalAdjustResourceList(void *ResourceList);
void HalAllProcessorsStarted(void);
uint32_t HalAllocateAdapterChannel(void *DmaAdapter, void *DeviceObject, uint32_t NumberOfMapRegisters, void *ExecutionRoutine, void *Context);
void* HalAllocateCommonBuffer(void *DmaAdapter, uint32_t Length, uint64_t *LogicalAddress, bool CacheEnabled);
uint32_t HalAllocateCrashDumpRegisters(void *DmaAdapter, uint32_t *NumberOfMapRegisters);
uint32_t HalAllocateHardwareCounters(void *CounterGroup, uint32_t CounterCount, void *CounterArray);
uint32_t HalAssignSlotResources(void *RegistryPath, void *DriverClassName, void *DriverObject, void *DeviceObject, uint32_t BusDataType, uint32_t BusNumber, uint32_t SlotNumber, void **AllocatedResources);
void HalBugCheckSystem(uint32_t BugCheckCode, uint64_t Param1, uint64_t Param2, uint64_t Param3, uint64_t Param4);
uint64_t HalCalibratePerformanceCounter(uint64_t *Frequency, uint32_t Interval);
void HalClearSoftwareInterrupt(uint32_t Request);
uint8_t HalConvertDeviceIdtToIrql(uint32_t Vector);
void HalDisableInterrupt(uint32_t Vector);
void HalDisplayString(const char *String);
uint32_t HalDmaAllocateCrashDumpRegistersEx(void *DmaAdapter, uint32_t Flags, uint32_t NumberOfMapRegisters, void *MapRegisterBase);
void HalDmaFreeCrashDumpRegistersEx(void *DmaAdapter, void *MapRegisterBase, uint32_t NumberOfMapRegisters);
bool HalEnableInterrupt(uint32_t Vector, uint8_t Irql);
uint32_t HalEnumerateEnvironmentVariablesEx(uint32_t InformationClass, void *Buffer, uint32_t *BufferLength);
uint32_t HalEnumerateProcessors(void *ProcessorInformation, uint32_t *ProcessorCount);
void HalFlushCommonBuffer(void *DmaAdapter, uint32_t Length, uint64_t LogicalAddress, void *VirtualAddress, bool CacheEnabled);
void HalFreeCommonBuffer(void *DmaAdapter, uint32_t Length, uint64_t LogicalAddress, void *VirtualAddress, bool CacheEnabled);
void HalFreeHardwareCounters(void *CounterGroup);
void* HalGetAdapter(void *DeviceDescription, uint32_t *NumberOfMapRegisters);
uint32_t HalGetBusData(uint32_t BusDataType, uint32_t BusNumber, uint32_t SlotNumber, void *Buffer, uint32_t Length);
uint32_t HalGetBusDataByOffset(uint32_t BusDataType, uint32_t BusNumber, uint32_t SlotNumber, void *Buffer, uint32_t Offset, uint32_t Length);
uint32_t HalGetEnvironmentVariable(const wchar_t *Name, void *Guid, void *Buffer, uint32_t *Length);
uint32_t HalGetEnvironmentVariableEx(const wchar_t *Name, void *Guid, void *Buffer, uint32_t *Length, uint32_t *Attributes);
uint32_t HalGetInterruptTargetInformation(uint32_t Vector, uint32_t Flags, void *TargetInformation);
uint32_t HalGetInterruptVector(uint32_t InterfaceType, uint32_t BusNumber, uint32_t BusInterruptLevel, uint32_t BusInterruptVector, uint8_t *Irql, uint64_t *Affinity);
uint32_t HalGetMemoryCachingRequirements(uint64_t PhysicalAddress, uint64_t Length, uint32_t *CachingType);
uint32_t HalGetMessageRoutingInfo(uint32_t InterruptType, void *RoutingInfo);
uint32_t HalGetProcessorIdByNtNumber(uint32_t ProcessorNumber, uint32_t *ApicId);
uint32_t HalGetVectorInput(uint32_t Vector, void *VectorInput);
void HalHandleMcheck(void *Context);
bool HalHandleNMI(void *Context);
bool HalInitSystem(uint32_t Phase, void *LoaderBlock);
void HalInitializeBios(uint32_t BiosPhase, void *BiosBlock);
void HalInitializeOnResume(void);
bool HalInitializeProcessor(uint32_t ProcessorNumber, void *LoaderBlock);
bool HalIsHyperThreadingEnabled(void);
void HalMakeBeep(uint32_t Frequency);
void HalPerformEndOfInterrupt(void);
void HalProcessorIdle(void);
uint32_t HalQueryDisplayParameters(uint32_t *Width, uint32_t *Height, uint32_t *CursorX, uint32_t *CursorY);
uint32_t HalQueryEnvironmentVariableInfoEx(uint32_t InformationClass, void *Guid, const wchar_t *Name, void *Buffer, uint32_t *Length);
uint32_t HalQueryMaximumProcessorCount(void);
bool HalQueryRealTimeClock(void *TimeFields);
uint32_t HalReadDmaCounter(void *DmaAdapter);
uint32_t HalRegisterDynamicProcessor(void *ProcessorContext, uint32_t *ProcessorIndex);
void HalRegisterErrataCallbacks(void);
void HalReportResourceUsage(void);
void HalRequestClockInterrupt(uint32_t ClockFrequency);
void HalRequestDeferredRecoveryServiceInterrupt(void);
void HalRequestIpi(uint64_t Affinity);
void HalRequestIpiSpecifyVector(uint64_t Affinity, uint32_t Vector);
void HalRequestSoftwareInterrupt(uint8_t Irql);
void HalReturnToFirmware(uint32_t Action);
void HalSendNMI(uint64_t Affinity);
void HalSendSoftwareInterrupt(uint8_t Irql, uint64_t Affinity);
uint32_t HalSetBusData(uint32_t BusDataType, uint32_t BusNumber, uint32_t SlotNumber, void *Buffer, uint32_t Length);
uint32_t HalSetBusDataByOffset(uint32_t BusDataType, uint32_t BusNumber, uint32_t SlotNumber, void *Buffer, uint32_t Offset, uint32_t Length);
void HalSetDisplayParameters(uint32_t CursorX, uint32_t CursorY);
uint32_t HalSetEnvironmentVariable(const wchar_t *Name, void *Guid, void *Value, uint32_t ValueLength);
uint32_t HalSetEnvironmentVariableEx(const wchar_t *Name, void *Guid, void *Value, uint32_t ValueLength, uint32_t Attributes);
uint32_t HalSetProfileInterval(uint32_t Interval);
bool HalSetRealTimeClock(void *TimeFields);
uint32_t HalStartDynamicProcessor(uint32_t ProcessorIndex, void *ProcessorContext);
bool HalStartNextProcessor(void *LoaderBlock, void *ProcessorState);
void HalStartProfileInterrupt(uint32_t Reserved);
void HalStopProfileInterrupt(uint32_t Reserved);
void HalSystemVectorDispatchEntry(void);
uint64_t HalTranslateBusAddress(uint32_t InterfaceType, uint32_t BusNumber, uint64_t BusAddress, uint32_t *AddressSpace, uint64_t *TranslatedAddress);
void HalWheaUpdateCmciPolicy(void *Policy);
bool IoFlushAdapterBuffers(void *DmaAdapter, void *Mdl, void *MapRegisterBase, void *CurrentVa, uint32_t Length, bool WriteToDevice);
void IoFreeAdapterChannel(void *DmaAdapter);
void IoFreeMapRegisters(void *DmaAdapter, void *MapRegisterBase, uint32_t NumberOfMapRegisters);
uint64_t IoMapTransfer(void *DmaAdapter, void *Mdl, void *MapRegisterBase, void *CurrentVa, uint32_t *Length, bool WriteToDevice);
uint32_t IoReadPartitionTable(void *DeviceObject, uint32_t SectorSize, bool ReturnRecognizedPartitions, void **PartitionBuffer);
uint32_t IoSetPartitionInformation(void *DeviceObject, uint32_t SectorSize, uint32_t PartitionNumber, uint32_t PartitionType);
uint32_t IoWritePartitionTable(void *DeviceObject, uint32_t SectorSize, uint32_t PartitionCount, uint32_t DiskSignature, void *PartitionBuffer);
bool KdComPortInUse(void *PortInformation);
bool KdHvComPortInUse(void *PortInformation);
void KeFlushWriteBuffer(void);
int64_t KeQueryPerformanceCounter(int64_t *PerformanceFrequency);
void KeStallExecutionProcessor(uint32_t Microseconds);
void* x86BiosAllocateBuffer(uint32_t *Length, void **BiosBuffer);
bool x86BiosCall(uint16_t BiosInterrupt, void *BiosRegisters);
void x86BiosFreeBuffer(void *BiosBuffer);
bool x86BiosReadMemory(uint32_t PhysicalAddress, void *Buffer, uint32_t Length);
bool x86BiosWriteMemory(uint32_t PhysicalAddress, void *Buffer, uint32_t Length);

#endif /* _HAL_FORWARDERS_H_ */
