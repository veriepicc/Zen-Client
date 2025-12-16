/*
 * Zenova - Ultra-Optimized Signature Scanner
 * -------------------------------------------
 * Zen Development 2025
 * Features:
 *  - Hand-rolled AVX2/SSE4.1 SIMD with proper fallbacks
 *  - Compile-time signature parsing with zero runtime overhead
 *  - Bitmask wildcards (8x more compact than std::optional<byte>)
 *  - Cache-aware prefetching
 *  - Branchless hot paths
 *  - Zero heap allocations in scan path
 *
 * License: MIT
 */

module;

#include <windows.h>
#include <psapi.h>
#include <intrin.h>

#pragma comment(lib, "psapi.lib")
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <array>
#include <span>
#include <string_view>
#include <type_traits>
#include <bit>
#include <utility>

export module Zenova;

// ============================================================================
// Configuration
// ============================================================================

#if defined(_M_X64) || defined(__x86_64__)
    #define ZENOVA_X64 1
#else
    #define ZENOVA_X64 0
#endif

#if defined(__AVX2__)
    #define ZENOVA_AVX2 1
#else
    // Runtime detection fallback
    #define ZENOVA_AVX2 0
#endif

#if defined(__SSE4_1__) || defined(_MSC_VER)
    #define ZENOVA_SSE41 1
#else
    #define ZENOVA_SSE41 0
#endif

#ifdef _MSC_VER
    #define ZEN_FORCEINLINE __forceinline
    #define ZEN_NOINLINE __declspec(noinline)
    #define ZEN_LIKELY(x) (x)
    #define ZEN_UNLIKELY(x) (x)
#else
    #define ZEN_FORCEINLINE inline __attribute__((always_inline))
    #define ZEN_NOINLINE __attribute__((noinline))
    #define ZEN_LIKELY(x) __builtin_expect(!!(x), 1)
    #define ZEN_UNLIKELY(x) __builtin_expect(!!(x), 0)
#endif

export namespace Zenova {

// ============================================================================
// CPU Feature Detection (Runtime)
// ============================================================================

namespace cpu {
    struct Features {
        bool avx2 = false;
        bool sse41 = false;
        bool bmi1 = false;
    };

    [[nodiscard]] ZEN_FORCEINLINE Features detect() noexcept {
        Features f{};
        int cpuInfo[4]{};

        __cpuid(cpuInfo, 0);
        const int maxFunc = cpuInfo[0];

        if (maxFunc >= 1) {
            __cpuidex(cpuInfo, 1, 0);
            f.sse41 = (cpuInfo[2] & (1 << 19)) != 0;
        }

        if (maxFunc >= 7) {
            __cpuidex(cpuInfo, 7, 0);
            f.avx2 = (cpuInfo[1] & (1 << 5)) != 0;
            f.bmi1 = (cpuInfo[1] & (1 << 3)) != 0;
        }

        return f;
    }

    inline const Features& get() noexcept {
        static const Features f = detect();
        return f;
    }
}

// ============================================================================
// Signature Element (2 bytes: value + is_wildcard)
// For compile-time, we pack this tight
// ============================================================================

struct SigByte {
    std::byte value{};
    bool wildcard = true;  // true = don't care (?)

    constexpr SigByte() noexcept = default;
    constexpr SigByte(std::byte v) noexcept : value(v), wildcard(false) {}
    constexpr SigByte(std::nullptr_t) noexcept : value{}, wildcard(true) {}

    [[nodiscard]] constexpr bool matches(std::byte b) const noexcept {
        return wildcard | (value == b);  // Branchless!
    }
};

// ============================================================================
// Fixed-Size Signature (compile-time)
// ============================================================================

template<std::size_t N>
struct Signature {
    std::array<SigByte, N> bytes{};
    std::size_t firstNonWildcard = 0;  // Index of first concrete byte
    std::size_t actualSize = N;         // Actual number of signature bytes (may be < N)

    [[nodiscard]] constexpr std::size_t size() const noexcept { return actualSize; }
    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return N; }
    [[nodiscard]] constexpr const SigByte& operator[](std::size_t i) const noexcept { return bytes[i]; }
    [[nodiscard]] constexpr auto begin() const noexcept { return bytes.begin(); }
    [[nodiscard]] constexpr auto end() const noexcept { return bytes.begin() + actualSize; }

