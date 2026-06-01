# TODO:

## Dependencies

- [x] convert lib `miniaudio` to submodule
- [x] use `libvorbis` + `ogg` instead of `stb_vorbis`
  - uses SSE / SSE2 / AVX for vectorized math (`stb_vorbis` is mostly scalar C)
  - optimized MDCT implementation
  - architecture-dependent optimizations
  - faster decoding via precomputed tables (trades memory for speed)
  - cache-friendly data structures and transform steps
  - tuned and specialized critical decoding stages
  - designed for maximum decoding performance, while `stb_vorbis` focuses on minimal code size and single-file simplicity

## Audio features

- support for audio formats:
  - [x] `wav`
  - [x] `mp3`
  - [x] `flac`
  - [x] `opus`
  - [x] `ogg`
  - [x] `aiff`
- [ ] better sound api implementation for heaps
- [x] streaming support for HashLink / native target
  - [x] seekable native stream decoder for supported formats
  - [x] Heaps `hxd.snd.Data` streaming with initial preload window
  - [x] Stream directly from file path (native)
  - [ ] JS / WASM streaming support
- [x] optimizations
  - [x] avoid extra decoded PCM `Bytes` roundtrip when creating native buffers from encoded bytes
  - [x] release temporary decoded PCM after miniaudio copies it into `ma_audio_buffer`
  - [x] allocate PCM directly on HashLink heap for decoding API
  - [x] optimized sound callback removal ($O(1)$)
  - [x] broader load-time profiling and copy audit
- [ ] device enumeration api
- [ ] audio input / capture api
- [ ] microphone recording support
- [ ] duplex mode support (`playback` + `capture`)
- [ ] input/output device selection by id / name
- [ ] hotplug handling for audio devices
- expose device info:
  - [ ] backend name
  - [ ] device name
  - [ ] default sample rate
  - [ ] channel count
  - [ ] native sample format
  - [ ] is default device
- recording helpers:
  - [ ] start / stop / pause recording
  - [ ] capture into dynamic buffer
  - [ ] capture directly into file
  - [ ] user callback for live input processing
- [ ] resampling / channel conversion for capture streams
- [ ] low-latency playback / capture mode
- [ ] expose backend-specific controls where possible

## Effects / DSP

- [ ] basic DSP / effects API
- [ ] effect chain / processing graph via `ma_node_graph`
- [ ] gain / volume control
  - [ ] per-sound volume
  - [ ] per-bus volume
  - [ ] master / bus gain
- [ ] pan support
  - [ ] simple stereo pan via `ma_sound_set_pan()`
  - [ ] optional lower-level panner bindings
- [ ] delay / echo effect via `ma_delay_node`
- [ ] low-pass filter via `ma_lpf_node`
- [ ] high-pass filter via `ma_hpf_node`
- [ ] optional band-pass / biquad filter support
- [ ] wet / dry mix controls
- [ ] per-sound effect routing
- [ ] bus / master effects
- [ ] parallel routing via splitter node
- [ ] runtime parameter updates for supported effects
- [ ] custom DSP nodes for missing effects / advanced automation
- [ ] reverb support
  - [ ] evaluate `extras/nodes/ma_reverb_node`
  - [ ] document current limitations (`stereo-only`)
  - [ ] decide whether to ship as optional extra or custom integrated node
- [ ] decide implementation strategy:
  - [ ] use built-in `miniaudio` nodes where available
  - [ ] use `extras` nodes only as optional / experimental features
  - [ ] add custom DSP layer for unsupported or limited effects
  - [ ] keep public API backend-agnostic

## Memory management

- [x] allocate native audio objects through GC-managed HashLink handles
  - [x] buffers
  - [x] sounds
  - [x] sound groups
- [x] keep audio buffers alive while they are still referenced
- [x] decide buffer lifetime strategy: refcounting over GC-managed native handles
- [x] add GC-safe handling for streamed / decoded audio data
  - [x] native stream decoder allocated with HashLink finalizer
  - [x] explicit stream disposal from Heaps resource cache
  - [x] Heaps resource `dispose()` clears decoded / streamed cache and native buffer
- [x] prevent premature buffer freeing when sounds share the same source
- [x] prevent parent sound group disposal while child groups / sounds still reference it

## Bindings / public API

- [ ] add bindings for playback device control
- [ ] add bindings for capture device control
- [ ] add bindings for device enumeration
- [ ] add bindings for recording api
- [x] add bindings for native stream decoder reads
- [ ] add bindings for stream callbacks
- [ ] add bindings for effect / DSP api
- [ ] add bindings for backend / device capability queries
- add high-level convenience API:
  - [ ] `listPlaybackDevices()`
  - [ ] `listCaptureDevices()`
  - [ ] `openPlaybackDevice()`
  - [ ] `openCaptureDevice()`
  - [ ] `startRecording()`
  - [ ] `stopRecording()`
  - [ ] `createEffectChain()`
- [ ] validate stable C ABI for all exported audio/device types
- [ ] document ownership rules in bindings

## Development

- [x] test cases
- [x] playback tests
- [x] decode / stream tests
  - [x] deterministic decode tests for supported fixture formats
  - [x] Heaps resource cache tests
  - [x] forced streaming path tests with mid-stream decode checks
- [ ] device enumeration tests
- [ ] capture / recording tests
- [ ] duplex mode tests
- [ ] effect chain tests
- [ ] stress tests for buffer lifetime / GC interactions
  - [x] disposal-order regression test for sound-owned buffers and groups
  - [ ] high-volume repeated scene/cache churn test

## Language / portability

- [x] translate codebase from `C++` to `C`, uhh so why not `C++`?:
  - simpler ABI and easier interop with other languages / FFI
  - easier embedding into existing C codebases and engines
  - lower runtime / language feature complexity
  - no exceptions / RTTI / templates overhead in build and maintenance
  - more predictable compilation across toolchains and platforms
  - better fit for minimal dependency and low-level systems code
  - easier integration with custom allocators, GC, and manual memory ownership rules
  - smaller binaries and simpler build pipeline
  - easier to audit, port, and debug in constrained environments
  - keeps public API straightforward and stable
- verify cross-platform audio backends:
  - [ ] `Windows` (`WASAPI`, fallback if needed)
  - [ ] `Linux` (`PulseAudio`, `ALSA`, optionally `JACK`)
  - [ ] `macOS` (`CoreAudio`)
  - [ ] `iOS`
  - [ ] `Android`
  - [ ] `Web` / `Emscripten` if feasible

## Build / CI

- [x] actions build via cmake in release mode for C++ `hdll` + `.lib`, all packed into one zip
- add CI builds for more platforms:
  - [x] `Windows`
  - [ ] `Linux`
  - [ ] `Linux arm64`
  - [ ] `macOS`
  - [ ] `macOS arm64`

## Docs

- [ ] add fully readme
- add examples:
  - [ ] basic playback
  - [ ] streaming playback
  - [ ] microphone capture
  - [ ] record to file
  - [ ] duplex input/output
  - [ ] effect chain setup
