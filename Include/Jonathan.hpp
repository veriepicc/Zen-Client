/*
 * Jonathan - Tiny Hooking Library
 * -------------------------------
 * Zen Development 2025
 *
 * License: MIT
 *
 * Design Goals:
 *  - Small and fast
 *  - No external dependencies
 *  - Uses 5-byte jumps when possible; absolute jumps otherwise
 *  - Allocates trampolines near target functions for better performance
 *  - Thread suspension can be disabled for UWP compatibility
 */

#pragma once

#if !(defined _M_IX86) && !(defined _M_X64) && !(defined __i386__) && !(defined __x86_64__)
    #error Jonathan supports only x86 and x64 systems.
#endif

#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <utility>

#if defined(_MSC_VER)
    #include <intrin.h>
    #define JN_FORCEINLINE __forceinline
#else
    #define JN_FORCEINLINE inline __attribute__((always_inline))
#endif

// Config
#ifndef JONATHAN_CFG_X64_ABS_JMP
#define JONATHAN_CFG_X64_ABS_JMP 1  // use FF 25 [RIP+0] + 8-byte addr for long jumps
#endif

#ifndef JONATHAN_CFG_X64_PREFER_REL32
#define JONATHAN_CFG_X64_PREFER_REL32 1  // prefer 5-byte E9 rel32 when in ±2GB range
#endif

#ifndef JONATHAN_CFG_FREEZE_THREADS
#define JONATHAN_CFG_FREEZE_THREADS 1 // suspend threads during patching and fix IP
#endif

#ifndef JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE
#  if defined(_M_X64) || defined(__x86_64__)
#    define JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE 64
#  else
#    define JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE 32
#  endif
#endif

namespace Jonathan {

namespace config {
#if JONATHAN_CFG_X64_ABS_JMP
    inline constexpr bool x64AbsJmp = true;
#else
    inline constexpr bool x64AbsJmp = false;
#endif

#if JONATHAN_CFG_X64_PREFER_REL32
    inline constexpr bool preferRel32 = true;
#else
    inline constexpr bool preferRel32 = false;
#endif

#if JONATHAN_CFG_FREEZE_THREADS
    inline constexpr bool freezeThreads = true;
#else
    inline constexpr bool freezeThreads = false;
#endif

    inline constexpr size_t trampolineSlotSize = static_cast<size_t>(JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE);
#if defined(_M_X64) || defined(__x86_64__)
    inline constexpr size_t absJmpSize = 14; // FF 25 + 8 bytes = pain
#else
    inline constexpr size_t absJmpSize = 5;  // x86 is simpler
#endif
    inline constexpr size_t relJmpSize = 5;
#if defined(_M_X64) || defined(__x86_64__)
    inline constexpr uintptr_t nearRange = 0x40000000ull; // 1GB should be enough
#else
    inline constexpr uintptr_t nearRange = 0x7FFFFFFF;
#endif
}

// Status

enum class status : int32_t {
    unknown = -1,
    ok = 0,
    already_initialized,
    not_initialized,
    already_created,
    not_created,
    enabled,
    disabled,
    not_executable,
    unsupported_function,
    memory_alloc,
    memory_protect,
    module_not_found,
    function_not_found
};

[[nodiscard]] JN_FORCEINLINE const char* status_to_string(status s) noexcept {
    switch (s) {
        case status::ok: return "OK";
        case status::unknown: return "UNKNOWN";
        case status::already_initialized: return "ALREADY_INITIALIZED";
        case status::not_initialized: return "NOT_INITIALIZED";
        case status::already_created: return "ALREADY_CREATED";
        case status::not_created: return "NOT_CREATED";
        case status::enabled: return "ENABLED";
        case status::disabled: return "DISABLED";
        case status::not_executable: return "NOT_EXECUTABLE";
        case status::unsupported_function: return "UNSUPPORTED_FUNCTION";
        case status::memory_alloc: return "MEMORY_ALLOC";
        case status::memory_protect: return "MEMORY_PROTECT";
        case status::module_not_found: return "MODULE_NOT_FOUND";
        case status::function_not_found: return "FUNCTION_NOT_FOUND";
        default: return "(unknown)";
    }
}

// Memory

namespace mem {
    enum : DWORD {
        page_exec_flags = (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY)
    };

    [[nodiscard]] JN_FORCEINLINE bool is_executable_address(void* address) noexcept {
        MEMORY_BASIC_INFORMATION mi{};
        if (!VirtualQuery(address, &mi, sizeof(mi))) return false;
        return mi.State == MEM_COMMIT && (mi.Protect & page_exec_flags);
    }

    JN_FORCEINLINE void uninitialize() noexcept {}

    [[nodiscard]] JN_FORCEINLINE uint8_t* try_alloc_page(void* where) noexcept {
        SYSTEM_INFO si{}; GetSystemInfo(&si);
#if defined(_M_X64) || defined(__x86_64__)
        const uintptr_t gran = si.dwAllocationGranularity;
        uintptr_t minAddr = (uintptr_t)si.lpMinimumApplicationAddress;
        uintptr_t maxAddr = (uintptr_t)si.lpMaximumApplicationAddress;
        uintptr_t origin  = (uintptr_t)where;
        const uintptr_t range = 0x40000000ull;
        if (origin > range && minAddr < origin - range) minAddr = origin - range;
        if (maxAddr > origin + range) maxAddr = origin + range;
        maxAddr -= 0x1000 - 1; // room for one page

        uintptr_t probe = origin;
        probe -= probe % gran;
        for (;;) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery((void*)probe, &mbi, sizeof(mbi))) break;
            if (mbi.State == MEM_FREE) {
                void* p = VirtualAlloc((void*)probe, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (p) return (uint8_t*)p;
            }
            if ((uintptr_t)mbi.AllocationBase < gran) break;
            if (probe < minAddr + gran) break;
            probe = (uintptr_t)mbi.AllocationBase - gran;
        }

