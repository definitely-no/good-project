/**
 * Windows 11 Code Integrity Module (ci.dll) - Architectural Header
 * Reconstructed Source Code from Windows 11 Build 26300.9457 (ge_release, amd64)
 * Architecture: x86_64 PE32+ (Subsystem 1: Native)
 */

#ifndef _CI_RECONSTRUCTED_H_
#define _CI_RECONSTRUCTED_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STATUS_SUCCESS                   0x00000000
#define STATUS_INVALID_PARAMETER         0xC000000D
#define STATUS_ACCESS_DENIED             0xC0000022
#define STATUS_IMAGE_CERT_REVOKED        0xC0000603
#define STATUS_INVALID_IMAGE_HASH        0xC0000428

#define SHA256_HASH_SIZE                 32

typedef struct _CI_POLICY_INFO {
    uint32_t StructSize;
    uint32_t PolicyFlags;
    uint64_t PolicyRuleMask;
} CI_POLICY_INFO, *PCI_POLICY_INFO;

typedef struct _CI_VALIDATION_INFO {
    uint32_t Size;
    uint32_t ValidationFlags;
    uint8_t  Digest[SHA256_HASH_SIZE];
    uint32_t DigestSize;
    uint32_t HashAlgorithmId;
    void    *CertChainContext;
} CI_VALIDATION_INFO, *PCI_VALIDATION_INFO;

/* Code Integrity Exports */
int32_t CiInitialize(uint32_t Flags, void *KernelCallbacks, uint32_t Options, PCI_POLICY_INFO PolicyInfo, void *Reserved);
int32_t CiCheckSignedFile(const void *FileBuffer, uint32_t FileSize, PCI_VALIDATION_INFO ValidationInfo);
int32_t CiValidateFileObject(void *FileObject, int32_t ValidationMode, PCI_VALIDATION_INFO ValidationInfo);
int32_t CiVerifyHashInCatalog(const uint8_t *Hash, uint32_t HashSize, uint32_t HashAlg, PCI_VALIDATION_INFO ValidationInfo);
int32_t CiFreePolicyInfo(PCI_POLICY_INFO PolicyInfo);

#endif /* _CI_RECONSTRUCTED_H_ */