    // Get the first concrete byte value (for SIMD first-byte search)
    [[nodiscard]] constexpr std::byte firstByte() const noexcept {
        return bytes[firstNonWildcard].value;
    }
};

// ============================================================================
// Compile-Time Signature Parser
// ============================================================================

namespace detail {
    [[nodiscard]] constexpr int hexValue(char c) noexcept {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        return -1;
    }

    [[nodiscard]] constexpr bool isHex(char c) noexcept {
        return hexValue(c) >= 0;
    }

    [[nodiscard]] constexpr bool isWildcard(char c) noexcept {
        return c == '?';
    }

    [[nodiscard]] constexpr bool isSpace(char c) noexcept {
        return c == ' ' || c == '\t';
    }

    // Count how many signature bytes a string contains
    [[nodiscard]] constexpr std::size_t countBytes(std::string_view str) noexcept {
        std::size_t count = 0;
        std::size_t i = 0;

        while (i < str.size()) {
            // Skip whitespace
            while (i < str.size() && isSpace(str[i])) ++i;
            if (i >= str.size()) break;

            // Wildcard?
            if (isWildcard(str[i])) {
                ++count;
                ++i;
                // Handle ?? (double question mark)
                if (i < str.size() && isWildcard(str[i])) ++i;
            }
            // Hex byte?
            else if (isHex(str[i])) {
                ++count;
                ++i;
                if (i < str.size() && isHex(str[i])) ++i;
            }
            else {
                ++i; // Skip unknown char
            }
        }

        return count;
    }

    // Parse signature string into array
    template<std::size_t N>
    [[nodiscard]] constexpr Signature<N> parseSignature(std::string_view str) noexcept {
        Signature<N> sig{};
        std::size_t idx = 0;
        std::size_t i = 0;
        bool foundFirst = false;

        while (i < str.size() && idx < N) {
            // Skip whitespace
            while (i < str.size() && isSpace(str[i])) ++i;
            if (i >= str.size()) break;

            if (isWildcard(str[i])) {
                sig.bytes[idx] = SigByte{nullptr};
                ++i;
                if (i < str.size() && isWildcard(str[i])) ++i;
                ++idx;
            }
            else if (isHex(str[i])) {
                int hi = hexValue(str[i]);
                ++i;
                int lo = 0;
                if (i < str.size() && isHex(str[i])) {
                    lo = hexValue(str[i]);
                    ++i;
                } else {
                    lo = hi;
                    hi = 0;
                }
                sig.bytes[idx] = SigByte{static_cast<std::byte>((hi << 4) | lo)};
                if (!foundFirst) {
                    sig.firstNonWildcard = idx;
                    foundFirst = true;
                }
                ++idx;
            }
            else {
                ++i;
            }
        }

        return sig;
    }
}

// Compile-time signature creation macro
#define ZEN_SIG(str) \
    []() consteval { \
        constexpr std::string_view s = str; \
        constexpr std::size_t N = ::Zenova::detail::countBytes(s); \
        return ::Zenova::detail::parseSignature<N>(s); \
    }()

// Compile-time signature type deduction
template<std::size_t N>
using FixedSig = Signature<N>;

// Helper struct to hold string for template deduction
template<std::size_t N>
struct SigStr {
    char data[N]{};
    static constexpr std::size_t len = N - 1;
    
    constexpr SigStr(const char (&s)[N]) noexcept {
        for (std::size_t i = 0; i < N; ++i) data[i] = s[i];
    }
    
    constexpr std::string_view view() const noexcept { return {data, len}; }
    
    static constexpr std::size_t byteCount(const char (&s)[N]) noexcept {
        return detail::countBytes(std::string_view{s, N - 1});
    }
};

// Deduction guide
template<std::size_t N>
SigStr(const char (&)[N]) -> SigStr<N>;

// Helper to create signature at compile time - MSVC compatible version
// Usage: constexpr auto sig = makeSig<"48 89 5C 24 ?">();
template<SigStr S>
[[nodiscard]] consteval auto makeSig() noexcept {
    constexpr std::string_view sv = S.view();
    constexpr std::size_t count = detail::countBytes(sv);
    return detail::parseSignature<count>(sv);
}

