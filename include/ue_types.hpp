#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace UE4 {

// ============================================================================
// Core Unreal Engine Types
// ============================================================================

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

// FName structure (for name pool)
struct FName {
    uint32_t ComparisonIndex;
    uint32_t Number;
};

// FString structure
struct FString {
    wchar_t* Data;
    int32_t Count;
    int32_t Max;
};

// Basic vector types
struct FVector {
    float X, Y, Z;
};

struct FVector2D {
    float X, Y;
};

struct FQuat {
    float X, Y, Z, W;
};

struct FRotator {
    float Pitch, Yaw, Roll;
};

struct FTransform {
    FQuat Rotation;
    FVector Translation;
    FVector Scale3D;
};

// ============================================================================
// Unreal Object System
// ============================================================================

class UObject;
class UClass;
class UProperty;
class UField;

// UObject base class
class UObject {
public:
    virtual ~UObject() {}
    
    int32_t VTableOffset;  // 0x00
    int32_t ObjectFlags;   // 0x08
    uint64_t Outer;        // 0x10 - Pointer to outer object
    uint64_t Name;         // 0x18 - FName
    uint64_t Class;        // 0x20 - UClass pointer
    
    static const int32_t UObjectSize = 0x30;
};

// UField - base for all field types
class UField : public UObject {
public:
    uint64_t Next;  // 0x30 - Pointer to next field
    
    static const int32_t UFieldSize = 0x38;
};

// UProperty
class UProperty : public UField {
public:
    int32_t ArrayDim;           // 0x38
    int32_t ElementSize;        // 0x3C
    uint64_t PropertyFlags;     // 0x40
    int32_t Offset_Internal;    // 0x48
    uint8_t FieldClass;         // 0x4C
    
    static const int32_t UPropertySize = 0x50;
};

// UStruct
class UStruct : public UField {
public:
    uint64_t SuperStruct;       // 0x38
    uint64_t Children;          // 0x40
    uint64_t ChildProperties;   // 0x48
    int32_t PropertiesSize;     // 0x50
    int32_t MinAlignment;       // 0x54
    
    static const int32_t UStructSize = 0x58;
};

// UClass
class UClass : public UStruct {
public:
    // Additional class-specific fields
    uint32_t ClassFlags;        // 0x58
    int32_t ClassUnique;        // 0x5C
    
    static const int32_t UClassSize = 0x60;
};

// ============================================================================
// FUObjectItem (for GUObjectArray iteration)
// ============================================================================

struct FUObjectItem {
    uint64_t Object;            // Actual object pointer (with flags in lower bits)
    int32_t Flags;
    int32_t ClusterIndex;
    int32_t SerialNumber;
    
    UObject* GetObject() const {
        return (UObject*)(Object & ~0xF);  // Mask out lower bits
    }
};

// ============================================================================
// Global Pointers (to be found via pattern scanning)
// ============================================================================

struct GlobalPointers {
    uint64_t GWorld;            // World pointer
    uint64_t GNames;            // Name pool (or NamePoolData)
    uint64_t GUObjectArray;     // Object array
    uint64_t DecryptionFunc;    // Pointer decryption function address
};

} // namespace UE4