        probe = origin;
        probe -= probe % gran;
        probe += gran;
        while (probe <= maxAddr) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery((void*)probe, &mbi, sizeof(mbi))) break;
            if (mbi.State == MEM_FREE) {
                void* p = VirtualAlloc((void*)probe, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
                if (p) return (uint8_t*)p;
            }
            uintptr_t next = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
            next += gran - 1; next -= next % gran;
            if (next <= probe) break;
            probe = next;
        }
        return nullptr;
#else
        void* p = VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        return (uint8_t*)p;
#endif
    }

    [[nodiscard]] JN_FORCEINLINE uint8_t* alloc_slot(void* origin) noexcept {
        uint8_t* page = try_alloc_page(origin);
        if (!page) return nullptr;
        const size_t align = 32;
        const size_t max_off = (0x1000 - JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE);
        uint64_t r = __rdtsc();
        size_t off = (size_t)((r >> 4) % ((max_off / align) + 1)) * align;
#if defined(_DEBUG)
        std::memset(page + off, 0xCC, JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE);
#endif
        return page + off;
    }

    JN_FORCEINLINE void free_slot(void* ptr) noexcept {
        if (!ptr) return;
        uintptr_t base = (uintptr_t)ptr & ~(uintptr_t)0xFFF;
        VirtualFree((void*)base, 0, MEM_RELEASE);
    }
}

// Emit

namespace emit {
#if defined(_M_X64) || defined(__x86_64__)
    JN_FORCEINLINE void jmp_abs64(void* at, const void* dest) noexcept {
        uint8_t* p = (uint8_t*)at;
        p[0]=0xFF; p[1]=0x25; p[2]=0x00; p[3]=0x00; p[4]=0x00; p[5]=0x00;
        *(uint64_t*)(p + 6) = (uint64_t)(uintptr_t)dest;
    }

    JN_FORCEINLINE void call_abs64(void* at, const void* dest) noexcept {
        uint8_t* p = (uint8_t*)at;
        p[0]=0xFF; p[1]=0x15; p[2]=0x02; p[3]=0x00; p[4]=0x00; p[5]=0x00;
        p[6]=0xEB; p[7]=0x08;
        *(uint64_t*)(p + 8) = (uint64_t)(uintptr_t)dest;
    }
#endif

        JN_FORCEINLINE void jmp_rel32(void* at, intptr_t rel) noexcept {
        uint8_t* p = (uint8_t*)at;
        p[0]=0xE9; *(int32_t*)(p + 1) = (int32_t)rel;
    }

    JN_FORCEINLINE void call_rel32(void* at, intptr_t rel) noexcept {
        uint8_t* p = (uint8_t*)at;
        p[0]=0xE8; *(int32_t*)(p + 1) = (int32_t)rel;
    }

    JN_FORCEINLINE void jmp_rel8(void* at, int8_t rel) noexcept {
        uint8_t* p = (uint8_t*)at;
        p[0]=0xEB; p[1]=(uint8_t)rel;
    }

    [[nodiscard]] JN_FORCEINLINE bool jmp_auto(void* at, const void* srcNext, const void* dest) noexcept {
#if defined(_M_X64) || defined(__x86_64__)
        if constexpr (config::preferRel32) {
            intptr_t rel = (intptr_t)dest - ((intptr_t)srcNext);
            if (rel >= INT32_MIN && rel <= INT32_MAX) { jmp_rel32(at, rel); return true; }
        }
        jmp_abs64(at, dest); return false;
#else
        intptr_t rel = (intptr_t)dest - ((intptr_t)srcNext);
        jmp_rel32(at, rel); return true;
#endif
    }

    [[nodiscard]] JN_FORCEINLINE bool call_auto(void* at, const void* srcNext, const void* dest) noexcept {
#if defined(_M_X64) || defined(__x86_64__)
        if constexpr (config::preferRel32) {
            intptr_t rel = (intptr_t)dest - ((intptr_t)srcNext);
            if (rel >= INT32_MIN && rel <= INT32_MAX) { call_rel32(at, rel); return true; }
        }
        call_abs64(at, dest); return false;
#else
        intptr_t rel = (intptr_t)dest - ((intptr_t)srcNext);
        call_rel32(at, rel); return true;
#endif
    }
}

// Decoder (x86/x64)

enum : uint32_t {
    IDF_NONE      = 0,
    IDF_MODRM     = 1u << 0,
    IDF_SIB       = 1u << 1,
    IDF_DISP8     = 1u << 2,
    IDF_DISP32    = 1u << 3,
    IDF_IMM8      = 1u << 4,
    IDF_IMM16     = 1u << 5,
    IDF_IMM32     = 1u << 6,
    IDF_REL8      = 1u << 7,
    IDF_REL32     = 1u << 8,
    IDF_OPCODE_0F = 1u << 9,
};

struct insn_info {
    uint8_t len;
    uint32_t flags;
    uint8_t opcode;
    uint8_t opcode2;
};