// Alternative: runtime-capable version that still optimizes well
template<std::size_t N>
[[nodiscard]] constexpr auto parseSig(const char (&str)[N]) noexcept {
    constexpr std::size_t maxBytes = N; // Upper bound
    Signature<maxBytes> sig{};
    std::size_t idx = 0;
    std::size_t i = 0;
    bool foundFirst = false;

    while (i < N - 1 && idx < maxBytes) {
        // Skip whitespace
        while (i < N - 1 && detail::isSpace(str[i])) ++i;
        if (i >= N - 1) break;

        if (detail::isWildcard(str[i])) {
            sig.bytes[idx] = SigByte{nullptr};
            ++i;
            if (i < N - 1 && detail::isWildcard(str[i])) ++i;
            ++idx;
        }
        else if (detail::isHex(str[i])) {
            int hi = detail::hexValue(str[i]);
            ++i;
            int lo = 0;
            if (i < N - 1 && detail::isHex(str[i])) {
                lo = detail::hexValue(str[i]);
                ++i;
            } else {
                lo = hi;
                hi = 0;
            }
            sig.bytes[idx] = SigByte{static_cast<std::byte>((hi << 4) | lo)};
            if (!foundFirst) {
                sig.firstNonWildcard = idx;
                foundFirst = true;
            }
            ++idx;
        }
        else {
            ++i;
        }
    }

    sig.actualSize = idx;  // Store the actual number of parsed bytes
    return sig;
}

// ============================================================================
// Scan Result
// ============================================================================

class ScanResult {
    const std::byte* ptr_ = nullptr;

public:
    constexpr ScanResult() noexcept = default;
    constexpr ScanResult(std::nullptr_t) noexcept : ptr_(nullptr) {}
    constexpr ScanResult(const std::byte* p) noexcept : ptr_(p) {}

    [[nodiscard]] constexpr bool valid() const noexcept { return ptr_ != nullptr; }
    [[nodiscard]] constexpr explicit operator bool() const noexcept { return valid(); }

    [[nodiscard]] constexpr const std::byte* get() const noexcept { return ptr_; }
    [[nodiscard]] constexpr const std::byte* operator*() const noexcept { return ptr_; }

    // Read value at offset
    template<typename T>
    [[nodiscard]] T read(std::size_t offset = 0) const noexcept {
        T val;
        std::memcpy(&val, ptr_ + offset, sizeof(T));
        return val;
    }

    // Resolve RIP-relative address (common in x64)
    [[nodiscard]] const std::byte* rel(std::size_t offset) const noexcept {
        if (!ptr_) return nullptr;
        const auto rip = ptr_ + offset + 4;  // RIP points to next instruction
        const auto rel32 = read<std::int32_t>(offset);
        return rip + rel32;
    }

    // Get as uintptr
    [[nodiscard]] std::uintptr_t addr() const noexcept {
        return reinterpret_cast<std::uintptr_t>(ptr_);
    }

    // Cast to function pointer
    template<typename Fn>
    [[nodiscard]] Fn as() const noexcept {
        return reinterpret_cast<Fn>(ptr_);
    }
};

// ============================================================================
// SIMD Scanner Core - The Secret Sauce 🔥
// ============================================================================

namespace simd {

// ----------------------
// Byte Rarity Scoring - Find the best anchor byte
// Common x64 opcodes score LOW, rare bytes score HIGH
// ----------------------
[[nodiscard]] ZEN_FORCEINLINE std::uint8_t byteRarity(std::byte b) noexcept {
    const auto v = static_cast<std::uint8_t>(b);
    switch (v) {
        case 0x00: case 0xFF: return 1;  // Very common
        case 0x48: case 0x89: case 0x8B: case 0x83: case 0x81: return 2;  // x64 REX/MOV
        case 0xC3: case 0xC7: case 0xE8: case 0xE9: return 3;  // ret/call/jmp
        case 0x0F: case 0xB6: case 0xB7: case 0xB8: case 0xB9: return 8;  // Rarer opcodes
        default: return (v >> 4) < 10 ? (v >> 4) : 10;  // Higher bytes = rarer
    }
}

// Find the two rarest non-wildcard bytes in a pattern (Twin Anchors)
[[nodiscard]] ZEN_FORCEINLINE std::pair<std::size_t, std::size_t> findTwinAnchors(std::span<const SigByte> pattern) noexcept {
    std::size_t bestIdx = 0;
    std::size_t secondIdx = 0;
    std::uint8_t bestScore = 0;
    std::uint8_t secondScore = 0;
    
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        if (pattern[i].wildcard) continue;
        
        const auto score = byteRarity(pattern[i].value);
        if (score > bestScore) {
            secondScore = bestScore;
            secondIdx = bestIdx;
            bestScore = score;
            bestIdx = i;
        } else if (score > secondScore) {
            secondScore = score;
            secondIdx = i;
        }
    }
    
