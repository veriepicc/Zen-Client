module;
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <xaudio2.h>
#include <wrl/client.h>

#include <cstdint>
#include <cstdio>
#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <mutex>
#include <fstream>
#include <iostream>
#include <cmath>

export module SoundPlayer;

using Microsoft::WRL::ComPtr;

namespace Detail::Audio
{
    struct VoiceContext : public IXAudio2VoiceCallback
    {
        void OnVoiceProcessingPassStart(UINT32 /*bytesRequired*/) noexcept override {}
        void OnVoiceProcessingPassEnd() noexcept override {}
        void OnStreamEnd() noexcept override {}
        void OnBufferStart(void* /*pBufferContext*/) noexcept override {}
        void OnBufferEnd(void* pBufferContext) noexcept override
        {
            // Avoid destroying the voice or deleting the callback from within the callback thread
            // to prevent re-entrancy crashes in XAudio2. Cleanup can be deferred if needed.
            (void)pBufferContext;
        }
        void OnLoopEnd(void* /*pBufferContext*/) noexcept override {}
        void OnVoiceError(void* /*pBufferContext*/, HRESULT /*Error*/) noexcept override {}
    };

    struct WavHeader
    {
        // Minimal RIFF/WAVE PCM header parsing
        std::uint16_t audioFormat { 1 }; // 1 = PCM
        std::uint16_t numChannels { 0 };
        std::uint32_t sampleRate { 0 };
        std::uint16_t bitsPerSample { 0 };
        std::uint32_t dataOffset { 0 };
        std::uint32_t dataSize { 0 };
    };

    inline bool parseWavPCM(std::ifstream& in, WavHeader& out)
    {
        auto readU32 = [&in]() {
            std::uint32_t v = 0; in.read(reinterpret_cast<char*>(&v), 4); return v; };
        auto readU16 = [&in]() {
            std::uint16_t v = 0; in.read(reinterpret_cast<char*>(&v), 2); return v; };

        char riff[4]; in.read(riff, 4); if (!in || std::string_view(riff,4) != "RIFF") return false;
        (void)readU32(); // total size
        char wave[4]; in.read(wave, 4); if (!in || std::string_view(wave,4) != "WAVE") return false;

        bool fmtFound = false, dataFound = false;
        std::uint32_t dataOffset = 0, dataSize = 0;
        std::uint16_t audioFmt = 0, numCh = 0, bps = 0; std::uint32_t rate = 0;

        while (in && (!fmtFound || !dataFound))
        {
            char chunkIdC[4]; if (!in.read(chunkIdC, 4)) break;
            std::string chunkId(chunkIdC, 4);
            std::uint32_t chunkSize = readU32();
            const std::streampos chunkStart = in.tellg();

            if (chunkId == "fmt ")
            {
                audioFmt = readU16();
                numCh = readU16();
                rate = readU32();
                (void)readU32(); // byteRate
                (void)readU16(); // blockAlign
                bps = readU16();
                // Skip any extra fmt bytes
                const std::int64_t consumed = 16; // we read standard 16 bytes
                const std::int64_t remain = static_cast<std::int64_t>(chunkSize) - consumed;
                if (remain > 0) in.seekg(remain, std::ios::cur);
                fmtFound = true;
            }
            else if (chunkId == "data")
            {
                dataOffset = static_cast<std::uint32_t>(in.tellg());
                dataSize = chunkSize;
                in.seekg(chunkSize, std::ios::cur);
                dataFound = true;
            }
            else
            {
                in.seekg(chunkSize, std::ios::cur);
            }

            // Chunk sizes are padded to 2 bytes
            if (chunkSize & 1) in.seekg(1, std::ios::cur);

            // Safety: avoid infinite loop
            if (!in) break;
            const auto at = in.tellg();
            if (at == chunkStart) break;
        }

        if (!fmtFound || !dataFound) return false;
        if (audioFmt != 1) return false; // PCM only
        if (!(bps == 8 || bps == 16 || bps == 32)) return false;
        if (numCh == 0 || rate == 0) return false;

        out.audioFormat = audioFmt;
        out.numChannels = numCh;
        out.sampleRate = rate;
        out.bitsPerSample = bps;
        out.dataOffset = dataOffset;
        out.dataSize = dataSize;
        return true;
    }
}

export class SoundPlayer
{
public:
    bool initialize()
    {
        std::scoped_lock lock(mutex);
        if (initialized) return true;
        if (FAILED(XAudio2Create(xaudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR)))
        {
            std::cout << "[Sound] XAudio2Create failed" << std::endl;
            return false;
        }
        if (FAILED(xaudio2->CreateMasteringVoice(&masterVoice)))
        {
            std::cout << "[Sound] CreateMasteringVoice failed" << std::endl;
            xaudio2.Reset();
            return false;
        }
        initialized = true;
        std::cout << "[Sound] Initialized" << std::endl;
        return true;
    }