// Returns 0 on failure
[[nodiscard]] inline JN_FORCEINLINE size_t decode_length_x86_64(const uint8_t* code, insn_info* out) noexcept {
    const uint8_t* p = code;
    uint8_t prefixes = 0;
    uint8_t rex = 0; // x64 REX

    // prefixes
    for (int i = 0; i < 16; ++i) {
        uint8_t c = *p;
        switch (c) {
            case 0xF0: case 0xF2: case 0xF3: case 0x2E: case 0x36: case 0x3E:
            case 0x26: case 0x64: case 0x65: case 0x66: case 0x67:
                prefixes = 1; ++p; continue;
            default: break;
        }
        break;
    }
#if defined(_M_X64) || defined(__x86_64__)
    if ((*p & 0xF0) == 0x40) { rex = *p++; }
#endif

    uint8_t op = *p++;
    uint8_t op2 = 0;
    uint32_t flags = 0;

    if (op == 0x0F) { op2 = *p++; flags |= IDF_OPCODE_0F; }

    auto need_modrm = [&](uint8_t primary, uint8_t secondary, bool has0F)->bool {
        if (!has0F) {
            if ((primary & 0xF0) == 0x50) return false;
            if (primary == 0x90 || primary == 0xCC) return false;
            if (primary == 0x68 || primary == 0x6A) return false;
            if ((primary & 0xF8) == 0xB8) return false;
            if (primary == 0xE8 || primary == 0xE9 || primary == 0xEB) return false;
            if ((primary & 0xF0) == 0x70) return false;
            if (primary == 0xC3 || primary == 0xC2) return false;
            return true;
        } else {
            if ((secondary & 0xF0) == 0x80) return false;
            return true;
        }
    };

    bool modrm_needed = need_modrm(op, op2, (flags & IDF_OPCODE_0F) != 0);
    uint8_t modrm = 0, mod = 0, rm = 0, reg = 0, sib = 0, ss = 0, base = 0, index = 0;
    if (modrm_needed) {
        modrm = *p++; flags |= IDF_MODRM;
        mod = (modrm >> 6) & 3; reg = (modrm >> 3) & 7; rm = modrm & 7;
        if (mod != 3 && rm == 4) { // SIB
            sib = *p++; flags |= IDF_SIB;
            ss = (sib >> 6) & 3; index = (sib >> 3) & 7; base = sib & 7;
        }
        if (mod == 0) {
            if (rm == 5) { flags |= IDF_DISP32; p += 4; }
            if ((flags & IDF_SIB) && base == 5) { flags |= IDF_DISP32; p += 4; }
        } else if (mod == 1) { flags |= IDF_DISP8;  p += 1; }
        else if (mod == 2)   { flags |= IDF_DISP32; p += 4; }
    }

    // immediates / rel
    if (!(flags & IDF_OPCODE_0F)) {
        if (op == 0xE8) { flags |= IDF_REL32; p += 4; }
        else if (op == 0xE9) { flags |= IDF_REL32; p += 4; }
        else if (op == 0xEB) { flags |= IDF_REL8;  p += 1; }
        else if ((op & 0xF0) == 0x70) { flags |= IDF_REL8; p += 1; }
        else if ((op & 0xF8) == 0xB8) {
#if defined(_M_X64) || defined(__x86_64__)
            if (rex & 0x8) { flags |= IDF_IMM32; p += 8; } else { flags |= IDF_IMM32; p += 4; }
#else
            flags |= IDF_IMM32; p += 4;
#endif
        }
        else if (op == 0x68) { flags |= IDF_IMM32; p += 4; }
        else if (op == 0x6A) { flags |= IDF_IMM8;  p += 1; }
        else if (op == 0xC2) { flags |= IDF_IMM16; p += 2; }
        else if (op == 0x80) { if (!(flags & IDF_MODRM)) return 0; flags |= IDF_IMM8; p += 1; }
        else if (op == 0x81) { if (!(flags & IDF_MODRM)) return 0; flags |= IDF_IMM32; p += 4; }
        else if (op == 0x83) { if (!(flags & IDF_MODRM)) return 0; flags |= IDF_IMM8; p += 1; }
        else if (op == 0xC7) { if (!(flags & IDF_MODRM)) return 0; flags |= IDF_IMM32; p += 4; }
        else { }
    } else {
        if ((op2 & 0xF0) == 0x80) { flags |= IDF_REL32; p += 4; }
        
    }

    size_t len = (size_t)(p - code);
    if (len == 0 || len > 15) return 0;
    if (out) { out->len = (uint8_t)len; out->flags = flags; out->opcode = op; out->opcode2 = op2; }
    return len;
}

// Jump pad planning

struct redirect_plan {
    void* target;
    void* detour;
    void* tramp;
    uint8_t patch_above;
    uint8_t n_ip;
    uint8_t old_ip[8];
    uint8_t new_ip[8];
};