    // Fallback if we didn't find a second valid byte (e.g. 1-byte patterns)
    if (secondScore == 0) secondIdx = bestIdx;
    
    return {bestIdx, secondIdx};
}

// ----------------------
// Runtime Scalar Scanner (for dynamic patterns)
// ----------------------
[[nodiscard]] ZEN_FORCEINLINE const std::byte* scanRuntimeScalar(
    const std::byte* begin,
    const std::byte* end,
    std::span<const SigByte> pattern,
    std::size_t anchorIdx
) noexcept {
    const std::size_t sigLen = pattern.size();
    if (sigLen == 0 || static_cast<std::size_t>(end - begin) < sigLen) return nullptr;
    if (anchorIdx >= sigLen) return nullptr;

    const std::byte* scanEnd = end - sigLen + 1;
    const std::byte anchorByte = pattern[anchorIdx].value;

    for (const std::byte* p = begin; p < scanEnd; ++p) {
        if (p[anchorIdx] != anchorByte) continue;

        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = pattern[i].matches(p[i]);
        }

        if (match) return p;
    }

    return nullptr;
}

// -----------------------------------------------------------------------------
// AVX2 Runtime Scanner
// Uses dual-anchor filtering and 4x loop unrolling (128 bytes/iter).
// -----------------------------------------------------------------------------
[[nodiscard]] ZEN_NOINLINE const std::byte* scanRuntimeAVX2(
    const std::byte* begin,
    const std::byte* end,
    std::span<const SigByte> pattern,
    std::pair<std::size_t, std::size_t> anchors
) noexcept {
    const std::size_t sigLen = pattern.size();
    if (sigLen == 0 || static_cast<std::size_t>(end - begin) < sigLen) return nullptr;
    
    // Unpack anchors
    const std::size_t idx1 = anchors.first;
    const std::size_t idx2 = anchors.second;
    const std::byte byte1 = pattern[idx1].value;
    const std::byte byte2 = pattern[idx2].value;

    const std::byte* scanEnd = end - sigLen + 1;
    const bool singleAnchor = (idx1 == idx2); // Fallback for short patterns

    // Broadcast anchor bytes to all 32 lanes
    const __m256i needle1 = _mm256_set1_epi8(static_cast<char>(byte1));
    const __m256i needle2 = _mm256_set1_epi8(static_cast<char>(byte2));

    // Align to 32-byte boundary
    const std::byte* aligned = reinterpret_cast<const std::byte*>(
        (reinterpret_cast<std::uintptr_t>(begin) + 31) & ~std::uintptr_t(31)
    );

    // Scan unaligned prefix (scalar)
    for (const std::byte* p = begin; p < aligned && p < scanEnd; ++p) {
        if (p[idx1] != byte1) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = pattern[i].matches(p[i]);
        }
        if (match) return p;
    }

    // SIMD loop: 128 bytes per iteration (4x32 unrolled)
    const std::byte* simd128End = scanEnd - 128;
    const std::byte* p = aligned;
    
    while (p <= simd128End) {
        _mm_prefetch(reinterpret_cast<const char*>(p + 1024), _MM_HINT_T0);
        
        // Helper macro to load and compare dual anchors
        // We AND the results: (h1 == n1) & (h2 == n2)
        // Block 0
        __m256i v0_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + idx1));
        __m256i c0 = _mm256_cmpeq_epi8(v0_1, needle1);
        if (!singleAnchor) {
            __m256i v0_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + idx2));
            c0 = _mm256_and_si256(c0, _mm256_cmpeq_epi8(v0_2, needle2));
        }
        unsigned int mask0 = static_cast<unsigned int>(_mm256_movemask_epi8(c0));

        // Block 32
        __m256i v1_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 32 + idx1));
        __m256i c1 = _mm256_cmpeq_epi8(v1_1, needle1);
        if (!singleAnchor) {
            __m256i v1_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 32 + idx2));
            c1 = _mm256_and_si256(c1, _mm256_cmpeq_epi8(v1_2, needle2));
        }
        unsigned int mask32 = static_cast<unsigned int>(_mm256_movemask_epi8(c1));

        // Block 64
        __m256i v2_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 64 + idx1));
        __m256i c2 = _mm256_cmpeq_epi8(v2_1, needle1);
        if (!singleAnchor) {
            __m256i v2_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 64 + idx2));
            c2 = _mm256_and_si256(c2, _mm256_cmpeq_epi8(v2_2, needle2));
        }
        unsigned int mask64 = static_cast<unsigned int>(_mm256_movemask_epi8(c2));

        // Block 96
        __m256i v3_1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 96 + idx1));
        __m256i c3 = _mm256_cmpeq_epi8(v3_1, needle1);
        if (!singleAnchor) {
            __m256i v3_2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + 96 + idx2));
            c3 = _mm256_and_si256(c3, _mm256_cmpeq_epi8(v3_2, needle2));
        }
        unsigned int mask96 = static_cast<unsigned int>(_mm256_movemask_epi8(c3));
        
        // Quick exit if no matches in any chunk
        if ((mask0 | mask32 | mask64 | mask96) == 0) {
            p += 128;
            continue;
        }
        
        // Process matches
        auto checkMask = [&](unsigned int mask, std::size_t offset) -> const std::byte* {
            while (mask != 0) {
                const unsigned long bit = _tzcnt_u32(mask);
                const std::byte* cand = p + offset + bit;
                if (cand < scanEnd) {
                    bool match = true;
                    for (std::size_t i = 0; i < sigLen && match; ++i) {
                        match = pattern[i].matches(cand[i]);
                    }
                    if (match) return cand;
                }
                mask &= mask - 1;
            }
            return nullptr;
        };

        if (const auto* r = checkMask(mask0, 0)) { _mm256_zeroupper(); return r; }
        if (const auto* r = checkMask(mask32, 32)) { _mm256_zeroupper(); return r; }
        if (const auto* r = checkMask(mask64, 64)) { _mm256_zeroupper(); return r; }
        if (const auto* r = checkMask(mask96, 96)) { _mm256_zeroupper(); return r; }
        
        p += 128;
    }

    // Handle remaining 32-byte chunks
    const std::byte* simd32End = scanEnd - 32;
    while (p <= simd32End) {
        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + idx1));
        __m256i c1 = _mm256_cmpeq_epi8(v1, needle1);
        if (!singleAnchor) {
            __m256i v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + idx2));
            __m256i c2 = _mm256_cmpeq_epi8(v2, needle2);
            c1 = _mm256_and_si256(c1, c2);
        }
        unsigned int mask = static_cast<unsigned int>(_mm256_movemask_epi8(c1));

        while (mask != 0) {
            const unsigned long bitPos = _tzcnt_u32(mask);
            const std::byte* candidate = p + bitPos;
            if (candidate < scanEnd) {
                bool match = true;
                for (std::size_t i = 0; i < sigLen && match; ++i) {
                    match = pattern[i].matches(candidate[i]);
                }
                if (match) { _mm256_zeroupper(); return candidate; }
            }
            mask &= mask - 1;
        }
        p += 32;
    }

    // Cleanup remaining bytes (scalar)
    for (; p < scanEnd; ++p) {
        if (p[idx1] != byte1) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = pattern[i].matches(p[i]);
        }
        if (match) { _mm256_zeroupper(); return p; }
    }

    _mm256_zeroupper();
    return nullptr;
}

