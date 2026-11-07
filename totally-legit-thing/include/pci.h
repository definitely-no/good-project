/**
 * Windows 11 PCI Bus Driver (pci.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _PCI_RECONSTRUCTED_H_
#define _PCI_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#pragma pack(push, 1)
typedef struct _PCI_COMMON_CONFIG {
    uint16_t VendorID;
    uint16_t DeviceID;
    uint16_t Command;
    uint16_t Status;
    uint8_t  RevisionID;
    uint8_t  ProgIf;
    uint8_t  SubClass;
    uint8_t  BaseClass;
    uint8_t  CacheLineSize;
    uint8_t  LatencyTimer;
    uint8_t  HeaderType;
    uint8_t  BIST;
    uint32_t BaseAddresses[6];
    uint32_t CardBusCISPtr;
    uint16_t SubsystemVendorID;
    uint16_t SubsystemID;
    uint32_t ROMBaseAddress;
    uint8_t  CapabilitiesPtr;
    uint8_t  Reserved[7];
    uint8_t  InterruptLine;
    uint8_t  InterruptPin;
    uint8_t  MinGnt;
    uint8_t  MaxLat;
} PCI_COMMON_CONFIG, *PPCI_COMMON_CONFIG;
#pragma pack(pop)

/* PCI Driver Prototypes */
NTSTATUS PciDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS PciReadConfigSpace(uint8_t Bus, uint8_t Device, uint8_t Function, PPCI_COMMON_CONFIG Config);

#endif /* _PCI_RECONSTRUCTED_H_ */