[[nodiscard]] inline JN_FORCEINLINE bool make_jump_pad(redirect_plan* td) noexcept {
    uint8_t* t = (uint8_t*)td->target;
    uint8_t* u = (uint8_t*)td->tramp;
    size_t copied = 0;
    size_t u_pos = 0;
    td->n_ip = 0;

    const size_t needed = config::absJmpSize;

    // copy until we have enough bytes to place the hook
    while (copied < needed) {
        insn_info ii{};
        size_t l = decode_length_x86_64(t + copied, &ii);
        if (l == 0) return false;

        if (td->n_ip < 8) {
            td->old_ip[td->n_ip] = (uint8_t)copied;
            td->new_ip[td->n_ip] = (uint8_t)u_pos;
            td->n_ip++;
        }

        if (ii.flags & IDF_REL32) {
            intptr_t old_rel = *(int32_t*)(t + copied + (l - 4));
            uint8_t op = ii.opcode;
            uintptr_t src_next = (uintptr_t)(t + copied + l);
            uintptr_t dst = src_next + old_rel;
            {
#if defined(_M_X64) || defined(__x86_64__)
                if (op == 0xE8) {
                    if (emit::call_auto(u + u_pos, (uint8_t*)u + u_pos + l, (void*)dst)) { u_pos += l; }
                    else { u_pos += 14; }
                } else if (op == 0xE9) {
                    if (emit::jmp_auto(u + u_pos, (uint8_t*)u + u_pos + l, (void*)dst)) { u_pos += l; }
                    else { u_pos += 14; }
                } else if ((ii.flags & IDF_OPCODE_0F) && (ii.opcode2 & 0xF0) == 0x80) {
                    if (emit::jmp_auto(u + u_pos, (uint8_t*)u + u_pos + 6, (void*)dst)) { u_pos += 6; }
                    else { u_pos += 14; }
                } else {
                    emit::jmp_abs64(u + u_pos, (void*)dst); u_pos += 14;
                }
#else
                intptr_t new_rel = (intptr_t)dst - (intptr_t)(u + u_pos + l);
                std::memcpy(u + u_pos, t + copied, l);
                *(int32_t*)(u + u_pos + (l - 4)) = (int32_t)new_rel;
                u_pos += l;
#endif
            }
        }
        else if (ii.flags & IDF_REL8) {
            int8_t old_rel8 = *(int8_t*)(t + copied + (l - 1));
            uintptr_t src_next = (uintptr_t)(t + copied + l);
            uintptr_t dst = src_next + old_rel8;
#if defined(_M_X64) || defined(__x86_64__)
            uint8_t op = ii.opcode;
            if (op == 0xEB) {
                intptr_t rel32 = (intptr_t)dst - (intptr_t)((uint8_t*)u + u_pos + 5);
                if (rel32 >= INT32_MIN && rel32 <= INT32_MAX) { emit::jmp_rel32(u + u_pos, rel32); u_pos += 5; }
                else { emit::jmp_abs64(u + u_pos, (void*)dst); u_pos += 14; }
            } else if ((op & 0xF0) == 0x70) {
                // invert condition and fall through a short jump, then rel32 to target
                uint8_t cc = (uint8_t)(op - 0x70);
                uint8_t inverted = (uint8_t)(cc ^ 1);
                intptr_t rel32 = (intptr_t)dst - (intptr_t)((uint8_t*)u + u_pos + 8);
                if (rel32 >= INT32_MIN && rel32 <= INT32_MAX) {
                    uint8_t buf[8];
                    buf[0] = 0x70 | inverted; buf[1] = 0x05; // jncc +5
                    buf[2] = 0xE9; *(int32_t*)(buf + 3) = (int32_t)rel32; // jmp rel32
                    buf[7] = 0x90; // pad
                    std::memcpy(u + u_pos, buf, 8); u_pos += 8;
                } else { emit::jmp_abs64(u + u_pos, (void*)dst); u_pos += 14; }
            } else { emit::jmp_abs64(u + u_pos, (void*)dst); u_pos += 14; }
#else
            intptr_t new_rel = (intptr_t)dst - (intptr_t)(u + u_pos + l);
            std::memcpy(u + u_pos, t + copied, l);
            *(int8_t*)(u + u_pos + (l - 1)) = (int8_t)new_rel;
            u_pos += l;
#endif
        }
        else {
            std::memcpy(u + u_pos, t + copied, l);
            u_pos += l;
        }

        copied += l;
        if (u_pos + 16 > JONATHAN_CFG_TRAMPOLINE_SLOT_SIZE) return false; // ran out of space :(
    }

    // final jump back to target+copied
#if defined(_M_X64) || defined(__x86_64__)
    (void)emit::jmp_auto(u + u_pos, (uint8_t*)u + u_pos + 5, t + copied);
#else
    intptr_t rel = (intptr_t)(t + copied) - (intptr_t)(u + u_pos + 5);
    emit::jmp_rel32(u + u_pos, rel);
#endif
    return true;
}

// Hooks

struct detour_node {
    void* target;
    void* detour;
    void* relay;
    void* tramp;
    uint8_t backup[16];
    uint8_t patch[16];
    uint8_t backed_len;
    uint8_t patch_len;
    uint8_t enabled;
    uint8_t queued;
    uint8_t n_ip;
    uint8_t old_ip[8];
    uint8_t new_ip[8];
};

inline LONG g_lock = 0;
inline HANDLE g_heap = nullptr;
inline detour_node* g_hooks = nullptr;
inline uint32_t g_hooks_size = 0;
inline uint32_t g_hooks_cap  = 0;

JN_FORCEINLINE void enter_spin() noexcept {
    size_t spins = 0;
    while (InterlockedCompareExchange(&g_lock, 1, 0) != 0) {
#if defined(_MSC_VER)
        _mm_pause(); // be nice to the CPU
#endif
        if (++spins > 10000) Sleep(1); // don't burn the house down
    }
}

JN_FORCEINLINE void leave_spin() noexcept {
    InterlockedExchange(&g_lock, 0);
}

[[nodiscard]] inline JN_FORCEINLINE uint32_t find_hook(void* target) noexcept {
    for (uint32_t i = 0; i < g_hooks_size; ++i) if (g_hooks[i].target == target) return i;
    return UINT32_MAX;
}