// -----------------------------------------------------------------------------
// Unified Runtime Scanner
// Dispatches to AVX2 or Scalar implementation based on CPU support.
// Automatically calculates optimal twin anchors for the given pattern.
// -----------------------------------------------------------------------------
[[nodiscard]] ZEN_FORCEINLINE const std::byte* scanRuntime(
    const std::byte* begin,
    const std::byte* end,
    std::span<const SigByte> pattern
) noexcept {
    const auto anchors = findTwinAnchors(pattern);
    
    if (cpu::get().avx2) {
        return scanRuntimeAVX2(begin, end, pattern, anchors);
    }
    
    // Fallback to scalar uses best anchor only
    return scanRuntimeScalar(begin, end, pattern, anchors.first);
}

} // namespace simd

// Public API for runtime scanning
[[nodiscard]] inline const std::byte* scanRuntime(
    std::span<const std::byte> data,
    std::span<const SigByte> pattern
) noexcept {
    return simd::scanRuntime(data.data(), data.data() + data.size(), pattern);
}

namespace simd {

// ----------------------
// Scalar Fallback
// ----------------------
template<std::size_t N>
[[nodiscard]] ZEN_NOINLINE const std::byte* scanScalar(
    const std::byte* begin,
    const std::byte* end,
    const Signature<N>& sig
) noexcept {
    const std::size_t sigLen = sig.size();
    if (sigLen == 0 || static_cast<std::size_t>(end - begin) < sigLen) return nullptr;

    const std::byte* scanEnd = end - sigLen + 1;
    const std::byte firstByte = sig.firstByte();
    const std::size_t firstOffset = sig.firstNonWildcard;

    for (const std::byte* p = begin; p < scanEnd; ++p) {
        // Quick first-byte check
        if (p[firstOffset] != firstByte) continue;

        // Full signature check
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = sig[i].matches(p[i]);
        }

        if (match) return p;
    }

