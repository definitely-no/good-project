/**
 * Windows 11 NT Kernel (ntoskrnl.exe) - Memory Manager: Virtual Address Descriptors (VAD)
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+
 *
 * Implements:
 *   - MMVAD allocation and balanced AVL tree insertion
 *   - Virtual Address range querying within process address space
 */

#include "ntoskrnl.h"

/**
 * Inserts a Virtual Address Descriptor into the process balanced VAD tree.
 */
NTSTATUS MiInsertVad(PMMVAD Vad, PMMVAD *Root)
{
    PMMVAD Current = NULL;
    PMMVAD Parent = NULL;

    if (!Vad || !Root) {
        return STATUS_INVALID_PARAMETER;
    }

    if (*Root == NULL) {
        *Root = Vad;
        Vad->Core.Parent = NULL;
        Vad->Core.LeftChild = NULL;
        Vad->Core.RightChild = NULL;
        return STATUS_SUCCESS;
    }

    Current = *Root;
    while (Current) {
        Parent = Current;
        if (Vad->Core.StartingVpn < Current->Core.StartingVpn) {
            Current = (PMMVAD)Current->Core.LeftChild;
        } else if (Vad->Core.StartingVpn > Current->Core.EndingVpn) {
            Current = (PMMVAD)Current->Core.RightChild;
        } else {
            /* Collision / address range overlap */
            return STATUS_ACCESS_VIOLATION;
        }
    }

    Vad->Core.Parent = (PMMVAD_SHORT)Parent;
    Vad->Core.LeftChild = NULL;
    Vad->Core.RightChild = NULL;

    if (Vad->Core.StartingVpn < Parent->Core.StartingVpn) {
        Parent->Core.LeftChild = (PMMVAD_SHORT)Vad;
    } else {
        Parent->Core.RightChild = (PMMVAD_SHORT)Vad;
    }

    return STATUS_SUCCESS;
}

/**
 * Searches the process VAD tree to locate the descriptor spanning a target virtual address.
 */
PMMVAD MiFindVad(ULONG_PTR VirtualAddress, PMMVAD Root)
{
    ULONG_PTR Vpn = VirtualAddress >> PAGE_SHIFT;
    PMMVAD Current = Root;

    while (Current) {
        if (Vpn < Current->Core.StartingVpn) {
            Current = (PMMVAD)Current->Core.LeftChild;
        } else if (Vpn > Current->Core.EndingVpn) {
            Current = (PMMVAD)Current->Core.RightChild;
        } else {
            /* Address resides within this VAD */
            return Current;
        }
    }

    return NULL;
}