[[nodiscard]] inline JN_FORCEINLINE detour_node* add_hook() noexcept {
    if (!g_hooks) {
        g_hooks_cap = 32;
        g_hooks = (detour_node*)HeapAlloc(g_heap, HEAP_ZERO_MEMORY, g_hooks_cap * sizeof(detour_node));
        if (!g_hooks) return nullptr;
    } else if (g_hooks_size >= g_hooks_cap) {
        uint32_t newcap = g_hooks_cap * 2;
        detour_node* p = (detour_node*)HeapReAlloc(g_heap, HEAP_ZERO_MEMORY, g_hooks, newcap * sizeof(detour_node));
        if (!p) return nullptr;
        g_hooks = p; g_hooks_cap = newcap;
    }
    return &g_hooks[g_hooks_size++];
}

// Thread freezing and IP fixups
struct frozen_threads { DWORD* ids; uint32_t size; uint32_t cap; };

[[nodiscard]] inline JN_FORCEINLINE bool enum_threads(frozen_threads* ft) noexcept {
    ft->ids = nullptr; ft->size = 0; ft->cap = 0;
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snap == INVALID_HANDLE_VALUE) return false;
    THREADENTRY32 te{}; te.dwSize = sizeof(te);
    if (Thread32First(snap, &te)) {
        do {
            if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(DWORD) &&
                te.th32OwnerProcessID == GetCurrentProcessId() && te.th32ThreadID != GetCurrentThreadId()) {
                if (ft->size >= ft->cap) {
                    uint32_t newcap = ft->cap ? ft->cap * 2 : 128;
                    DWORD* p = (DWORD*)HeapReAlloc(g_heap, 0, ft->ids, newcap * sizeof(DWORD));
                    if (!p) { CloseHandle(snap); return false; }
                    ft->ids = p; ft->cap = newcap;
                }
                ft->ids[ft->size++] = te.th32ThreadID;
            }
            te.dwSize = sizeof(te);
        } while (Thread32Next(snap, &te));
    }
    CloseHandle(snap);
    return true;
}

[[nodiscard]] inline JN_FORCEINLINE uintptr_t map_old_ip(const detour_node& h, uintptr_t ip) noexcept {
    // TODO: maybe optimize this with a hash table if we get tons of hooks
    for (uint8_t i = 0; i < h.n_ip; ++i) {
        if (ip == (uintptr_t)h.tramp + h.new_ip[i]) return (uintptr_t)h.target + h.old_ip[i];
    }
    return 0;
}

[[nodiscard]] inline JN_FORCEINLINE uintptr_t map_new_ip(const detour_node& h, uintptr_t ip) noexcept {
    for (uint8_t i = 0; i < h.n_ip; ++i) {
        if (ip == (uintptr_t)h.target + h.old_ip[i]) return (uintptr_t)h.tramp + h.new_ip[i];
    }
    return 0;
}

inline JN_FORCEINLINE void fix_ip_for(HANDLE th, uint32_t pos, bool enabling) noexcept {
    CONTEXT c{}; c.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(th, &c)) return;
#if defined(_M_X64) || defined(__x86_64__)
    DWORD64* pIP = &c.Rip;
#else
    DWORD* pIP = &c.Eip;
#endif
    if (pos == UINT32_MAX) {
        for (uint32_t i = 0; i < g_hooks_size; ++i) {
            const auto& h = g_hooks[i];
            if (enabling) {
                uintptr_t nip = map_new_ip(h, (uintptr_t)*pIP);
                if (nip) { *pIP = (decltype(*pIP))nip; SetThreadContext(th, &c); }
            } else {
                uintptr_t oip = map_old_ip(h, (uintptr_t)*pIP);
                if (oip) { *pIP = (decltype(*pIP))oip; SetThreadContext(th, &c); }
            }
        }
    } else {
        const auto& h = g_hooks[pos];
        if (enabling) {
            uintptr_t nip = map_new_ip(h, (uintptr_t)*pIP);
            if (nip) { *pIP = (decltype(*pIP))nip; SetThreadContext(th, &c); }
        } else {
            uintptr_t oip = map_old_ip(h, (uintptr_t)*pIP);
            if (oip) { *pIP = (decltype(*pIP))oip; SetThreadContext(th, &c); }
        }
    }
}

[[nodiscard]] inline JN_FORCEINLINE bool freeze_threads(uint32_t pos, bool enabling, frozen_threads* ft) noexcept {
#if !JONATHAN_CFG_FREEZE_THREADS
    ft->ids = nullptr; ft->size = ft->cap = 0; return true;
#else
    if (!enum_threads(ft)) return false;
    for (uint32_t i = 0; i < ft->size; ++i) {
        HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SET_CONTEXT, FALSE, ft->ids[i]);
        if (th) {
            SuspendThread(th);
            fix_ip_for(th, pos, enabling);
            CloseHandle(th);
        }
    }
    return true;
#endif
}

inline JN_FORCEINLINE void unfreeze_threads(const frozen_threads* ft) noexcept {
#if !JONATHAN_CFG_FREEZE_THREADS
    (void)ft;
#else
    if (!ft->ids) return;
    for (uint32_t i = 0; i < ft->size; ++i) {
        HANDLE th = OpenThread(THREAD_SUSPEND_RESUME | THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION | THREAD_SET_CONTEXT, FALSE, ft->ids[i]);
        if (th) { ResumeThread(th); CloseHandle(th); }
    }
    HeapFree(g_heap, 0, ft->ids);
#endif
}