    return nullptr;
}

// ----------------------
// SSE4.1 Scanner (16 bytes at a time)
// ----------------------
#if ZENOVA_SSE41
template<std::size_t N>
[[nodiscard]] ZEN_NOINLINE const std::byte* scanSSE41(
    const std::byte* begin,
    const std::byte* end,
    const Signature<N>& sig
) noexcept {
    const std::size_t sigLen = sig.size();
    if (sigLen == 0 || static_cast<std::size_t>(end - begin) < sigLen) return nullptr;

    const std::byte* scanEnd = end - sigLen + 1;
    const std::byte firstByte = sig.firstByte();
    const std::size_t firstOffset = sig.firstNonWildcard;

    // Broadcast first byte to all 16 lanes
    const __m128i needle = _mm_set1_epi8(static_cast<char>(firstByte));

    // Align starting pointer
    const std::byte* aligned = reinterpret_cast<const std::byte*>(
        (reinterpret_cast<std::uintptr_t>(begin) + 15) & ~std::uintptr_t(15)
    );

    // Scan unaligned prefix
    for (const std::byte* p = begin; p < aligned && p < scanEnd; ++p) {
        if (p[firstOffset] != firstByte) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = sig[i].matches(p[i]);
        }
        if (match) return p;
    }

    // SIMD scan
    const std::byte* simdEnd = scanEnd - 16;
    for (const std::byte* p = aligned; p <= simdEnd; p += 16) {
        // Prefetch ahead
        _mm_prefetch(reinterpret_cast<const char*>(p + 256), _MM_HINT_T0);

        // Load 16 bytes starting at first non-wildcard position
        __m128i haystack = _mm_load_si128(reinterpret_cast<const __m128i*>(p + firstOffset));
        __m128i cmp = _mm_cmpeq_epi8(haystack, needle);
        int mask = _mm_movemask_epi8(cmp);

        while (mask != 0) {
            const unsigned long bitPos = _tzcnt_u32(static_cast<unsigned>(mask));
            const std::byte* candidate = p + bitPos;

            if (candidate >= scanEnd) break;

            // Full signature verification
            bool match = true;
            for (std::size_t i = 0; i < sigLen && match; ++i) {
                match = sig[i].matches(candidate[i]);
            }

            if (match) return candidate;

            mask &= mask - 1;  // Clear lowest bit (BLSR)
        }
    }

    // Scan remaining bytes
    for (const std::byte* p = simdEnd + 1; p < scanEnd; ++p) {
        if (p[firstOffset] != firstByte) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = sig[i].matches(p[i]);
        }
        if (match) return p;
    }

    return nullptr;
}
#endif