    void shutdown()
    {
        std::scoped_lock lock(mutex);
        for (auto& kv : clips) kv.second.data.clear();
        clips.clear();
        if (masterVoice) { masterVoice->DestroyVoice(); masterVoice = nullptr; }
        xaudio2.Reset();
        initialized = false;
        std::cout << "[Sound] Shutdown" << std::endl;
    }

    bool loadWavFile(const std::string& id, const std::wstring& path)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in) return false;
        Detail::Audio::WavHeader wh{};
        if (!Detail::Audio::parseWavPCM(in, wh)) return false;

        std::vector<std::uint8_t> data(wh.dataSize);
        in.seekg(wh.dataOffset, std::ios::beg);
        in.read(reinterpret_cast<char*>(data.data()), data.size());
        if (!in) return false;

        WAVEFORMATEX wfx{};
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = wh.numChannels;
        wfx.nSamplesPerSec = wh.sampleRate;
        wfx.wBitsPerSample = wh.bitsPerSample;
        wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        Clip clip{};
        clip.fmt = wfx;
        clip.data = std::move(data);
        clip.sharedBuffer = std::make_shared<std::vector<std::uint8_t>>(clip.data.begin(), clip.data.end());

        std::scoped_lock lock(mutex);
        clips[id] = std::move(clip);
        return true;
    }

    bool play(const std::string& id, float volume = 1.0f, bool loop = false)
    {
        std::scoped_lock lock(mutex);
        if (!initialized) return false;
        auto it = clips.find(id);
        if (it == clips.end()) return false;
        Clip& clip = it->second;

        // Use a persistent callback object to avoid freeing during callback
        static Detail::Audio::VoiceContext s_callback;
        IXAudio2SourceVoice* voice = nullptr;
        if (FAILED(xaudio2->CreateSourceVoice(&voice, &clip.fmt, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &s_callback)))
        {
            return false;
        }

        XAUDIO2_BUFFER buffer{};
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = static_cast<UINT32>(clip.sharedBuffer ? clip.sharedBuffer->size() : clip.data.size());
        buffer.pAudioData = reinterpret_cast<const BYTE*>(clip.sharedBuffer ? clip.sharedBuffer->data() : clip.data.data());
        // pin shared buffer lifetime for this play instance
        buffer.pContext = clip.sharedBuffer.get();
        buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

        if (FAILED(voice->SubmitSourceBuffer(&buffer))) { voice->DestroyVoice(); return false; }
        voice->SetVolume(volume);
        if (FAILED(voice->Start(0))) { voice->DestroyVoice(); return false; }
        return true;
    }

    bool playOneShotSine(float frequencyHz = 880.0f, float durationSec = 0.15f, float volume = 0.3f)
    {
        if (!initialized) return false;
        const int sampleRate = 48000;
        const int channels = 1;
        const int bitsPerSample = 16;
        const int samples = static_cast<int>(durationSec * sampleRate);
        std::vector<std::int16_t> pcm(samples);
        for (int i = 0; i < samples; ++i)
        {
            float t = static_cast<float>(i) / sampleRate;
            float s = std::sinf(2.0f * 3.1415926535f * frequencyHz * t);
            int v = static_cast<int>(s * 32767.0f);
            pcm[i] = static_cast<std::int16_t>(v);
        }

        WAVEFORMATEX wfx{};
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = channels;
        wfx.nSamplesPerSec = sampleRate;
        wfx.wBitsPerSample = bitsPerSample;
        wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        static Detail::Audio::VoiceContext s_callback;
        IXAudio2SourceVoice* voice = nullptr;
        if (FAILED(xaudio2->CreateSourceVoice(&voice, &wfx, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &s_callback)))
        {
            return false;
        }

        XAUDIO2_BUFFER buffer{};
        buffer.Flags = XAUDIO2_END_OF_STREAM;
        buffer.AudioBytes = static_cast<UINT32>(pcm.size() * sizeof(std::int16_t));
        buffer.pAudioData = reinterpret_cast<const BYTE*>(pcm.data());
        buffer.pContext = nullptr;
        buffer.LoopCount = 0;

        if (FAILED(voice->SubmitSourceBuffer(&buffer))) { voice->DestroyVoice(); return false; }
        voice->SetVolume(volume);
        if (FAILED(voice->Start(0))) { voice->DestroyVoice(); return false; }
        // Buffer memory must outlive playback. Keep it static small for one-shot beep.
        static std::vector<std::int16_t> s_pcm;
        s_pcm = std::move(pcm);
        return true;
    }

private:
    struct Clip
    {
        WAVEFORMATEX fmt{};
        std::vector<std::uint8_t> data;
        std::shared_ptr<std::vector<std::uint8_t>> sharedBuffer;
    };

    ComPtr<IXAudio2> xaudio2;
    IXAudio2MasteringVoice* masterVoice { nullptr };
    bool initialized { false };
    std::unordered_map<std::string, Clip> clips;
    std::mutex mutex;
};

export inline SoundPlayer& GetSoundPlayer()
{
    static SoundPlayer instance;
    return instance;
}


