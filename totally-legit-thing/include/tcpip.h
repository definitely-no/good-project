/**
 * Windows 11 TCP/IP Network Stack (tcpip.sys) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _TCPIP_RECONSTRUCTED_H_
#define _TCPIP_RECONSTRUCTED_H_

#include "ntoskrnl.h"

#define IPPROTO_TCP 6
#define IPPROTO_UDP 17

typedef struct _IPV4_HEADER {
    uint8_t  VersionAndHeaderLength;
    uint8_t  TypeOfService;
    uint16_t TotalLength;
    uint16_t Identification;
    uint16_t FlagsAndFragmentOffset;
    uint8_t  TimeToLive;
    uint8_t  Protocol;
    uint16_t Checksum;
    uint32_t SourceAddress;
    uint32_t DestinationAddress;
} IPV4_HEADER, *PIPV4_HEADER;

typedef struct _TCP_HEADER {
    uint16_t SourcePort;
    uint16_t DestinationPort;
    uint32_t SequenceNumber;
    uint32_t AcknowledgmentNumber;
    uint16_t DataOffsetAndFlags;
    uint16_t WindowSize;
    uint16_t Checksum;
    uint16_t UrgentPointer;
} TCP_HEADER, *PTCP_HEADER;

/* TCP/IP Dispatch Prototypes */
NTSTATUS TcpipDriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);
NTSTATUS TcpipDispatchDeviceControl(PDEVICE_OBJECT DeviceObject, PIRP Irp);

#endif /* _TCPIP_RECONSTRUCTED_H_ */