inline JN_FORCEINLINE status activate_patch(uint32_t pos, bool enable) noexcept {
    detour_node& h = g_hooks[pos];
    const size_t hook_sz = h.patch_len;

    DWORD old{};
    if (!VirtualProtect(h.target, hook_sz, PAGE_EXECUTE_READWRITE, &old)) return status::memory_protect;
    if (enable) {
        if (hook_sz == 5) {
            *(int32_t*)((uint8_t*)h.target + 1) = *(const int32_t*)(h.patch + 1);
            *(uint8_t*)h.target = *h.patch;
        } else {
            std::memcpy(h.target, h.patch, hook_sz);
        }
    } else {
        std::memcpy(h.target, h.backup, h.backed_len);
        if (h.backed_len < hook_sz) std::memset((uint8_t*)h.target + h.backed_len, 0x90, hook_sz - h.backed_len);
    }
    VirtualProtect(h.target, hook_sz, old, &old);
    FlushInstructionCache(GetCurrentProcess(), h.target, hook_sz);
    h.enabled = enable ? 1 : 0;
    h.queued = h.enabled;
    return status::ok;
}

// API

[[nodiscard]] inline status init() noexcept {
    enter_spin();
    if (g_heap) { leave_spin(); return status::already_initialized; }
    g_heap = HeapCreate(0, 0, 0);
    if (!g_heap) { leave_spin(); return status::memory_alloc; }
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status shutdown() noexcept {
    enter_spin();
    if (!g_heap) { leave_spin(); return status::not_initialized; }
    // disable all hooks
    frozen_threads ft{};
    if (!freeze_threads(UINT32_MAX, false, &ft)) { leave_spin(); return status::memory_alloc; }
    for (uint32_t i = 0; i < g_hooks_size; ++i) {
        if (g_hooks[i].enabled) activate_patch(i, false);
        mem::free_slot(g_hooks[i].tramp);
    }
    unfreeze_threads(&ft);

    HeapFree(g_heap, 0, g_hooks);
    g_hooks = nullptr; g_hooks_size = 0; g_hooks_cap = 0;

    mem::uninitialize();
    HeapDestroy(g_heap); g_heap = nullptr;
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status create_hook(void* target, void* detour, void** original) noexcept {
    if (!g_heap) return status::not_initialized;
    if (!mem::is_executable_address(target) || !mem::is_executable_address(detour)) return status::not_executable;

    enter_spin();
    if (find_hook(target) != UINT32_MAX) { leave_spin(); return status::already_created; }

    uint8_t* slot = mem::alloc_slot(target);
    if (!slot) { leave_spin(); return status::memory_alloc; }

    redirect_plan td{}; td.target = target; td.detour = detour; td.tramp = slot; td.patch_above = 0;
    if (!make_jump_pad(&td)) {
        mem::free_slot(slot); leave_spin(); return status::unsupported_function;
    }

    detour_node* h = add_hook();
    if (!h) { mem::free_slot(slot); leave_spin(); return status::memory_alloc; }

    h->target = target; h->detour = detour; h->relay = detour; h->tramp = slot; h->enabled = 0; h->queued = 0;
    h->n_ip = td.n_ip; std::memcpy(h->old_ip, td.old_ip, sizeof(td.old_ip)); std::memcpy(h->new_ip, td.new_ip, sizeof(td.new_ip));

    // backup original bytes to be overwritten by hook
#if defined(_M_X64) || defined(__x86_64__)
    size_t patch_len = 0;
    if (emit::jmp_auto(h->patch, (uint8_t*)target + 5, detour)) patch_len = 5; else patch_len = 14;
#else
    size_t patch_len = 5; emit::jmp_rel32(h->patch, (intptr_t)detour - ((intptr_t)target + 5));
#endif
    size_t backed = 0; size_t pos = 0;
    while (backed < patch_len) {
        insn_info ii{}; size_t l = decode_length_x86_64((const uint8_t*)target + pos, &ii);
        if (!l) { leave_spin(); return status::unsupported_function; }
        if (pos + l > sizeof(h->backup)) { leave_spin(); return status::unsupported_function; }
        std::memcpy(h->backup + pos, (const uint8_t*)target + pos, l);
        pos += l; backed += l;
    }
    h->backed_len = (uint8_t)pos;
    h->patch_len = (uint8_t)patch_len;

    if (original) *original = h->tramp;
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status remove_hook(void* target) noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    uint32_t idx = find_hook(target);
    if (idx == UINT32_MAX) { leave_spin(); return status::not_created; }

    if (g_hooks[idx].enabled) {
        frozen_threads ft{}; if (!freeze_threads(idx, false, &ft)) { leave_spin(); return status::memory_alloc; }
        activate_patch(idx, false);
        unfreeze_threads(&ft);
    }

    mem::free_slot(g_hooks[idx].tramp);
    // compact vector
    if (idx != g_hooks_size - 1) g_hooks[idx] = g_hooks[g_hooks_size - 1];
    g_hooks_size--;
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status enable_hook(void* target) noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    status st = status::ok;
    if (target == nullptr) {
        frozen_threads ft{}; if (!freeze_threads(UINT32_MAX, true, &ft)) { leave_spin(); return status::memory_alloc; }
        for (uint32_t i = 0; i < g_hooks_size; ++i) if (!g_hooks[i].enabled) { st = activate_patch(i, true); if (st != status::ok) break; }
        unfreeze_threads(&ft);
    } else {
        uint32_t idx = find_hook(target);
        if (idx == UINT32_MAX) st = status::not_created;
        else if (g_hooks[idx].enabled) st = status::enabled;
        else {
            frozen_threads ft{}; if (!freeze_threads(idx, true, &ft)) { leave_spin(); return status::memory_alloc; }
            st = activate_patch(idx, true);
            unfreeze_threads(&ft);
        }
    }
    leave_spin();
    return st;
}

[[nodiscard]] inline status disable_hook(void* target) noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    status st = status::ok;
    if (target == nullptr) {
        frozen_threads ft{}; if (!freeze_threads(UINT32_MAX, false, &ft)) { leave_spin(); return status::memory_alloc; }
        for (uint32_t i = 0; i < g_hooks_size; ++i) if (g_hooks[i].enabled) { st = activate_patch(i, false); if (st != status::ok) break; }
        unfreeze_threads(&ft);
    } else {
        uint32_t idx = find_hook(target);
        if (idx == UINT32_MAX) st = status::not_created;
        else if (!g_hooks[idx].enabled) st = status::disabled;
        else {
            frozen_threads ft{}; if (!freeze_threads(idx, false, &ft)) { leave_spin(); return status::memory_alloc; }
            st = activate_patch(idx, false);
            unfreeze_threads(&ft);
        }
    }
    leave_spin();
    return st;
}

[[nodiscard]] inline status queue_enable_hook(void* target) noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    if (target == nullptr) {
        for (uint32_t i = 0; i < g_hooks_size; ++i) if (!g_hooks[i].enabled) g_hooks[i].queued = 1;
    } else {
        uint32_t idx = find_hook(target);
        if (idx == UINT32_MAX) { leave_spin(); return status::not_created; }
        if (!g_hooks[idx].enabled) g_hooks[idx].queued = 1;
    }
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status queue_disable_hook(void* target) noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    if (target == nullptr) {
        for (uint32_t i = 0; i < g_hooks_size; ++i) if (g_hooks[i].enabled) g_hooks[i].queued = 0;
    } else {
        uint32_t idx = find_hook(target);
        if (idx == UINT32_MAX) { leave_spin(); return status::not_created; }
        if (g_hooks[idx].enabled) g_hooks[idx].queued = 0;
    }
    leave_spin();
    return status::ok;
}