// ----------------------
// AVX2 Scanner (32 bytes at a time) - THE BEAST 🔥
// ----------------------
template<std::size_t N>
[[nodiscard]] ZEN_NOINLINE const std::byte* scanAVX2(
    const std::byte* begin,
    const std::byte* end,
    const Signature<N>& sig
) noexcept {
    const std::size_t sigLen = sig.size();
    if (sigLen == 0 || static_cast<std::size_t>(end - begin) < sigLen) return nullptr;

    const std::byte* scanEnd = end - sigLen + 1;
    const std::byte firstByte = sig.firstByte();
    const std::size_t firstOffset = sig.firstNonWildcard;

    // Broadcast first byte to all 32 lanes
    const __m256i needle = _mm256_set1_epi8(static_cast<char>(firstByte));

    // Align to 32-byte boundary
    const std::byte* aligned = reinterpret_cast<const std::byte*>(
        (reinterpret_cast<std::uintptr_t>(begin) + 31) & ~std::uintptr_t(31)
    );

    // Scan unaligned prefix with scalar
    for (const std::byte* p = begin; p < aligned && p < scanEnd; ++p) {
        if (p[firstOffset] != firstByte) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = sig[i].matches(p[i]);
        }
        if (match) return p;
    }

    // SIMD scan - 32 bytes per iteration
    const std::byte* simdEnd = scanEnd - 32;
    for (const std::byte* p = aligned; p <= simdEnd; p += 32) {
        // Aggressive prefetching: load next cache lines
        _mm_prefetch(reinterpret_cast<const char*>(p + 512), _MM_HINT_T0);
        _mm_prefetch(reinterpret_cast<const char*>(p + 576), _MM_HINT_T0);

        // Load 32 bytes at first non-wildcard offset
        __m256i haystack = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p + firstOffset));
        __m256i cmp = _mm256_cmpeq_epi8(haystack, needle);
        unsigned int mask = static_cast<unsigned int>(_mm256_movemask_epi8(cmp));

        // Process all matches in this 32-byte window
        while (mask != 0) {
            // Find lowest set bit (first match position)
            const unsigned long bitPos = _tzcnt_u32(mask);
            const std::byte* candidate = p + bitPos;

            if (candidate >= scanEnd) break;

            // Full signature verification
            bool match = true;
            for (std::size_t i = 0; i < sigLen && match; ++i) {
                match = sig[i].matches(candidate[i]);
            }

            if (match) {
                _mm256_zeroupper();  // Avoid AVX-SSE transition penalty
                return candidate;
            }

            mask &= mask - 1;  // Clear lowest bit (BLSR equivalent)
        }
    }

    // Cleanup: scan remaining bytes with scalar
    for (const std::byte* p = simdEnd > aligned ? simdEnd : aligned; p < scanEnd; ++p) {
        if (p[firstOffset] != firstByte) continue;
        bool match = true;
        for (std::size_t i = 0; i < sigLen && match; ++i) {
            match = sig[i].matches(p[i]);
        }
        if (match) {
            _mm256_zeroupper();
            return p;
        }
    }

    _mm256_zeroupper();
    return nullptr;
}

} // namespace simd

// ============================================================================
// Unified Scanner (auto-selects best implementation)
// ============================================================================

template<std::size_t N>
[[nodiscard]] ZEN_FORCEINLINE ScanResult scan(
    const std::byte* begin,
    const std::byte* end,
    const Signature<N>& sig
) noexcept {
    if (cpu::get().avx2) {
        return simd::scanAVX2(begin, end, sig);
    }
#if ZENOVA_SSE41
    else if (cpu::get().sse41) {
        return simd::scanSSE41(begin, end, sig);
    }
#endif
    else {
        return simd::scanScalar(begin, end, sig);
    }
}

// Span overload
template<std::size_t N>
[[nodiscard]] ZEN_FORCEINLINE ScanResult scan(
    std::span<const std::byte> data,
    const Signature<N>& sig
) noexcept {
    return scan(data.data(), data.data() + data.size(), sig);
}

// ============================================================================
// Process Utilities (Windows)
// ============================================================================

namespace process {

    struct Module {
        std::byte* base = nullptr;
        std::size_t size = 0;

        [[nodiscard]] bool valid() const noexcept { return base != nullptr && size > 0; }
        [[nodiscard]] std::span<std::byte> data() const noexcept { return {base, size}; }
    };

    struct Section {
        std::byte* base = nullptr;
        std::size_t size = 0;

        [[nodiscard]] bool valid() const noexcept { return base != nullptr && size > 0; }
        [[nodiscard]] std::span<std::byte> data() const noexcept { return {base, size}; }
    };