[[nodiscard]] inline status apply_queued() noexcept {
    if (!g_heap) return status::not_initialized;
    enter_spin();
    uint32_t first = UINT32_MAX; for (uint32_t i = 0; i < g_hooks_size; ++i) if (g_hooks[i].enabled != g_hooks[i].queued) { first = i; break; }
    if (first == UINT32_MAX) { leave_spin(); return status::ok; }
    frozen_threads ft{}; if (!freeze_threads(UINT32_MAX, true, &ft)) { leave_spin(); return status::memory_alloc; }
    status st = status::ok;
    // Batch protect per target to reduce kernel transitions
    for (uint32_t i = first; i < g_hooks_size; ++i) {
        if (g_hooks[i].enabled == g_hooks[i].queued) continue;
        detour_node& h = g_hooks[i];
        const size_t hook_sz = h.patch_len;
        DWORD old{};
        if (!VirtualProtect(h.target, hook_sz, PAGE_EXECUTE_READWRITE, &old)) { st = status::memory_protect; break; }
        const bool to_enable = (h.queued != 0);
        if (to_enable) {
            if (hook_sz == 5) {
                *(int32_t*)((uint8_t*)h.target + 1) = *(const int32_t*)(h.patch + 1);
                *(uint8_t*)h.target = *h.patch;
            } else {
                std::memcpy(h.target, h.patch, hook_sz);
            }
        } else {
            std::memcpy(h.target, h.backup, h.backed_len);
            if (h.backed_len < hook_sz) std::memset((uint8_t*)h.target + h.backed_len, 0x90, hook_sz - h.backed_len);
        }
        VirtualProtect(h.target, hook_sz, old, &old);
        FlushInstructionCache(GetCurrentProcess(), h.target, hook_sz);
        h.enabled = to_enable ? 1 : 0;
    }
    unfreeze_threads(&ft);
    leave_spin();
    return st;
}

// Convenience typed overloads
template <class T>
inline status create_hook(T* target, T* detour, T** original) {
    return create_hook((void*)target, (void*)detour, (void**)original);
}

template <class T>
inline status enable_hook(T* target) { return enable_hook((void*)target); }

template <class T>
inline status disable_hook(T* target) { return disable_hook((void*)target); }

template <class T>
inline status remove_hook(T* target) { return remove_hook((void*)target); }

} // namespace Jonathan

namespace Jonathan {

template <class Fn>
class hook_handle {
public:
    using function_type = Fn;

    hook_handle() noexcept = default;
    hook_handle(const hook_handle&) = delete;
    hook_handle& operator=(const hook_handle&) = delete;
    hook_handle(hook_handle&& other) noexcept { *this = std::move(other); }
    hook_handle& operator=(hook_handle&& other) noexcept {
        if (this == &other) return *this;
        target_ = other.target_; detour_ = other.detour_; original_ = other.original_;
        created_ = other.created_; enabled_ = other.enabled_;
        other.reset();
        return *this;
    }
    ~hook_handle() { shutdown(); }

    [[nodiscard]] bool create(Fn* target, Fn* detour) noexcept {
        if (created_) return true;
        original_ = nullptr;
        const status st = create_hook(target, detour, &original_);
        if (st != status::ok) return false;
        target_ = target; detour_ = detour; created_ = true; return true;
    }

    [[nodiscard]] bool enable() noexcept {
        if (!created_ || enabled_) return created_;
        const status st = enable_hook(target_);
        if (st != status::ok) return false;
        enabled_ = true; return true;
    }

    [[nodiscard]] bool disable() noexcept {
        if (!created_ || !enabled_) return true;
        const status st = disable_hook(target_);
        if (st != status::ok) return false;
        enabled_ = false; return true;
    }

    [[nodiscard]] bool remove() noexcept {
        if (!created_) return true;
        (void)disable();
        const status st = remove_hook(target_);
        if (st != status::ok) return false;
        reset(); return true;
    }

    [[nodiscard]] Fn* original() const noexcept { return original_; }
    [[nodiscard]] bool is_created() const noexcept { return created_; }
    [[nodiscard]] bool is_enabled() const noexcept { return enabled_; }

private:
    void reset() noexcept { target_ = nullptr; detour_ = nullptr; original_ = nullptr; created_ = false; enabled_ = false; }
    void shutdown() noexcept { (void)remove(); }

    Fn* target_{};
    Fn* detour_{};
    Fn* original_{};
    bool created_{};
    bool enabled_{};
};

} // namespace Jonathan

//======================================================================================
// MinHook compatibility layer (opt-in)
//======================================================================================

#if !defined(JONATHAN_ENABLE_MINHOOK_COMPAT)
#define JONATHAN_ENABLE_MINHOOK_COMPAT 1
#endif

#if JONATHAN_ENABLE_MINHOOK_COMPAT
extern "C" {
    typedef enum JN_STATUS__ {
        JN_UNKNOWN = -1,
        JN_OK = 0,
        JN_ERROR_ALREADY_INITIALIZED,
        JN_ERROR_NOT_INITIALIZED,
        JN_ERROR_ALREADY_CREATED,
        JN_ERROR_NOT_CREATED,
        JN_ERROR_ENABLED,
        JN_ERROR_DISABLED,
        JN_ERROR_NOT_EXECUTABLE,
        JN_ERROR_UNSUPPORTED_FUNCTION,
        JN_ERROR_MEMORY_ALLOC,
        JN_ERROR_MEMORY_PROTECT,
        JN_ERROR_MODULE_NOT_FOUND,
        JN_ERROR_FUNCTION_NOT_FOUND
    } JN_STATUS;

    static JN_FORCEINLINE JN_STATUS MH_Initialize(void) {
        using Jonathan::status; using Jonathan::init;
        status s = init();
        switch (s) {
            case status::ok: return JN_OK;
            case status::already_initialized: return JN_ERROR_ALREADY_INITIALIZED;
            case status::memory_alloc: return JN_ERROR_MEMORY_ALLOC;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_Uninitialize(void) {
        using Jonathan::status; using Jonathan::shutdown;
        status s = shutdown();
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_CreateHook(void* target, void* detour, void** original) {
        using namespace Jonathan;
        status s = create_hook(target, detour, original);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::already_created: return JN_ERROR_ALREADY_CREATED;
            case status::not_executable: return JN_ERROR_NOT_EXECUTABLE;
            case status::unsupported_function: return JN_ERROR_UNSUPPORTED_FUNCTION;
            case status::memory_alloc: return JN_ERROR_MEMORY_ALLOC;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_RemoveHook(void* target) {
        using namespace Jonathan;
        status s = remove_hook(target);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::not_created: return JN_ERROR_NOT_CREATED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_EnableHook(void* target) {
        using namespace Jonathan;
        status s = enable_hook(target);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::not_created: return JN_ERROR_NOT_CREATED;
            case status::enabled: return JN_ERROR_ENABLED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_DisableHook(void* target) {
        using namespace Jonathan;
        status s = disable_hook(target);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::not_created: return JN_ERROR_NOT_CREATED;
            case status::disabled: return JN_ERROR_DISABLED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_QueueEnableHook(void* target) {
        using namespace Jonathan;
        status s = queue_enable_hook(target);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::not_created: return JN_ERROR_NOT_CREATED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_QueueDisableHook(void* target) {
        using namespace Jonathan;
        status s = queue_disable_hook(target);
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            case status::not_created: return JN_ERROR_NOT_CREATED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE JN_STATUS MH_ApplyQueued(void) {
        using namespace Jonathan;
        status s = apply_queued();
        switch (s) {
            case status::ok: return JN_OK;
            case status::not_initialized: return JN_ERROR_NOT_INITIALIZED;
            default: return JN_UNKNOWN;
        }
    }

    static JN_FORCEINLINE const char* MH_StatusToString(JN_STATUS st) {
        switch (st) {
            case JN_OK: return "MH_OK";
            case JN_UNKNOWN: return "MH_UNKNOWN";
            case JN_ERROR_ALREADY_INITIALIZED: return "MH_ERROR_ALREADY_INITIALIZED";
            case JN_ERROR_NOT_INITIALIZED: return "MH_ERROR_NOT_INITIALIZED";
            case JN_ERROR_ALREADY_CREATED: return "MH_ERROR_ALREADY_CREATED";
            case JN_ERROR_NOT_CREATED: return "MH_ERROR_NOT_CREATED";
            case JN_ERROR_ENABLED: return "MH_ERROR_ENABLED";
            case JN_ERROR_DISABLED: return "MH_ERROR_DISABLED";
            case JN_ERROR_NOT_EXECUTABLE: return "MH_ERROR_NOT_EXECUTABLE";
            case JN_ERROR_UNSUPPORTED_FUNCTION: return "MH_ERROR_UNSUPPORTED_FUNCTION";
            case JN_ERROR_MEMORY_ALLOC: return "MH_ERROR_MEMORY_ALLOC";
            case JN_ERROR_MEMORY_PROTECT: return "MH_ERROR_MEMORY_PROTECT";
            case JN_ERROR_MODULE_NOT_FOUND: return "MH_ERROR_MODULE_NOT_FOUND";
            case JN_ERROR_FUNCTION_NOT_FOUND: return "MH_ERROR_FUNCTION_NOT_FOUND";
            default: return "(unknown)";
        }
    }
}
#endif // JONATHAN_ENABLE_MINHOOK_COMPAT