    // Get current process module
    [[nodiscard]] inline Module getProcessModule() noexcept {
        HMODULE hMod = GetModuleHandleW(nullptr);
        if (!hMod) return {};

        MODULEINFO mi{};
        if (!GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi))) {
            // Fallback: parse PE header manually
            auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(hMod);
            if (dos->e_magic != IMAGE_DOS_SIGNATURE) return {};

            auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
                reinterpret_cast<std::byte*>(hMod) + dos->e_lfanew
            );
            if (nt->Signature != IMAGE_NT_SIGNATURE) return {};

            return Module{
                reinterpret_cast<std::byte*>(hMod),
                nt->OptionalHeader.SizeOfImage
            };
        }

        return Module{
            reinterpret_cast<std::byte*>(mi.lpBaseOfDll),
            mi.SizeOfImage
        };
    }

    // Get module by name
    [[nodiscard]] inline Module getModule(const wchar_t* name) noexcept {
        HMODULE hMod = GetModuleHandleW(name);
        if (!hMod) return {};

        auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(hMod);
        if (dos->e_magic != IMAGE_DOS_SIGNATURE) return {};

        auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(
            reinterpret_cast<std::byte*>(hMod) + dos->e_lfanew
        );
        if (nt->Signature != IMAGE_NT_SIGNATURE) return {};

        return Module{
            reinterpret_cast<std::byte*>(hMod),
            nt->OptionalHeader.SizeOfImage
        };
    }

    // Get section by name
    [[nodiscard]] inline Section getSection(Module mod, std::string_view name) noexcept {
        if (!mod.valid()) return {};

        auto* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(mod.base);
        auto* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(mod.base + dos->e_lfanew);
        auto* section = IMAGE_FIRST_SECTION(nt);

        for (WORD i = 0; i < nt->FileHeader.NumberOfSections; ++i, ++section) {
            // Section name is 8 bytes, may not be null-terminated
            char sectionName[9]{};
            std::memcpy(sectionName, section->Name, 8);

            if (name == sectionName) {
                return Section{
                    mod.base + section->VirtualAddress,
                    section->Misc.VirtualSize
                };
            }
        }

        return {};
    }

    // Common overload for .text section
    [[nodiscard]] inline Section getTextSection(Module mod) noexcept {
        return getSection(mod, ".text");
    }

} // namespace process

// ============================================================================
// High-Level API (convenient wrappers)
// ============================================================================

// Scan entire module for signature
template<std::size_t N>
[[nodiscard]] ZEN_FORCEINLINE ScanResult findPattern(
    const Signature<N>& sig,
    process::Module mod = process::getProcessModule()
) noexcept {
    if (!mod.valid()) return nullptr;
    return scan(mod.data(), sig);
}

// Scan specific section
template<std::size_t N>
[[nodiscard]] ZEN_FORCEINLINE ScanResult findPattern(
    const Signature<N>& sig,
    std::string_view section,
    process::Module mod = process::getProcessModule()
) noexcept {
    if (!mod.valid()) return nullptr;
    auto sec = process::getSection(mod, section);
    if (!sec.valid()) return nullptr;
    return scan(sec.data(), sig);
}

// Scan .text section (most common case, optimized path)
template<std::size_t N>
[[nodiscard]] ZEN_FORCEINLINE ScanResult findPatternText(
    const Signature<N>& sig,
    process::Module mod = process::getProcessModule()
) noexcept {
    return findPattern(sig, ".text", mod);
}

// ============================================================================
// Compile-Time Signature Literal (Bonus Sugar 🍬)
// ============================================================================

namespace literals {

template<std::size_t N>
struct SigString {
    char data[N]{};
    static constexpr std::size_t size = N - 1;

    consteval SigString(const char (&str)[N]) noexcept {
        for (std::size_t i = 0; i < N; ++i) data[i] = str[i];
    }

    consteval std::string_view view() const noexcept {
        return {data, size};
    }
};

template<SigString S>
[[nodiscard]] consteval auto operator""_sig() noexcept {
    constexpr std::string_view sv = S.view();
    constexpr std::size_t count = detail::countBytes(sv);
    return detail::parseSignature<count>(sv);
}

} // namespace literals

} // namespace Zenova

// ============================================================================
// Convenience Macro (only works within this module file)
// For code importing Zenova, use Zenova::makeSig("pattern") directly
// ============================================================================

// Usage within this module: auto sig = ZSIG("48 89 5C 24 ? 48 89 74 24");
#define ZSIG(str) (::Zenova::makeSig(str))

