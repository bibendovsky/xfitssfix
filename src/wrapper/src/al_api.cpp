// XFITSSFIX - X-Fi Titanium Streaming Sound Fix
// Copyright (c) 2022 Boris I. Bendovsky (bibendovsky@hotmail.com) and Contributors
// SPDX-License-Identifier: MIT

#include <cstddef>
#include <algorithm>
#include <exception>
#include <functional>
#include <chrono>
#include <list>
#include <memory>
#include <mutex>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <xfitssfix/al.h>
#include <xfitssfix/alc.h>
#include <xfitssfix/al_api.h>
#include <xfitssfix/al_api_utils.h>
#include <xfitssfix/al_loader.h>
#include <xfitssfix/al_symbols.h>
#include <xfitssfix/env.h>
#include <xfitssfix/exception.h>
#include <xfitssfix/fs.h>
#include <xfitssfix/fs_path.h>
#include <xfitssfix/logger.h>
#include <xfitssfix/moveable_mutex_lock.h>
#include <xfitssfix/mutex.h>
#include <xfitssfix/process.h>
#include <xfitssfix/scope_exit.h>
#include <xfitssfix/shared_library.h>
#include <xfitssfix/span.h>
#include <xfitssfix/string.h>
#include <xfitssfix/thread.h>
#include <xfitssfix/utils.h>

extern "C" {

// ALC 1.1

ALC_API ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice* device, const ALCint* attrlist);
ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext* context);
ALC_API void ALC_APIENTRY alcProcessContext(ALCcontext* context);
ALC_API void ALC_APIENTRY alcSuspendContext(ALCcontext* context);
ALC_API void ALC_APIENTRY alcDestroyContext(ALCcontext* context);
ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext();
ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext* context);
ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar* devicename);
ALC_API ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice* device);
ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice* device);
ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice* device, const ALCchar* extname);
ALC_API void* ALC_APIENTRY alcGetProcAddress(ALCdevice* device, const ALCchar* funcname);
ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice* device, const ALCchar* enumname);
ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice* device, ALCenum param);
ALC_API void ALC_APIENTRY alcGetIntegerv(ALCdevice* device, ALCenum param, ALCsizei size, ALCint* values);
ALC_API ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(const ALCchar* devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize);
ALC_API ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice* device);
ALC_API void ALC_APIENTRY alcCaptureStart(ALCdevice* device);
ALC_API void ALC_APIENTRY alcCaptureStop(ALCdevice* device);
ALC_API void ALC_APIENTRY alcCaptureSamples(ALCdevice* device, ALCvoid* buffer, ALCsizei samples);

// AL 1.1

AL_API void AL_APIENTRY alDopplerFactor(ALfloat value);
AL_API void AL_APIENTRY alDopplerVelocity(ALfloat value);
AL_API void AL_APIENTRY alSpeedOfSound(ALfloat value);
AL_API void AL_APIENTRY alDistanceModel(ALenum distanceModel);
AL_API void AL_APIENTRY alEnable(ALenum capability);
AL_API void AL_APIENTRY alDisable(ALenum capability);
AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability);
AL_API const ALchar* AL_APIENTRY alGetString(ALenum param);
AL_API void AL_APIENTRY alGetBooleanv(ALenum param, ALboolean* values);
AL_API void AL_APIENTRY alGetIntegerv(ALenum param, ALint* values);
AL_API void AL_APIENTRY alGetFloatv(ALenum param, ALfloat* values);
AL_API void AL_APIENTRY alGetDoublev(ALenum param, ALdouble* values);
AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum param);
AL_API ALint AL_APIENTRY alGetInteger(ALenum param);
AL_API ALfloat AL_APIENTRY alGetFloat(ALenum param);
AL_API ALdouble AL_APIENTRY alGetDouble(ALenum param);
AL_API ALenum AL_APIENTRY alGetError();
AL_API ALboolean AL_APIENTRY alIsExtensionPresent(const ALchar* extname);
AL_API void* AL_APIENTRY alGetProcAddress(const ALchar* fname);
AL_API ALenum AL_APIENTRY alGetEnumValue(const ALchar* ename);
AL_API void AL_APIENTRY alListenerf(ALenum param, ALfloat value);
AL_API void AL_APIENTRY alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alListenerfv(ALenum param, const ALfloat* values);
AL_API void AL_APIENTRY alListeneri(ALenum param, ALint value);
AL_API void AL_APIENTRY alListener3i(ALenum param, ALint value1, ALint value2, ALint value3);
AL_API void AL_APIENTRY alListeneriv(ALenum param, const ALint* values);
AL_API void AL_APIENTRY alGetListenerf(ALenum param, ALfloat* value);
AL_API void AL_APIENTRY alGetListener3f(ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
AL_API void AL_APIENTRY alGetListenerfv(ALenum param, ALfloat* values);
AL_API void AL_APIENTRY alGetListeneri(ALenum param, ALint* value);
AL_API void AL_APIENTRY alGetListener3i(ALenum param, ALint* value1, ALint* value2, ALint* value3);
AL_API void AL_APIENTRY alGetListeneriv(ALenum param, ALint* values);
AL_API void AL_APIENTRY alGenSources(ALsizei n, ALuint* sources);
AL_API void AL_APIENTRY alDeleteSources(ALsizei n, const ALuint* sources);
AL_API ALboolean AL_APIENTRY alIsSource(ALuint source);
AL_API void AL_APIENTRY alSourcef(ALuint source, ALenum param, ALfloat value);
AL_API void AL_APIENTRY alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alSourcefv(ALuint source, ALenum param, const ALfloat* values);
AL_API void AL_APIENTRY alSourcei(ALuint source, ALenum param, ALint value);
AL_API void AL_APIENTRY alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3);
AL_API void AL_APIENTRY alSourceiv(ALuint source, ALenum param, const ALint* values);
AL_API void AL_APIENTRY alGetSourcef(ALuint source, ALenum param, ALfloat* value);
AL_API void AL_APIENTRY alGetSource3f(ALuint source, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
AL_API void AL_APIENTRY alGetSourcefv(ALuint source, ALenum param, ALfloat* values);
AL_API void AL_APIENTRY alGetSourcei(ALuint source, ALenum param, ALint* value);
AL_API void AL_APIENTRY alGetSource3i(ALuint source, ALenum param, ALint* value1, ALint* value2, ALint* value3);
AL_API void AL_APIENTRY alGetSourceiv(ALuint source, ALenum param, ALint* values);
AL_API void AL_APIENTRY alSourcePlayv(ALsizei n, const ALuint* sources);
AL_API void AL_APIENTRY alSourceStopv(ALsizei n, const ALuint* sources);
AL_API void AL_APIENTRY alSourceRewindv(ALsizei n, const ALuint* sources);
AL_API void AL_APIENTRY alSourcePausev(ALsizei n, const ALuint* sources);
AL_API void AL_APIENTRY alSourcePlay(ALuint source);
AL_API void AL_APIENTRY alSourceStop(ALuint source);
AL_API void AL_APIENTRY alSourceRewind(ALuint source);
AL_API void AL_APIENTRY alSourcePause(ALuint source);
AL_API void AL_APIENTRY alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint* buffers);
AL_API void AL_APIENTRY alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint* buffers);
AL_API void AL_APIENTRY alGenBuffers(ALsizei n, ALuint* buffers);
AL_API void AL_APIENTRY alDeleteBuffers(ALsizei n, const ALuint* buffers);
AL_API ALboolean AL_APIENTRY alIsBuffer(ALuint buffer);
AL_API void AL_APIENTRY alBufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq);
AL_API void AL_APIENTRY alBufferf(ALuint buffer, ALenum param, ALfloat value);
AL_API void AL_APIENTRY alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
AL_API void AL_APIENTRY alBufferfv(ALuint buffer, ALenum param, const ALfloat* values);
AL_API void AL_APIENTRY alBufferi(ALuint buffer, ALenum param, ALint value);
AL_API void AL_APIENTRY alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3);
AL_API void AL_APIENTRY alBufferiv(ALuint buffer, ALenum param, const ALint* values);
AL_API void AL_APIENTRY alGetBufferf(ALuint buffer, ALenum param, ALfloat* value);
AL_API void AL_APIENTRY alGetBuffer3f(ALuint buffer, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3);
AL_API void AL_APIENTRY alGetBufferfv(ALuint buffer, ALenum param, ALfloat* values);
AL_API void AL_APIENTRY alGetBufferi(ALuint buffer, ALenum param, ALint* value);
AL_API void AL_APIENTRY alGetBuffer3i(ALuint buffer, ALenum param, ALint* value1, ALint* value2, ALint* value3);
AL_API void AL_APIENTRY alGetBufferiv(ALuint buffer, ALenum param, ALint* values);

} // extern "C"

namespace {

// EFX 1.0

void AL_APIENTRY alGenEffects(ALsizei n, ALuint* effects) noexcept;
void AL_APIENTRY alDeleteEffects(ALsizei n, ALuint* effects) noexcept;
ALboolean AL_APIENTRY alIsEffect(ALuint eid) noexcept;
void AL_APIENTRY alEffecti(ALuint eid, ALenum param, ALint value) noexcept;
void AL_APIENTRY alEffectiv(ALuint eid, ALenum param, ALint* values) noexcept;
void AL_APIENTRY alEffectf(ALuint eid, ALenum param, ALfloat value) noexcept;
void AL_APIENTRY alEffectfv(ALuint eid, ALenum param, ALfloat* values) noexcept;
void AL_APIENTRY alGetEffecti(ALuint eid, ALenum pname, ALint* value) noexcept;
void AL_APIENTRY alGetEffectiv(ALuint eid, ALenum pname, ALint* values) noexcept;
void AL_APIENTRY alGetEffectf(ALuint eid, ALenum pname, ALfloat* value) noexcept;
void AL_APIENTRY alGetEffectfv(ALuint eid, ALenum pname, ALfloat* values) noexcept;
void AL_APIENTRY alGenFilters(ALsizei n, ALuint* filters) noexcept;
void AL_APIENTRY alDeleteFilters(ALsizei n, ALuint* filters) noexcept;
ALboolean AL_APIENTRY alIsFilter(ALuint fid) noexcept;
void AL_APIENTRY alFilteri(ALuint fid, ALenum param, ALint value) noexcept;
void AL_APIENTRY alFilteriv(ALuint fid, ALenum param, ALint* values) noexcept;
void AL_APIENTRY alFilterf(ALuint fid, ALenum param, ALfloat value) noexcept;
void AL_APIENTRY alFilterfv(ALuint fid, ALenum param, ALfloat* values) noexcept;
void AL_APIENTRY alGetFilteri(ALuint fid, ALenum pname, ALint* value) noexcept;
void AL_APIENTRY alGetFilteriv(ALuint fid, ALenum pname, ALint* values) noexcept;
void AL_APIENTRY alGetFilterf(ALuint fid, ALenum pname, ALfloat* value) noexcept;
void AL_APIENTRY alGetFilterfv(ALuint fid, ALenum pname, ALfloat* values) noexcept;
void AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept;
void AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept;
ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint slot) noexcept;
void AL_APIENTRY alAuxiliaryEffectSloti(ALuint asid, ALenum param, ALint value) noexcept;
void AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint asid, ALenum param, ALint* values) noexcept;
void AL_APIENTRY alAuxiliaryEffectSlotf(ALuint asid, ALenum param, ALfloat value) noexcept;
void AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint asid, ALenum param, ALfloat* values) noexcept;
void AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint asid, ALenum pname, ALint* value) noexcept;
void AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint asid, ALenum pname, ALint* values) noexcept;
void AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint asid, ALenum pname, ALfloat* value) noexcept;
void AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint asid, ALenum pname, ALfloat* values) noexcept;

// EAX

ALenum AL_APIENTRY EAXSet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept;
ALenum AL_APIENTRY EAXGet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept;

} // namespace

// ==========================================================================

namespace xfitssfix {

class AlApiException : public Exception
{
public:
	explicit AlApiException(const char* message)
		:
		Exception{"AL_API", message}
	{}
};

// ==========================================================================

class AlApiImpl final : public AlApi
{
public:
	AlApiImpl() noexcept;
	~AlApiImpl() override;

	void on_thread_detach() noexcept override;
	void on_process_detach() noexcept override;

	// =========================================================================
	// ALC 1.1

	ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice* device, const ALCint* attrlist) noexcept;
	ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext* context) noexcept;
	void ALC_APIENTRY alcProcessContext(ALCcontext* context) noexcept;
	void ALC_APIENTRY alcSuspendContext(ALCcontext* context) noexcept;
	void ALC_APIENTRY alcDestroyContext(ALCcontext* context) noexcept;
	ALCcontext* ALC_APIENTRY alcGetCurrentContext() noexcept;
	ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext* context) noexcept;
	ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar* devicename) noexcept;
	ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice* device) noexcept;
	ALCenum ALC_APIENTRY alcGetError(ALCdevice* device) noexcept;
	ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice* device, const ALCchar* extname) noexcept;
	void* ALC_APIENTRY alcGetProcAddress(ALCdevice* device, const ALCchar* funcname) noexcept;
	ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice* device, const ALCchar* enumname) noexcept;
	const ALCchar* ALC_APIENTRY alcGetString(ALCdevice* device, ALCenum param) noexcept;
	void ALC_APIENTRY alcGetIntegerv(ALCdevice* device, ALCenum param, ALCsizei size, ALCint* values) noexcept;
	ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(const ALCchar* devicename, ALCuint frequency, ALCenum format, ALCsizei buffersize) noexcept;
	ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice* device) noexcept;
	void ALC_APIENTRY alcCaptureStart(ALCdevice* device) noexcept;
	void ALC_APIENTRY alcCaptureStop(ALCdevice* device) noexcept;
	void ALC_APIENTRY alcCaptureSamples(ALCdevice* device, ALCvoid* buffer, ALCsizei samples) noexcept;

	// AL 1.1

	void AL_APIENTRY alDopplerFactor(ALfloat value) noexcept;
	void AL_APIENTRY alDopplerVelocity(ALfloat value) noexcept;
	void AL_APIENTRY alSpeedOfSound(ALfloat value) noexcept;
	void AL_APIENTRY alDistanceModel(ALenum distanceModel) noexcept;
	void AL_APIENTRY alEnable(ALenum capability) noexcept;
	void AL_APIENTRY alDisable(ALenum capability) noexcept;
	ALboolean AL_APIENTRY alIsEnabled(ALenum capability) noexcept;
	const ALchar* AL_APIENTRY alGetString(ALenum param) noexcept;
	void AL_APIENTRY alGetBooleanv(ALenum param, ALboolean* values) noexcept;
	void AL_APIENTRY alGetIntegerv(ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alGetFloatv(ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetDoublev(ALenum param, ALdouble* values) noexcept;
	ALboolean AL_APIENTRY alGetBoolean(ALenum param) noexcept;
	ALint AL_APIENTRY alGetInteger(ALenum param) noexcept;
	ALfloat AL_APIENTRY alGetFloat(ALenum param) noexcept;
	ALdouble AL_APIENTRY alGetDouble(ALenum param) noexcept;
	ALenum AL_APIENTRY alGetError() noexcept;
	ALboolean AL_APIENTRY alIsExtensionPresent(const ALchar* extname) noexcept;
	void* AL_APIENTRY alGetProcAddress(const ALchar* fname) noexcept;
	ALenum AL_APIENTRY alGetEnumValue(const ALchar* ename) noexcept;
	void AL_APIENTRY alListenerf(ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept;
	void AL_APIENTRY alListenerfv(ALenum param, const ALfloat* values) noexcept;
	void AL_APIENTRY alListeneri(ALenum param, ALint value) noexcept;
	void AL_APIENTRY alListener3i(ALenum param, ALint value1, ALint value2, ALint value3) noexcept;
	void AL_APIENTRY alListeneriv(ALenum param, const ALint* values) noexcept;
	void AL_APIENTRY alGetListenerf(ALenum param, ALfloat* value) noexcept;
	void AL_APIENTRY alGetListener3f(ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) noexcept;
	void AL_APIENTRY alGetListenerfv(ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetListeneri(ALenum param, ALint* value) noexcept;
	void AL_APIENTRY alGetListener3i(ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept;
	void AL_APIENTRY alGetListeneriv(ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alGenSources(ALsizei n, ALuint* sources) noexcept;
	void AL_APIENTRY alDeleteSources(ALsizei n, const ALuint* sources) noexcept;
	ALboolean AL_APIENTRY alIsSource(ALuint source) noexcept;
	void AL_APIENTRY alSourcef(ALuint source, ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept;
	void AL_APIENTRY alSourcefv(ALuint source, ALenum param, const ALfloat* values) noexcept;
	void AL_APIENTRY alSourcei(ALuint source, ALenum param, ALint value) noexcept;
	void AL_APIENTRY alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3) noexcept;
	void AL_APIENTRY alSourceiv(ALuint source, ALenum param, const ALint* values) noexcept;
	void AL_APIENTRY alGetSourcef(ALuint source, ALenum param, ALfloat* value) noexcept;
	void AL_APIENTRY alGetSource3f(ALuint source, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) noexcept;
	void AL_APIENTRY alGetSourcefv(ALuint source, ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetSourcei(ALuint source, ALenum param, ALint* value) noexcept;
	void AL_APIENTRY alGetSource3i(ALuint source, ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept;
	void AL_APIENTRY alGetSourceiv(ALuint source, ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alSourcePlayv(ALsizei n, const ALuint* sources) noexcept;
	void AL_APIENTRY alSourceStopv(ALsizei n, const ALuint* sources) noexcept;
	void AL_APIENTRY alSourceRewindv(ALsizei n, const ALuint* sources) noexcept;
	void AL_APIENTRY alSourcePausev(ALsizei n, const ALuint* sources) noexcept;
	void AL_APIENTRY alSourcePlay(ALuint source) noexcept;
	void AL_APIENTRY alSourceStop(ALuint source) noexcept;
	void AL_APIENTRY alSourceRewind(ALuint source) noexcept;
	void AL_APIENTRY alSourcePause(ALuint source) noexcept;
	void AL_APIENTRY alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint* buffers) noexcept;
	void AL_APIENTRY alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint* buffers) noexcept;
	void AL_APIENTRY alGenBuffers(ALsizei n, ALuint* buffers) noexcept;
	void AL_APIENTRY alDeleteBuffers(ALsizei n, const ALuint* buffers) noexcept;
	ALboolean AL_APIENTRY alIsBuffer(ALuint buffer) noexcept;
	void AL_APIENTRY alBufferData(ALuint buffer, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq) noexcept;
	void AL_APIENTRY alBufferf(ALuint buffer, ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept;
	void AL_APIENTRY alBufferfv(ALuint buffer, ALenum param, const ALfloat* values) noexcept;
	void AL_APIENTRY alBufferi(ALuint buffer, ALenum param, ALint value) noexcept;
	void AL_APIENTRY alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3) noexcept;
	void AL_APIENTRY alBufferiv(ALuint buffer, ALenum param, const ALint* values) noexcept;
	void AL_APIENTRY alGetBufferf(ALuint buffer, ALenum param, ALfloat* value) noexcept;
	void AL_APIENTRY alGetBuffer3f(ALuint buffer, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) noexcept;
	void AL_APIENTRY alGetBufferfv(ALuint buffer, ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetBufferi(ALuint buffer, ALenum param, ALint* value) noexcept;
	void AL_APIENTRY alGetBuffer3i(ALuint buffer, ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept;
	void AL_APIENTRY alGetBufferiv(ALuint buffer, ALenum param, ALint* values) noexcept;

	// EFX 1.0

	void AL_APIENTRY alGenEffects(ALsizei n, ALuint* effects) noexcept;
	void AL_APIENTRY alDeleteEffects(ALsizei n, ALuint* effects) noexcept;
	ALboolean AL_APIENTRY alIsEffect(ALuint eid) noexcept;
	void AL_APIENTRY alEffecti(ALuint eid, ALenum param, ALint value) noexcept;
	void AL_APIENTRY alEffectiv(ALuint eid, ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alEffectf(ALuint eid, ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alEffectfv(ALuint eid, ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetEffecti(ALuint eid, ALenum pname, ALint* value) noexcept;
	void AL_APIENTRY alGetEffectiv(ALuint eid, ALenum pname, ALint* values) noexcept;
	void AL_APIENTRY alGetEffectf(ALuint eid, ALenum pname, ALfloat* value) noexcept;
	void AL_APIENTRY alGetEffectfv(ALuint eid, ALenum pname, ALfloat* values) noexcept;
	void AL_APIENTRY alGenFilters(ALsizei n, ALuint* filters) noexcept;
	void AL_APIENTRY alDeleteFilters(ALsizei n, ALuint* filters) noexcept;
	ALboolean AL_APIENTRY alIsFilter(ALuint fid) noexcept;
	void AL_APIENTRY alFilteri(ALuint fid, ALenum param, ALint value) noexcept;
	void AL_APIENTRY alFilteriv(ALuint fid, ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alFilterf(ALuint fid, ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alFilterfv(ALuint fid, ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetFilteri(ALuint fid, ALenum pname, ALint* value) noexcept;
	void AL_APIENTRY alGetFilteriv(ALuint fid, ALenum pname, ALint* values) noexcept;
	void AL_APIENTRY alGetFilterf(ALuint fid, ALenum pname, ALfloat* value) noexcept;
	void AL_APIENTRY alGetFilterfv(ALuint fid, ALenum pname, ALfloat* values) noexcept;
	void AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept;
	void AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept;
	ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint slot) noexcept;
	void AL_APIENTRY alAuxiliaryEffectSloti(ALuint asid, ALenum param, ALint value) noexcept;
	void AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint asid, ALenum param, ALint* values) noexcept;
	void AL_APIENTRY alAuxiliaryEffectSlotf(ALuint asid, ALenum param, ALfloat value) noexcept;
	void AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint asid, ALenum param, ALfloat* values) noexcept;
	void AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint asid, ALenum pname, ALint* value) noexcept;
	void AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint asid, ALenum pname, ALint* values) noexcept;
	void AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint asid, ALenum pname, ALfloat* value) noexcept;
	void AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint asid, ALenum pname, ALfloat* values) noexcept;

	// EAX

	ALenum AL_APIENTRY EAXSet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept;
	ALenum AL_APIENTRY EAXGet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept;

	// =========================================================================

private:
	struct Strings;
	struct ErrorMessages;
	struct Device;

	static constexpr auto null_string = "<NULL>";
	static constexpr auto min_string_buffer_capacity = 4 * 1'024;
	static constexpr auto sources_capacity = 256;

	using Clock = std::chrono::steady_clock;
	using ClockTimePoint = Clock::time_point;

	static constexpr auto log_level_none = 0;
	static constexpr auto log_level_error = 1;
	static constexpr auto log_level_warning = 2;
	static constexpr auto log_level_trace = 3;

	template<typename T>
	static String& ptr_to_str(T* ptr, String& string)
	{
		if (ptr != nullptr)
		{
			return to_string_hex(ptr, string);
		}

		string = null_string;
		return string;
	}

	using AlSymbolMap = std::unordered_map<std::string_view, void*>;

	struct Buffer
	{
		int rate;
		int frame_size;
		int frame_count;
		int duration_ms;
	};

	using BufferMap = std::unordered_map<ALuint, Buffer>;

	class BufferQueue
	{
	public:
		void reserve(int count)
		{
			assert(count >= 0);
			queue_.reserve(count);
		}

		int get_size() const noexcept
		{
			return static_cast<int>(queue_.size());
		}

		bool is_empty() const noexcept
		{
			return get_size() == 0;
		}

		void push(Buffer* buffer)
		{
			queue_.emplace_back(buffer);
		}

		void pop(int count)
		{
			assert(count >= 0);

			if (count == 0)
			{
				return;
			}

			if (count > get_size())
			{
				fail("Not enough buffers to pop.");
			}

			const auto begin_it = queue_.begin();
			const auto end_it = begin_it + count;
			queue_.erase(begin_it, end_it);
		}

		void clear()
		{
			queue_.clear();
		}

		Buffer* get(int index) const
		{
			if (index < 0 || index >= get_size())
			{
				fail("Index out of range.");
			}

			return queue_[index];
		}

		Buffer* get_front() const
		{
			return get(0);
		}

		Buffer* operator[](int index) const
		{
			return get(index);
		}

		Buffer* const* begin() const noexcept
		{
			return queue_.data();
		}

		Buffer* const* end() const noexcept
		{
			return begin() + get_size();
		}

	private:
		using Queue = std::vector<Buffer*>;

		class BufferQueueException : public Exception
		{
		public:
			explicit BufferQueueException(const char* message)
				:
				Exception{"BUFFER_QUEUE", message}
			{}
		};

		[[noreturn]] static void fail(const char* message)
		{
			throw BufferQueueException{message};
		}

		Queue queue_{};
	}; // BufferQueue

	struct Source
	{
		ALuint id{AL_NONE};
		bool is_playing{};
		bool is_looping{};
		bool is_streaming{};
		double r_rate{0.0};
		double r_pitch_1000{1'000.0};
		ClockTimePoint monitoring_time_point{};
		BufferQueue queue{};

		constexpr auto is_monitorable() const noexcept
		{
			return is_playing && !is_looping;
		}
	};

	using SourceMap = std::unordered_map<ALuint, Source>;
	using MonitoringSources = std::unordered_set<Source*>;

	struct Context;

	struct ContextThread
	{
		bool quit_flag{};
		AlApiImpl* al_api{};
		Context* context{};
		ThreadUPtr thread{};
	};

	using ContextThreadUPtr = std::unique_ptr<ContextThread>;

	struct Context
	{
		bool is_initialized;
		bool thread_quit_flag;
		bool has_xram;
		Device* device;
		ALCcontext* al_context;
		SourceMap source_map;
		MonitoringSources monitoring_sources;
		EfxSymbols efx_symbols;
		XRamSymbols xram_symbols;
		EaxSymbols eax_symbols;
		AlSymbolMap efx_symbol_map;
		ALenum xram_al_storage_accessible;
		AlSymbolMap eax_symbol_map;
		ContextThreadUPtr context_thread;
	};

	using Contexts = std::list<Context>;

	struct Device
	{
		ALCdevice* al_device;
		BufferMap buffer_map;
		Contexts contexts;
	}; // Device

	using Devices = std::list<Device>;

	using InitializeFunc = MoveableMutexLock (AlApiImpl::*)();
	using MutexLock = std::scoped_lock<Mutex>;

	InitializeFunc initialize_func_{};
	MutexUPtr mutex_{};
	Int process_id_{};
	int log_level_{};
	NullableLogger logger_{};
	SharedLibraryUPtr al_library_{};
	AlLoaderUPtr al_loader_{};
	AlSymbols al_symbols_{};
	AlSymbolMap alc_symbol_map_{};
	AlSymbolMap al_symbol_map_{};
	Devices devices_{};
	Context* current_context_{};

	String string_buffer_1_{};
	String string_buffer_2_{};

	[[noreturn]] static void fail(const char* message);

	template<typename TGlobalSymbol, typename TAlSymbol>
	static void try_map_al_symbol(
		const char* symbol_name,
		TGlobalSymbol global_symbol,
		const TAlSymbol& al_symbol,
		AlSymbolMap& al_symbol_map) noexcept
	{
		if (al_symbol == nullptr)
		{
			return;
		}

		al_symbol_map.emplace(symbol_name, reinterpret_cast<void*>(global_symbol));
	}

	static const char* get_source_param_string(ALenum param) noexcept;
	static String& source_param_to_string(ALenum param, String& string);

	static const char* get_buffer_format_string(ALenum param) noexcept;
	static String& buffer_format_to_string(ALenum param, String& string);

	void initialize_al_driver();
	void initialize_al_symbols();
	void initialize_alc_symbol_map();
	void initialize_al_symbol_map();
	static void initialize_efx_symbol_map(const EfxSymbols& symbols, AlSymbolMap& map) noexcept;
	void initialize_efx(Context& context);
	void initialize_xram(Context& context);
	static void initialize_eax_symbol_map(const EaxSymbols& symbols, AlSymbolMap& map) noexcept;
	void initialize_eax(Context& context);

	void initialize_logger();

	MoveableMutexLock initialize_invalid_state();
	MoveableMutexLock initialize_not_initialized();
	MoveableMutexLock initialize_get_lock();
	MoveableMutexLock initialize_try_to_initialize();
	MoveableMutexLock initialize();
	MoveableMutexLock get_lock();

	Device* find_device(ALCdevice* al_device) noexcept;
	Device& get_device(ALCdevice* al_device);
	Device& get_device();

	Context& get_context(ALCcontext* al_context);
	Context& get_context();

	Buffer& get_buffer(ALuint bid);
	Source& get_source(ALuint sid);

	void set_al_invalid_enum();

	static void* get_symbol(const AlSymbolMap& al_symbol_map, const ALchar* name);
	void* get_alc_symbol(const ALchar* name) const;
	void* get_al_symbol(const ALchar* name) const;

	void mark_source_as_monitoring(Context& context, Source& source);
	void mark_source_as_non_monitoring(Context& context, Source& source);

	void update_source_monitoring(Source& source);

	void handle_al_source_ix(ALuint sid, ALenum param, const ALint* values);
	void handle_al_source_fx(ALuint sid, ALenum param, const ALfloat* values);
	void handle_al_get_source_ix(ALuint sid, ALenum param, ALint* values);
	void handle_al_source_state(ALsizei ns, const ALuint *sids, ALenum state);

	struct MonitoringContext
	{
		Context* context;
		MonitoringSources* monitoring_sources;
		Source* source;
		ClockTimePoint time_point;
		String* string_buffer_1;
		String* string_buffer_2;
	};

	void handle_monitoring_source(MonitoringContext& monitoring_context);
	void handle_monitoring_sources(MonitoringContext& monitoring_context);

	static void thread_func_proxy(void* user_data);
	void thread_func(ContextThread& context_thread);
}; // AlApiImpl

// ==========================================================================

struct AlApiImpl::Strings {};

// ==========================================================================

struct AlApiImpl::ErrorMessages
{
	static constexpr auto invalid_state = "Invalid state.";
	static constexpr auto not_initialized = "Not initialized.";
};

// ==========================================================================

AlApiImpl::AlApiImpl() noexcept
{
	try
	{
		mutex_ = make_mutex();
		initialize_func_ = &AlApiImpl::initialize_try_to_initialize;
	}
	catch (...)
	{
		initialize_func_ = &AlApiImpl::initialize_invalid_state;
	}
}

AlApiImpl::~AlApiImpl()
{
	if (log_level_ > log_level_none)
	{
		logger_.info("");

		string_buffer_1_.clear();
		string_buffer_1_ += "Finalizing process ";
		string_buffer_1_ += to_string(process_id_, string_buffer_2_);
		string_buffer_1_ += '.';
		logger_.info(string_buffer_1_.c_str());

		logger_.info(">>>>>>>>>>>>>>>>>>>>>>>>");
		logger_.info("");
		logger_.flush();
	}
}

void AlApiImpl::on_thread_detach() noexcept
{
	if (log_level_ > log_level_none)
	{
		logger_.flush();
	}
}

void AlApiImpl::on_process_detach() noexcept
{
	if (log_level_ > log_level_none)
	{
		logger_.set_immediate_mode();
	}
}

ALCcontext* ALC_APIENTRY AlApiImpl::alcCreateContext(ALCdevice* device, const ALCint* attrlist) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ > log_level_none)
	{
		logger_.info("");
		logger_.info(al_api::Strings::equals_line_16);

		string_buffer_1_.clear();
		string_buffer_1_ += "Create a context on device ";
		string_buffer_1_ += ptr_to_str(device, string_buffer_2_);
		string_buffer_1_ += '.';
		logger_.info(string_buffer_1_.c_str());
	}

	const auto al_context = al_symbols_.alcCreateContext(device, attrlist);

	if (al_context == nullptr)
	{
		fail("AL failed to create acontext.");
	}

	auto& our_device = get_device(device);
	our_device.contexts.emplace_back(Context{});
	auto& context = our_device.contexts.back();
	context.device = &our_device;
	context.al_context = al_context;
	return al_context;
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCreateContext);
	}

	return nullptr;
}

ALCboolean ALC_APIENTRY AlApiImpl::alcMakeContextCurrent(ALCcontext* context) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ > log_level_none)
	{
		logger_.info("");
		logger_.info(al_api::Strings::equals_line_16);

		string_buffer_1_.clear();

		if (context != nullptr)
		{
			string_buffer_1_ += "Make context ";
			string_buffer_1_ += to_string_hex(context, string_buffer_2_);
			string_buffer_1_ += " current.";
		}
		else
		{
			string_buffer_1_ += "Unset the current context.";
		}

		logger_.info(string_buffer_1_.c_str());
	}

	const auto alc_result = al_symbols_.alcMakeContextCurrent(context);

	if (alc_result == ALC_FALSE)
	{
		fail("AL failed to unset current context.");
	}

	if (context != nullptr)
	{
		auto& our_context = get_context(context);
		current_context_ = &our_context;

		if (!our_context.is_initialized)
		{
			our_context.is_initialized = true;
			our_context.source_map.reserve(sources_capacity);
			our_context.monitoring_sources.reserve(sources_capacity);

			initialize_efx(our_context);
			initialize_xram(our_context);
			initialize_eax(our_context);

			//
			if (log_level_ > log_level_none)
			{
				logger_.info("Create a monitoring thread.");
			}

			auto context_thread = std::make_unique<ContextThread>();
			context_thread->al_api = this;
			context_thread->context = &our_context;
			context_thread->thread = make_thread(thread_func_proxy, context_thread.get());
			our_context.context_thread.swap(context_thread);

			if (log_level_ > log_level_none)
			{
				string_buffer_1_.clear();
				string_buffer_1_ += "Thread instance: ";
				string_buffer_1_ += to_string_hex(our_context.context_thread->thread.get(), string_buffer_2_);
				logger_.info(string_buffer_1_.c_str());
			}
		}
	}
	else
	{
		current_context_ = nullptr;
	}

	return ALC_TRUE;
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcMakeContextCurrent);
	}

	return ALC_FALSE;
}

void ALC_APIENTRY AlApiImpl::alcProcessContext(ALCcontext* context) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcProcessContext(context);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcProcessContext);
	}
}

void ALC_APIENTRY AlApiImpl::alcSuspendContext(ALCcontext* context) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcSuspendContext(context);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcSuspendContext);
	}
}

void ALC_APIENTRY AlApiImpl::alcDestroyContext(ALCcontext* context) noexcept
try
{
	auto context_thread = ContextThreadUPtr();

	{
		const auto lock = get_lock();

		if (log_level_ > log_level_none)
		{
			logger_.info("");
			logger_.info(al_api::Strings::equals_line_16);

			string_buffer_1_.clear();
			string_buffer_1_ += "Destroy context ";
			string_buffer_1_ += ptr_to_str(context, string_buffer_2_);
			string_buffer_1_ += '.';
			logger_.info(string_buffer_1_.c_str());
		}

		if (context != nullptr)
		{
			if (current_context_ != nullptr)
			{
				auto& our_context = get_context(context);

				if (&our_context == current_context_)
				{
					current_context_ = nullptr;

					if (log_level_ >= log_level_warning)
					{
						logger_.warning("Destroying the current context.");
						logger_.flush();
					}
				}
			}

			for (auto& device : devices_)
			{
				device.contexts.remove_if(
					[al_context = context, &context_thread](Context& context)
					{
						if (context.al_context != al_context)
						{
							return false;
						}

						if (context.context_thread != nullptr)
						{
							context.context_thread->quit_flag = true;
							context_thread.swap(context.context_thread);
						}

						return true;
					}
				);
			}
		}

		al_symbols_.alcDestroyContext(context);
	}

	if (context_thread != nullptr && context_thread->thread->is_joinable())
	{
		context_thread->thread->join();
	}
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcDestroyContext);
	}
}

ALCcontext* ALC_APIENTRY AlApiImpl::alcGetCurrentContext() noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcGetCurrentContext();
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetCurrentContext);
	}

	return nullptr;
}

ALCdevice* ALC_APIENTRY AlApiImpl::alcGetContextsDevice(ALCcontext* context) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcGetContextsDevice(context);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetContextsDevice);
	}

	return nullptr;
}

ALCdevice* ALC_APIENTRY AlApiImpl::alcOpenDevice(const ALCchar* devicename) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ > log_level_none)
	{
		logger_.info("");
		logger_.info(al_api::Strings::equals_line_16);

		if (devicename != nullptr)
		{
			string_buffer_1_.clear();
			string_buffer_1_ += "Open device \"";
			string_buffer_1_ += devicename;
			string_buffer_1_ += "\".";
			logger_.info(string_buffer_1_.c_str());
		}
		else
		{
			logger_.info("Open default device.");
		}
	}

	const auto al_device = al_symbols_.alcOpenDevice(devicename);

	if (al_device == nullptr)
	{
		fail("AL failed to open the device.");
	}

	const auto effective_device_name = al_symbols_.alcGetString(al_device, ALC_DEVICE_SPECIFIER);

	if (log_level_ > log_level_none)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += "Effective name: \"";
		string_buffer_1_ += (effective_device_name != nullptr ? effective_device_name : null_string);
		string_buffer_1_ += "\".";
		logger_.info(string_buffer_1_.c_str());

		string_buffer_1_.clear();
		string_buffer_1_ += "Instance: ";
		string_buffer_1_ += to_string_hex(al_device, string_buffer_2_);
		logger_.info(string_buffer_1_.c_str());
	}

	if (effective_device_name == nullptr)
	{
		fail("Null effective name.");
	}

#ifdef NDEBUG
	const auto effective_device_name_sv = std::string_view{effective_device_name};
	constexpr auto reference_device_name_sv = std::string_view{"SB X-Fi Audio [0001]"};

	if (effective_device_name_sv != reference_device_name_sv)
	{
		fail("Expected X-Fi Titanium device name.");
	}
#endif

	devices_.emplace_back(Device{});
	auto& device = devices_.back();
	device.al_device = al_device;
	return al_device;
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcOpenDevice);
	}

	return nullptr;
}

ALCboolean ALC_APIENTRY AlApiImpl::alcCloseDevice(ALCdevice* device) noexcept
try
{
	using Threads = std::vector<ThreadUPtr>;
	auto threads = Threads{};

	{
		const auto lock = get_lock();

		if (log_level_ > log_level_none)
		{
			logger_.info("");
			logger_.info(al_api::Strings::equals_line_16);

			string_buffer_1_.clear();
			string_buffer_1_ += "Close device ";
			string_buffer_1_ += ptr_to_str(device, string_buffer_2_);
			string_buffer_1_ += '.';
			logger_.info(string_buffer_1_.c_str());
		}

		if (device != nullptr)
		{
			const auto al_device = find_device(device);

			if (al_device != nullptr)
			{
				if (!al_device->buffer_map.empty())
				{
					if (log_level_ >= log_level_warning)
					{
						logger_.warning("The device has buffers.");
					}
				}

				if (!al_device->contexts.empty())
				{
					if (log_level_ >= log_level_warning)
					{
						logger_.warning("The device has contexts.");
					}
				}
			}
			else
			{
				if (log_level_ >= log_level_warning)
				{
					logger_.warning("Unregistered device.");
				}
			}
		}

		const auto al_result = al_symbols_.alcCloseDevice(device);

		if (al_result == ALC_FALSE)
		{
			fail("AL failed to close the device.");
		}

		auto context_count = std::size_t{};

		for (const auto& our_device : devices_)
		{
			if (our_device.al_device != device)
			{
				continue;
			}

			context_count += our_device.contexts.size();
		}

		threads.reserve(context_count);

		devices_.remove_if(
			[al_device = device, &threads](Device& device)
			{
				if (device.al_device != al_device)
				{
					return false;
				}

				for (auto& context : device.contexts)
				{
					if (context.context_thread == nullptr || context.context_thread->thread == nullptr)
					{
						continue;
					}

					context.thread_quit_flag = true;
					threads.emplace_back(std::move(context.context_thread->thread));
				}

				return true;
			}
		);
	}

	for (auto& thread : threads)
	{
		if (thread->is_joinable())
		{
			thread->join();
		}
	}

	return ALC_TRUE;
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCloseDevice);
	}

	return ALC_FALSE;
}

ALCenum ALC_APIENTRY AlApiImpl::alcGetError(ALCdevice* device) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcGetError(device);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetError);
	}

	return ALC_INVALID_DEVICE;
}

ALCboolean ALC_APIENTRY AlApiImpl::alcIsExtensionPresent(ALCdevice* device, const ALCchar* extname) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcIsExtensionPresent(device, extname);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcIsExtensionPresent);
	}

	return AL_FALSE;
}

void* ALC_APIENTRY AlApiImpl::alcGetProcAddress(ALCdevice*, const ALCchar* funcname) noexcept
try
{
	const auto lock = get_lock();
	const auto symbol = get_alc_symbol(funcname);

	if (symbol != nullptr)
	{
		return symbol;
	}

	string_buffer_1_.clear();
	string_buffer_1_ += "Symbol \"";
	string_buffer_1_ += funcname;
	string_buffer_1_ += "\" not found.";
	fail(string_buffer_1_.c_str());
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetProcAddress);
	}

	return nullptr;
}

ALCenum ALC_APIENTRY AlApiImpl::alcGetEnumValue(ALCdevice* device, const ALCchar* enumname) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcGetEnumValue(device, enumname);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetEnumValue);
	}

	return AL_NONE;
}

const ALCchar* ALC_APIENTRY AlApiImpl::alcGetString(ALCdevice* device, ALCenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcGetString(device, param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetString);
	}

	return nullptr;
}

void ALC_APIENTRY AlApiImpl::alcGetIntegerv(ALCdevice* device, ALCenum param, ALCsizei size, ALCint* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcGetIntegerv(device, param, size, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcGetIntegerv);
	}
}

ALCdevice* ALC_APIENTRY AlApiImpl::alcCaptureOpenDevice(
	const ALCchar* devicename,
	ALCuint frequency,
	ALCenum format,
	ALCsizei buffersize) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcCaptureOpenDevice(devicename, frequency, format, buffersize);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCaptureOpenDevice);
	}

	return nullptr;
}

ALCboolean ALC_APIENTRY AlApiImpl::alcCaptureCloseDevice(ALCdevice* device) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alcCaptureCloseDevice(device);;
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCaptureCloseDevice);
	}

	return ALC_FALSE;
}

void ALC_APIENTRY AlApiImpl::alcCaptureStart(ALCdevice* device) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcCaptureStart(device);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCaptureStart);
	}
}

void ALC_APIENTRY AlApiImpl::alcCaptureStop(ALCdevice* device) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcCaptureStop(device);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCaptureStop);
	}
}

void ALC_APIENTRY AlApiImpl::alcCaptureSamples(ALCdevice* device, ALCvoid* buffer, ALCsizei samples) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alcCaptureSamples(device, buffer, samples);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alcCaptureSamples);
	}
}

void AL_APIENTRY AlApiImpl::alDopplerFactor(ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alDopplerFactor(value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDopplerFactor);
	}
}

void AL_APIENTRY AlApiImpl::alDopplerVelocity(ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alDopplerVelocity(value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDopplerVelocity);
	}
}

void AL_APIENTRY AlApiImpl::alSpeedOfSound(ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alSpeedOfSound(value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSpeedOfSound);
	}
}

void AL_APIENTRY AlApiImpl::alDistanceModel(ALenum distanceModel) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alDistanceModel(distanceModel);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDistanceModel);
	}
}

void AL_APIENTRY AlApiImpl::alEnable(ALenum capability) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alEnable(capability);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alEnable);
	}
}

void AL_APIENTRY AlApiImpl::alDisable(ALenum capability) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alDisable(capability);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDisable);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsEnabled(ALenum capability) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alIsEnabled(capability);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsEnabled);
	}

	return AL_FALSE;
}

const ALchar* AL_APIENTRY AlApiImpl::alGetString(ALenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetString(param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetString);
	}

	return nullptr;
}

void AL_APIENTRY AlApiImpl::alGetBooleanv(ALenum param, ALboolean* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetBooleanv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBooleanv);
	}
}

void AL_APIENTRY AlApiImpl::alGetIntegerv(ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetIntegerv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetIntegerv);
	}
}

void AL_APIENTRY AlApiImpl::alGetFloatv(ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetFloatv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFloatv);
	}
}

void AL_APIENTRY AlApiImpl::alGetDoublev(ALenum param, ALdouble* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetDoublev(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetDoublev);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alGetBoolean(ALenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetBoolean(param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBoolean);
	}

	return AL_FALSE;
}

ALint AL_APIENTRY AlApiImpl::alGetInteger(ALenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetInteger(param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetInteger);
	}

	return 0;
}

ALfloat AL_APIENTRY AlApiImpl::alGetFloat(ALenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetFloat(param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFloat);
	}

	return 0.0F;
}

ALdouble AL_APIENTRY AlApiImpl::alGetDouble(ALenum param) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetDouble(param);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetDouble);
	}

	return 0.0;
}

ALenum AL_APIENTRY AlApiImpl::alGetError() noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alGetError();
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetError);
	}

	return AL_INVALID_OPERATION;
}

ALboolean AL_APIENTRY AlApiImpl::alIsExtensionPresent(const ALchar* extname) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alIsExtensionPresent(extname);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsExtensionPresent);
	}

	return AL_FALSE;
}

void* AL_APIENTRY AlApiImpl::alGetProcAddress(const ALchar* fname) noexcept
try
{
	const auto lock = get_lock();
	auto symbol = get_al_symbol(fname);

	if (symbol != nullptr)
	{
		return symbol;
	}

	if (current_context_ != nullptr)
	{
		symbol = get_symbol(current_context_->efx_symbol_map, fname);

		if (symbol != nullptr)
		{
			return symbol;
		}

		symbol = get_symbol(current_context_->eax_symbol_map, fname);

		if (symbol != nullptr)
		{
			return symbol;
		}
	}

	string_buffer_1_.clear();
	string_buffer_1_ += "Symbol \"";
	string_buffer_1_ += fname;
	string_buffer_1_ += "\" not found.";
	fail(string_buffer_1_.c_str());
}
catch (...)
{
	if (log_level_ > log_level_none)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetProcAddress);
	}

	return nullptr;
}

ALenum AL_APIENTRY AlApiImpl::alGetEnumValue(const ALchar* ename) noexcept
try
{
	return al_symbols_.alGetEnumValue(ename);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetEnumValue);
	}

	return 0;
}

void AL_APIENTRY AlApiImpl::alListenerf(ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListenerf(param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListenerf);
	}
}

void AL_APIENTRY AlApiImpl::alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListener3f(param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListener3f);
	}
}

void AL_APIENTRY AlApiImpl::alListenerfv(ALenum param, const ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListenerfv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListenerfv);
	}
}

void AL_APIENTRY AlApiImpl::alListeneri(ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListeneri(param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListeneri);
	}
}

void AL_APIENTRY AlApiImpl::alListener3i(ALenum param, ALint value1, ALint value2, ALint value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListener3i(param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListener3i);
	}
}

void AL_APIENTRY AlApiImpl::alListeneriv(ALenum param, const ALint* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alListeneriv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alListeneriv);
	}
}

void AL_APIENTRY AlApiImpl::alGetListenerf(ALenum param, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListenerf(param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListenerf);
	}
}

void AL_APIENTRY AlApiImpl::alGetListener3f(
	ALenum param,
	ALfloat* value1,
	ALfloat* value2,
	ALfloat* value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListener3f(param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListener3f);
	}
}

void AL_APIENTRY AlApiImpl::alGetListenerfv(ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListenerfv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListenerfv);
	}
}

void AL_APIENTRY AlApiImpl::alGetListeneri(ALenum param, ALint* value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListeneri(param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListeneri);
	}
}

void AL_APIENTRY AlApiImpl::alGetListener3i(ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListener3i(param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListener3i);
	}
}

void AL_APIENTRY AlApiImpl::alGetListeneriv(ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetListeneriv(param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetListeneriv);
	}
}

void AL_APIENTRY AlApiImpl::alGenSources(ALsizei n, ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alGenSources;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alGenSources(n, sources);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to generate the sources.");
	}

	if (n <= 0 || sources == nullptr)
	{
		return;
	}

	auto& context = get_context();
	const auto sources_span = make_span(sources, n);

	for (const auto source : sources_span)
	{
		context.source_map.emplace(source, Source{});
		auto& our_source = context.source_map[source];
		our_source.id = source;
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGenSources);
	}
}

void AL_APIENTRY AlApiImpl::alDeleteSources(ALsizei n, const ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alDeleteSources;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alDeleteSources(n, sources);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to delete the sources.");
	}

	if (n <= 0 || sources == nullptr)
	{
		return;
	}

	auto& source_map = get_context().source_map;
	const auto sources_span = make_span(sources, n);

	for (const auto source : sources_span)
	{
		source_map.erase(source);
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDeleteSources);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsSource(ALuint source) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alIsSource(source);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsSource);
	}

	return AL_FALSE;
}

void AL_APIENTRY AlApiImpl::alSourcef(ALuint source, ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcef;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(value, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcef(source, param, value);
	handle_al_source_fx(source, param, &value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcef);
	}
}

void AL_APIENTRY AlApiImpl::alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alSource3f(source, param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSource3f);
	}
}

void AL_APIENTRY AlApiImpl::alSourcefv(ALuint source, ALenum param, const ALfloat* values) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcefv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(values, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcefv(source, param, values);
	handle_al_source_fx(source, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcefv);
	}
}

void AL_APIENTRY AlApiImpl::alSourcei(ALuint source, ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcei;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(value, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcei(source, param, value);
	handle_al_source_ix(source, param, &value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcei);
	}
}

void AL_APIENTRY AlApiImpl::alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alSource3i(source, param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSource3i);
	}
}

void AL_APIENTRY AlApiImpl::alSourceiv(ALuint source, ALenum param, const ALint* values) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceiv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(values, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceiv(source, param, values);
	handle_al_source_ix(source, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceiv);
	}
}

void AL_APIENTRY AlApiImpl::alGetSourcef(ALuint source, ALenum param, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetSourcef(source, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSourcef);
	}
}

void AL_APIENTRY AlApiImpl::alGetSource3f(ALuint source, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetSource3f(source, param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSource3f);
	}
}

void AL_APIENTRY AlApiImpl::alGetSourcefv(ALuint source, ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetSourcefv(source, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSourcefv);
	}
}

void AL_APIENTRY AlApiImpl::alGetSourcei(ALuint source, ALenum param, ALint* value) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alGetSourcei;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(value, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alGetSourcei(source, param, value);
	handle_al_get_source_ix(source, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSourcei);
	}
}

void AL_APIENTRY AlApiImpl::alGetSource3i(ALuint source, ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetSource3i(source, param, value1, value2, value3);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSource3i);
	}
}

void AL_APIENTRY AlApiImpl::alGetSourceiv(ALuint source, ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alGetSourceiv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += source_param_to_string(param, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(values, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alGetSourceiv(source, param, values);
	handle_al_get_source_ix(source, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetSourceiv);
	}
}

void AL_APIENTRY AlApiImpl::alSourcePlayv(ALsizei n, const ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcePlayv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcePlayv(n, sources);
	handle_al_source_state(n, sources, AL_PLAYING);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcePlayv);
	}
}

void AL_APIENTRY AlApiImpl::alSourceStopv(ALsizei n, const ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceStopv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceStopv(n, sources);
	handle_al_source_state(n, sources, AL_STOPPED);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceStopv);
	}
}

void AL_APIENTRY AlApiImpl::alSourceRewindv(ALsizei n, const ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceRewindv;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceRewindv(n, sources);
	handle_al_source_state(n, sources, AL_INITIAL);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceRewindv);
	}
}

void AL_APIENTRY AlApiImpl::alSourcePausev(ALsizei n, const ALuint* sources) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcePausev;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(sources, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcePausev(n, sources);
	handle_al_source_state(n, sources, AL_PAUSED);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcePausev);
	}
}

void AL_APIENTRY AlApiImpl::alSourcePlay(ALuint source) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcePlay;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcePlay(source);
	handle_al_source_state(1, &source, AL_PLAYING);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcePlay);
	}
}

void AL_APIENTRY AlApiImpl::alSourceStop(ALuint source) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceStop;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceStop(source);
	handle_al_source_state(1, &source, AL_STOPPED);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceStop);
	}
}

void AL_APIENTRY AlApiImpl::alSourceRewind(ALuint source) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceRewind;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceRewind(source);
	handle_al_source_state(1, &source, AL_INITIAL);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceRewind);
	}
}

void AL_APIENTRY AlApiImpl::alSourcePause(ALuint source) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourcePause;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourcePause(source);
	handle_al_source_state(1, &source, AL_PAUSED);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourcePause);
	}
}

void AL_APIENTRY AlApiImpl::alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint* buffers) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceQueueBuffers;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(nb, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(buffers, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceQueueBuffers(source, nb, buffers);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to queue the buffers.");
	}

	if (nb <= 0 || buffers == nullptr)
	{
		return;
	}

	auto& our_source = get_source(source);
	our_source.is_streaming = true;
	auto& queue = our_source.queue;
	const auto is_queue_was_empty = queue.is_empty();

	auto buffers_span = make_span(buffers, nb);

	for (const auto buffer : buffers_span)
	{
		auto& our_buffer = get_buffer(buffer);
		queue.push(&our_buffer);
	}

	if (is_queue_was_empty)
	{
		our_source.r_rate = 1.0 / queue.get_front()->rate;
	}

	update_source_monitoring(our_source);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceQueueBuffers);
	}
}

void AL_APIENTRY AlApiImpl::alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint* buffers) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alSourceUnqueueBuffers;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(source, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(nb, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(buffers, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alSourceUnqueueBuffers(source, nb, buffers);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to unqueue the buffers.");
	}

	if (nb <= 0 || buffers == nullptr)
	{
		return;
	}

	auto& our_source = get_source(source);
	our_source.queue.pop(nb);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alSourceUnqueueBuffers);
	}
}

void AL_APIENTRY AlApiImpl::alGenBuffers(ALsizei n, ALuint* buffers) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alGenBuffers;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(buffers, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alGenBuffers(n, buffers);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to generate the buffers.");
	}

	if (n <= 0 || buffers == nullptr)
	{
		return;
	}

	auto& buffer_map = get_device().buffer_map;

	const auto buffers_span = make_span(buffers, n);

	for (const auto buffer : buffers_span)
	{
		buffer_map.emplace(buffer, Buffer{});
	}

	auto& context = get_context();

	if (context.has_xram)
	{
		const auto xram_result = context.xram_symbols.EAXSetBufferMode(
			n,
			buffers,
			context.xram_al_storage_accessible);

		if (xram_result == AL_FALSE)
		{
			if (log_level_ >= log_level_warning)
			{
				logger_.warning("AL failed to set X-RAM mode to ACCESSIBLE.");
			}
		}
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGenBuffers);
	}
}

void AL_APIENTRY AlApiImpl::alDeleteBuffers(ALsizei n, const ALuint* buffers) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alDeleteBuffers;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(n, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(buffers, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	al_symbols_.alGetError();
	al_symbols_.alDeleteBuffers(n, buffers);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to delete the buffers.");
	}

	auto& buffer_map = get_device().buffer_map;
	const auto buffers_span = make_span(buffers, n);

	for (const auto buffer : buffers_span)
	{
		buffer_map.erase(buffer);
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDeleteBuffers);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsBuffer(ALuint buffer) noexcept
try
{
	const auto lock = get_lock();
	return al_symbols_.alIsBuffer(buffer);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsBuffer);
	}

	return AL_FALSE;
}

void AL_APIENTRY AlApiImpl::alBufferData(
	ALuint buffer,
	ALenum format,
	const ALvoid* data,
	ALsizei size,
	ALsizei freq) noexcept
try
{
	const auto lock = get_lock();

	if (log_level_ >= log_level_trace)
	{
		string_buffer_1_.clear();
		string_buffer_1_ += AlSymbolsNames::alBufferData;
		string_buffer_1_ += '(';
		string_buffer_1_ += to_string(buffer, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += buffer_format_to_string(format, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += ptr_to_str(data, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(size, string_buffer_2_);
		string_buffer_1_ += ", ";
		string_buffer_1_ += to_string(freq, string_buffer_2_);
		string_buffer_1_ += ')';
		logger_.info(string_buffer_1_.c_str());
	}

	auto frame_size = 0;

	switch (format)
	{
		case AL_FORMAT_MONO8:
			frame_size = 1;
			break;

		case AL_FORMAT_MONO16:
			frame_size = 2;
			break;

		case AL_FORMAT_STEREO8:
			frame_size = 2;
			break;

		case AL_FORMAT_STEREO16:
			frame_size = 4;
			break;

		default:
			set_al_invalid_enum();
			fail("Unknown format.");
	}

	al_symbols_.alGetError();
	al_symbols_.alBufferData(buffer, format, data, size, freq);

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		set_al_invalid_enum();
		fail("AL failed to fill a buffer with data.");
	}

	auto& our_buffer = get_buffer(buffer);
	const auto frame_count = size / frame_size;
	our_buffer.frame_size = frame_size;
	our_buffer.rate = freq;
	our_buffer.frame_count = frame_count;
	our_buffer.duration_ms = static_cast<int>((frame_count * 1'000LL) / freq);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBufferData);
	}
}

void AL_APIENTRY AlApiImpl::alBufferf(ALuint buffer, ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alBufferf != nullptr)
	{
		al_symbols_.alBufferf(buffer, param, value);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBufferf);
	}
}

void AL_APIENTRY AlApiImpl::alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alBuffer3f != nullptr)
	{
		al_symbols_.alBuffer3f(buffer, param, value1, value2, value3);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBuffer3f);
	}
}

void AL_APIENTRY AlApiImpl::alBufferfv(ALuint buffer, ALenum param, const ALfloat* values) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alBufferfv != nullptr)
	{
		al_symbols_.alBufferfv(buffer, param, values);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBufferfv);
	}
}

void AL_APIENTRY AlApiImpl::alBufferi(ALuint buffer, ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alBufferi(buffer, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBufferi);
	}
}

void AL_APIENTRY AlApiImpl::alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alBuffer3i != nullptr)
	{
		al_symbols_.alBuffer3i(buffer, param, value1, value2, value3);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBuffer3i);
	}
}

void AL_APIENTRY AlApiImpl::alBufferiv(ALuint buffer, ALenum param, const ALint* values) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alBufferiv != nullptr)
	{
		al_symbols_.alBufferiv(buffer, param, values);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alBufferiv);
	}
}

void AL_APIENTRY AlApiImpl::alGetBufferf(ALuint buffer, ALenum param, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetBufferf(buffer, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBufferf);
	}
}

void AL_APIENTRY AlApiImpl::alGetBuffer3f(ALuint buffer, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alGetBuffer3f != nullptr)
	{
		al_symbols_.alGetBuffer3f(buffer, param, value1, value2, value3);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBuffer3f);
	}
}

void AL_APIENTRY AlApiImpl::alGetBufferfv(ALuint buffer, ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alGetBufferfv != nullptr)
	{
		al_symbols_.alGetBufferfv(buffer, param, values);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBufferfv);
	}
}

void AL_APIENTRY AlApiImpl::alGetBufferi(ALuint buffer, ALenum param, ALint* value) noexcept
try
{
	const auto lock = get_lock();
	al_symbols_.alGetBufferi(buffer, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBufferi);
	}
}

void AL_APIENTRY AlApiImpl::alGetBuffer3i(ALuint buffer, ALenum param, ALint* value1, ALint* value2, ALint* value3) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alGetBuffer3i != nullptr)
	{
		al_symbols_.alGetBuffer3i(buffer, param, value1, value2, value3);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBuffer3i);
	}
}

void AL_APIENTRY AlApiImpl::alGetBufferiv(ALuint buffer, ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();

	if (al_symbols_.alGetBufferiv != nullptr)
	{
		al_symbols_.alGetBufferiv(buffer, param, values);
	}
	else
	{
		set_al_invalid_enum();
	}
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetBufferiv);
	}
}

void AL_APIENTRY AlApiImpl::alGenEffects(ALsizei n, ALuint* effects) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGenEffects(n, effects);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGenEffects);
	}
}

void AL_APIENTRY AlApiImpl::alDeleteEffects(ALsizei n, ALuint* effects) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alDeleteEffects(n, effects);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDeleteEffects);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsEffect(ALuint eid) noexcept
try
{
	const auto lock = get_lock();
	return get_context().efx_symbols.alIsEffect(eid);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsEffect);
	}

	return AL_FALSE;
}

void AL_APIENTRY AlApiImpl::alEffecti(ALuint eid, ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alEffecti(eid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alEffecti);
	}
}

void AL_APIENTRY AlApiImpl::alEffectiv(ALuint eid, ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alEffectiv(eid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alEffectiv);
	}
}

void AL_APIENTRY AlApiImpl::alEffectf(ALuint eid, ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alEffectf(eid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alEffectf);
	}
}

void AL_APIENTRY AlApiImpl::alEffectfv(ALuint eid, ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alEffectfv(eid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alEffectfv);
	}
}

void AL_APIENTRY AlApiImpl::alGetEffecti(ALuint eid, ALenum pname, ALint* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetEffecti(eid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetEffecti);
	}
}

void AL_APIENTRY AlApiImpl::alGetEffectiv(ALuint eid, ALenum pname, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetEffectiv(eid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetEffectiv);
	}
}

void AL_APIENTRY AlApiImpl::alGetEffectf(ALuint eid, ALenum pname, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetEffectf(eid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetEffectf);
	}
}

void AL_APIENTRY AlApiImpl::alGetEffectfv(ALuint eid, ALenum pname, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetEffectfv(eid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetEffectfv);
	}
}

void AL_APIENTRY AlApiImpl::alGenFilters(ALsizei n, ALuint* filters) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGenFilters(n, filters);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGenFilters);
	}
}

void AL_APIENTRY AlApiImpl::alDeleteFilters(ALsizei n, ALuint* filters) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alDeleteFilters(n, filters);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDeleteFilters);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsFilter(ALuint fid) noexcept
try
{
	const auto lock = get_lock();
	return get_context().efx_symbols.alIsFilter(fid);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsFilter);
	}

	return AL_FALSE;
}

void AL_APIENTRY AlApiImpl::alFilteri(ALuint fid, ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alFilteri(fid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alFilteri);
	}
}

void AL_APIENTRY AlApiImpl::alFilteriv(ALuint fid, ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alFilteriv(fid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alFilteriv);
	}
}

void AL_APIENTRY AlApiImpl::alFilterf(ALuint fid, ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alFilterf(fid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alFilterf);
	}
}

void AL_APIENTRY AlApiImpl::alFilterfv(ALuint fid, ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alFilterfv(fid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alFilterfv);
	}
}

void AL_APIENTRY AlApiImpl::alGetFilteri(ALuint fid, ALenum pname, ALint* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetFilteri(fid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFilteri);
	}
}

void AL_APIENTRY AlApiImpl::alGetFilteriv(ALuint fid, ALenum pname, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetFilteriv(fid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFilteriv);
	}
}

void AL_APIENTRY AlApiImpl::alGetFilterf(ALuint fid, ALenum pname, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetFilterf(fid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFilterf);
	}
}

void AL_APIENTRY AlApiImpl::alGetFilterfv(ALuint fid, ALenum pname, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetFilterfv(fid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetFilterfv);
	}
}

void AL_APIENTRY AlApiImpl::alGenAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGenAuxiliaryEffectSlots(n, slots);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGenAuxiliaryEffectSlots);
	}
}

void AL_APIENTRY AlApiImpl::alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alDeleteAuxiliaryEffectSlots(n, slots);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alDeleteAuxiliaryEffectSlots);
	}
}

ALboolean AL_APIENTRY AlApiImpl::alIsAuxiliaryEffectSlot(ALuint slot) noexcept
try
{
	const auto lock = get_lock();
	return get_context().efx_symbols.alIsAuxiliaryEffectSlot(slot);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alIsAuxiliaryEffectSlot);
	}

	return AL_FALSE;
}

void AL_APIENTRY AlApiImpl::alAuxiliaryEffectSloti(ALuint asid, ALenum param, ALint value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alAuxiliaryEffectSloti(asid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alAuxiliaryEffectSloti);
	}
}

void AL_APIENTRY AlApiImpl::alAuxiliaryEffectSlotiv(ALuint asid, ALenum param, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alAuxiliaryEffectSlotiv(asid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alAuxiliaryEffectSlotiv);
	}
}

void AL_APIENTRY AlApiImpl::alAuxiliaryEffectSlotf(ALuint asid, ALenum param, ALfloat value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alAuxiliaryEffectSlotf(asid, param, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alAuxiliaryEffectSlotf);
	}
}

void AL_APIENTRY AlApiImpl::alAuxiliaryEffectSlotfv(ALuint asid, ALenum param, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alAuxiliaryEffectSlotfv(asid, param, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alAuxiliaryEffectSlotfv);
	}
}

void AL_APIENTRY AlApiImpl::alGetAuxiliaryEffectSloti(ALuint asid, ALenum pname, ALint* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetAuxiliaryEffectSloti(asid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetAuxiliaryEffectSloti);
	}
}

void AL_APIENTRY AlApiImpl::alGetAuxiliaryEffectSlotiv(ALuint asid, ALenum pname, ALint* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetAuxiliaryEffectSlotiv(asid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetAuxiliaryEffectSlotiv);
	}
}

void AL_APIENTRY AlApiImpl::alGetAuxiliaryEffectSlotf(ALuint asid, ALenum pname, ALfloat* value) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetAuxiliaryEffectSlotf(asid, pname, value);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetAuxiliaryEffectSlotf);
	}
}

void AL_APIENTRY AlApiImpl::alGetAuxiliaryEffectSlotfv(ALuint asid, ALenum pname, ALfloat* values) noexcept
try
{
	const auto lock = get_lock();
	get_context().efx_symbols.alGetAuxiliaryEffectSlotfv(asid, pname, values);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, AlSymbolsNames::alGetAuxiliaryEffectSlotfv);
	}
}

ALenum AL_APIENTRY AlApiImpl::EAXSet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept
try
{
	const auto lock = get_lock();
	return get_context().eax_symbols.EAXSet(psid, pid, sid, data, size);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, EaxSymbolsNames::EAXSet);
	}

	return AL_INVALID_OPERATION;
}

ALenum AL_APIENTRY AlApiImpl::EAXGet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept
try
{
	const auto lock = get_lock();
	return get_context().eax_symbols.EAXGet(psid, pid, sid, data, size);
}
catch (...)
{
	if (log_level_ >= log_level_error)
	{
		utils::log_exception(logger_, EaxSymbolsNames::EAXGet);
	}

	return AL_INVALID_OPERATION;
}

[[noreturn]] void AlApiImpl::fail(const char* message)
{
	throw AlApiException{message};
}

void AlApiImpl::initialize_logger()
{
	log_level_ = log_level_none;

	try
	{
		const auto log_level_string = env::get_var("XFITSSFIX_LOG_LEVEL");

		if (log_level_string.size() == 1)
		{
			const auto log_level_char = log_level_string.front();

			switch (log_level_char)
			{
				case '0':
					log_level_ = log_level_none;
					break;

				case '1':
					log_level_ = log_level_error;
					break;

				case '2':
					log_level_ = log_level_warning;
					break;

				case '3':
					log_level_ = log_level_trace;
					break;

				default:
					break;
			}
		}
	}
	catch (...)
	{
	}

	constexpr auto log_file_name = "xfitssfix_log.txt";

	auto logger_param = LoggerParam{};

	if (log_level_ > log_level_none)
	{
		logger_param.file_path = log_file_name;
	}

	logger_.make(logger_param);

	if (log_level_ > log_level_none && !logger_.has_file())
	{
		try
		{
			auto log_path = env::get_special_folder(env::SpecialFolderType::app_data);
			log_path /= "bibendovsky";
			fs::create_directory(log_path.get_data());
			log_path /= "xfitssfix";
			fs::create_directory(log_path.get_data());
			log_path /= log_file_name;
			logger_param.file_path = log_path.get_data();
			logger_.make(logger_param);
		}
		catch (...)
		{
		}
	}

	if (log_level_ > log_level_none)
	{
		logger_.info("");
		logger_.info("<<<<<<<<<<<<<<<<<<<<<<<<");
		logger_.info("XFITSSFIX v" XFITSSFIX_VERSION);
		logger_.info("<<<<<<<<<<<<<<<<<<<<<<<<");

		string_buffer_1_.clear();
		string_buffer_1_ += "Process ID: ";
		string_buffer_1_ += to_string(process_id_, string_buffer_2_);
		logger_.info(string_buffer_1_.c_str());

		logger_.info("");
	}
}

const char* AlApiImpl::get_source_param_string(ALenum param) noexcept
{
	switch (param)
	{
		case AL_PITCH: return "AL_PITCH";
		case AL_GAIN: return "AL_GAIN";
		case AL_MAX_DISTANCE: return "AL_MAX_DISTANCE";
		case AL_ROLLOFF_FACTOR: return "AL_ROLLOFF_FACTOR";
		case AL_REFERENCE_DISTANCE: return "AL_REFERENCE_DISTANCE";
		case AL_MIN_GAIN: return "AL_MIN_GAIN";
		case AL_MAX_GAIN: return "AL_MAX_GAIN";
		case AL_CONE_OUTER_GAIN: return "AL_CONE_OUTER_GAIN";
		case AL_CONE_INNER_ANGLE: return "AL_CONE_INNER_ANGLE";
		case AL_CONE_OUTER_ANGLE: return "AL_CONE_OUTER_ANGLE";
		case AL_POSITION: return "AL_POSITION";
		case AL_VELOCITY: return "AL_VELOCITY";
		case AL_DIRECTION: return "AL_DIRECTION";
		case AL_SOURCE_RELATIVE: return "AL_SOURCE_RELATIVE";
		case AL_SOURCE_TYPE: return "AL_SOURCE_TYPE";
		case AL_LOOPING: return "AL_LOOPING";
		case AL_BUFFER: return "AL_BUFFER";
		case AL_SOURCE_STATE: return "AL_SOURCE_STATE";
		case AL_BUFFERS_QUEUED: return "AL_BUFFERS_QUEUED";
		case AL_BUFFERS_PROCESSED: return "AL_BUFFERS_PROCESSED";
		case AL_SEC_OFFSET: return "AL_SEC_OFFSET";
		case AL_SAMPLE_OFFSET: return "AL_SAMPLE_OFFSET";
		case AL_BYTE_OFFSET: return "AL_BYTE_OFFSET";
		default: return nullptr;
	}
}

String& AlApiImpl::source_param_to_string(ALenum param, String& string)
{
	const auto param_string = get_source_param_string(param);

	if (param_string != nullptr)
	{
		return string = param_string;
	}
	else
	{
		return to_string(param, string);
	}
}

const char* AlApiImpl::get_buffer_format_string(ALenum param) noexcept
{
	switch (param)
	{
		case AL_FORMAT_MONO8: return "AL_FORMAT_MONO8";
		case AL_FORMAT_MONO16: return "AL_FORMAT_MONO16";
		case AL_FORMAT_STEREO8: return "AL_FORMAT_STEREO8";
		case AL_FORMAT_STEREO16: return "AL_FORMAT_STEREO16";
		default: return nullptr;
	}
}

String& AlApiImpl::buffer_format_to_string(ALenum param, String& string)
{
	const auto param_string = get_buffer_format_string(param);

	if (param_string != nullptr)
	{
		return string = param_string;
	}
	else
	{
		return to_string(param, string);
	}
}

void AlApiImpl::initialize_al_driver()
{
	if (log_level_ > log_level_none)
	{
		logger_.info("Load AL driver.");
	}

	constexpr const char* const known_names[] =
	{
		"ct_oal.dll",
#ifndef NDEBUG
		"sens_oal.dll",
		"soft_oal.dll",
		"wrap_oal.dll",
#endif
	};

	for (const auto known_name : known_names)
	{
		try
		{
			if (log_level_ > log_level_none)
			{
				const auto message = "Try to load a driver \"" + String{known_name} + "\".";
				logger_.info(message.c_str());
			}

			al_library_ = make_shared_library(known_name);
			return;
		}
		catch (...)
		{
			if (log_level_ >= log_level_error)
			{
				utils::log_exception(logger_);
			}
		}
	}

	fail("Failed to load any suitable driver.");
}

void AlApiImpl::initialize_al_symbols()
{
	if (log_level_ > log_level_none)
	{
		logger_.info("Create AL symbol loader.");
	}

	al_loader_ = make_al_loader(al_library_.get());

	if (log_level_ > log_level_none)
	{
		logger_.info("Load AL symbols.");
	}

	al_loader_->resolve_al_symbols(al_symbols_);
}

void AlApiImpl::initialize_alc_symbol_map()
{
	constexpr auto capacity = sizeof(AlSymbols) / sizeof(void*);

	alc_symbol_map_.clear();
	alc_symbol_map_.reserve(capacity);

	alc_symbol_map_.emplace(AlSymbolsNames::alcCreateContext, reinterpret_cast<void*>(::alcCreateContext));
	alc_symbol_map_.emplace(AlSymbolsNames::alcMakeContextCurrent, reinterpret_cast<void*>(::alcMakeContextCurrent));
	alc_symbol_map_.emplace(AlSymbolsNames::alcProcessContext, reinterpret_cast<void*>(::alcProcessContext));
	alc_symbol_map_.emplace(AlSymbolsNames::alcSuspendContext, reinterpret_cast<void*>(::alcSuspendContext));
	alc_symbol_map_.emplace(AlSymbolsNames::alcDestroyContext, reinterpret_cast<void*>(::alcDestroyContext));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetCurrentContext, reinterpret_cast<void*>(::alcGetCurrentContext));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetContextsDevice, reinterpret_cast<void*>(::alcGetContextsDevice));
	alc_symbol_map_.emplace(AlSymbolsNames::alcOpenDevice, reinterpret_cast<void*>(::alcOpenDevice));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCloseDevice, reinterpret_cast<void*>(::alcCloseDevice));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetError, reinterpret_cast<void*>(::alcGetError));
	alc_symbol_map_.emplace(AlSymbolsNames::alcIsExtensionPresent, reinterpret_cast<void*>(::alcIsExtensionPresent));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetProcAddress, reinterpret_cast<void*>(::alcGetProcAddress));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetEnumValue, reinterpret_cast<void*>(::alcGetEnumValue));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetString, reinterpret_cast<void*>(::alcGetString));
	alc_symbol_map_.emplace(AlSymbolsNames::alcGetIntegerv, reinterpret_cast<void*>(::alcGetIntegerv));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCaptureOpenDevice, reinterpret_cast<void*>(::alcCaptureOpenDevice));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCaptureCloseDevice, reinterpret_cast<void*>(::alcCaptureCloseDevice));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCaptureStart, reinterpret_cast<void*>(::alcCaptureStart));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCaptureStop, reinterpret_cast<void*>(::alcCaptureStop));
	alc_symbol_map_.emplace(AlSymbolsNames::alcCaptureSamples, reinterpret_cast<void*>(::alcCaptureSamples));
}

void AlApiImpl::initialize_al_symbol_map()
{
	constexpr auto capacity = sizeof(AlSymbols) / sizeof(void*);

	al_symbol_map_.clear();
	al_symbol_map_.reserve(capacity);

	al_symbol_map_.emplace(AlSymbolsNames::alcCreateContext, reinterpret_cast<void*>(::alcCreateContext));
	al_symbol_map_.emplace(AlSymbolsNames::alcMakeContextCurrent, reinterpret_cast<void*>(::alcMakeContextCurrent));
	al_symbol_map_.emplace(AlSymbolsNames::alcProcessContext, reinterpret_cast<void*>(::alcProcessContext));
	al_symbol_map_.emplace(AlSymbolsNames::alcSuspendContext, reinterpret_cast<void*>(::alcSuspendContext));
	al_symbol_map_.emplace(AlSymbolsNames::alcDestroyContext, reinterpret_cast<void*>(::alcDestroyContext));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetCurrentContext, reinterpret_cast<void*>(::alcGetCurrentContext));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetContextsDevice, reinterpret_cast<void*>(::alcGetContextsDevice));
	al_symbol_map_.emplace(AlSymbolsNames::alcOpenDevice, reinterpret_cast<void*>(::alcOpenDevice));
	al_symbol_map_.emplace(AlSymbolsNames::alcCloseDevice, reinterpret_cast<void*>(::alcCloseDevice));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetError, reinterpret_cast<void*>(::alcGetError));
	al_symbol_map_.emplace(AlSymbolsNames::alcIsExtensionPresent, reinterpret_cast<void*>(::alcIsExtensionPresent));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetProcAddress, reinterpret_cast<void*>(::alcGetProcAddress));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetEnumValue, reinterpret_cast<void*>(::alcGetEnumValue));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetString, reinterpret_cast<void*>(::alcGetString));
	al_symbol_map_.emplace(AlSymbolsNames::alcGetIntegerv, reinterpret_cast<void*>(::alcGetIntegerv));
	al_symbol_map_.emplace(AlSymbolsNames::alcCaptureOpenDevice, reinterpret_cast<void*>(::alcCaptureOpenDevice));
	al_symbol_map_.emplace(AlSymbolsNames::alcCaptureCloseDevice, reinterpret_cast<void*>(::alcCaptureCloseDevice));
	al_symbol_map_.emplace(AlSymbolsNames::alcCaptureStart, reinterpret_cast<void*>(::alcCaptureStart));
	al_symbol_map_.emplace(AlSymbolsNames::alcCaptureStop, reinterpret_cast<void*>(::alcCaptureStop));
	al_symbol_map_.emplace(AlSymbolsNames::alcCaptureSamples, reinterpret_cast<void*>(::alcCaptureSamples));

	al_symbol_map_.emplace(AlSymbolsNames::alDopplerFactor, reinterpret_cast<void*>(::alDopplerFactor));
	al_symbol_map_.emplace(AlSymbolsNames::alDopplerVelocity, reinterpret_cast<void*>(::alDopplerVelocity));
	al_symbol_map_.emplace(AlSymbolsNames::alSpeedOfSound, reinterpret_cast<void*>(::alSpeedOfSound));
	al_symbol_map_.emplace(AlSymbolsNames::alDistanceModel, reinterpret_cast<void*>(::alDistanceModel));
	al_symbol_map_.emplace(AlSymbolsNames::alEnable, reinterpret_cast<void*>(::alEnable));
	al_symbol_map_.emplace(AlSymbolsNames::alDisable, reinterpret_cast<void*>(::alDisable));
	al_symbol_map_.emplace(AlSymbolsNames::alIsEnabled, reinterpret_cast<void*>(::alIsEnabled));
	al_symbol_map_.emplace(AlSymbolsNames::alGetString, reinterpret_cast<void*>(::alGetString));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBooleanv, reinterpret_cast<void*>(::alGetBooleanv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetIntegerv, reinterpret_cast<void*>(::alGetIntegerv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetFloatv, reinterpret_cast<void*>(::alGetFloatv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetDoublev, reinterpret_cast<void*>(::alGetDoublev));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBoolean, reinterpret_cast<void*>(::alGetBoolean));
	al_symbol_map_.emplace(AlSymbolsNames::alGetInteger, reinterpret_cast<void*>(::alGetInteger));
	al_symbol_map_.emplace(AlSymbolsNames::alGetFloat, reinterpret_cast<void*>(::alGetFloat));
	al_symbol_map_.emplace(AlSymbolsNames::alGetDouble, reinterpret_cast<void*>(::alGetDouble));
	al_symbol_map_.emplace(AlSymbolsNames::alGetError, reinterpret_cast<void*>(::alGetError));
	al_symbol_map_.emplace(AlSymbolsNames::alIsExtensionPresent, reinterpret_cast<void*>(::alIsExtensionPresent));
	al_symbol_map_.emplace(AlSymbolsNames::alGetProcAddress, reinterpret_cast<void*>(::alGetProcAddress));
	al_symbol_map_.emplace(AlSymbolsNames::alGetEnumValue, reinterpret_cast<void*>(::alGetEnumValue));
	al_symbol_map_.emplace(AlSymbolsNames::alListenerf, reinterpret_cast<void*>(::alListenerf));
	al_symbol_map_.emplace(AlSymbolsNames::alListener3f, reinterpret_cast<void*>(::alListener3f));
	al_symbol_map_.emplace(AlSymbolsNames::alListenerfv, reinterpret_cast<void*>(::alListenerfv));
	al_symbol_map_.emplace(AlSymbolsNames::alListeneri, reinterpret_cast<void*>(::alListeneri));
	al_symbol_map_.emplace(AlSymbolsNames::alListener3i, reinterpret_cast<void*>(::alListener3i));
	al_symbol_map_.emplace(AlSymbolsNames::alListeneriv, reinterpret_cast<void*>(::alListeneriv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListenerf, reinterpret_cast<void*>(::alGetListenerf));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListener3f, reinterpret_cast<void*>(::alGetListener3f));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListenerfv, reinterpret_cast<void*>(::alGetListenerfv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListeneri, reinterpret_cast<void*>(::alGetListeneri));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListener3i, reinterpret_cast<void*>(::alGetListener3i));
	al_symbol_map_.emplace(AlSymbolsNames::alGetListeneriv, reinterpret_cast<void*>(::alGetListeneriv));
	al_symbol_map_.emplace(AlSymbolsNames::alGenSources, reinterpret_cast<void*>(::alGenSources));
	al_symbol_map_.emplace(AlSymbolsNames::alDeleteSources, reinterpret_cast<void*>(::alDeleteSources));
	al_symbol_map_.emplace(AlSymbolsNames::alIsSource, reinterpret_cast<void*>(::alIsSource));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcef, reinterpret_cast<void*>(::alSourcef));
	al_symbol_map_.emplace(AlSymbolsNames::alSource3f, reinterpret_cast<void*>(::alSource3f));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcefv, reinterpret_cast<void*>(::alSourcefv));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcei, reinterpret_cast<void*>(::alSourcei));
	al_symbol_map_.emplace(AlSymbolsNames::alSource3i, reinterpret_cast<void*>(::alSource3i));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceiv, reinterpret_cast<void*>(::alSourceiv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSourcef, reinterpret_cast<void*>(::alGetSourcef));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSource3f, reinterpret_cast<void*>(::alGetSource3f));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSourcefv, reinterpret_cast<void*>(::alGetSourcefv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSourcei, reinterpret_cast<void*>(::alGetSourcei));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSource3i, reinterpret_cast<void*>(::alGetSource3i));
	al_symbol_map_.emplace(AlSymbolsNames::alGetSourceiv, reinterpret_cast<void*>(::alGetSourceiv));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcePlayv, reinterpret_cast<void*>(::alSourcePlayv));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceStopv, reinterpret_cast<void*>(::alSourceStopv));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceRewindv, reinterpret_cast<void*>(::alSourceRewindv));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcePausev, reinterpret_cast<void*>(::alSourcePausev));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcePlay, reinterpret_cast<void*>(::alSourcePlay));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceStop, reinterpret_cast<void*>(::alSourceStop));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceRewind, reinterpret_cast<void*>(::alSourceRewind));
	al_symbol_map_.emplace(AlSymbolsNames::alSourcePause, reinterpret_cast<void*>(::alSourcePause));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceQueueBuffers, reinterpret_cast<void*>(::alSourceQueueBuffers));
	al_symbol_map_.emplace(AlSymbolsNames::alSourceUnqueueBuffers, reinterpret_cast<void*>(::alSourceUnqueueBuffers));
	al_symbol_map_.emplace(AlSymbolsNames::alGenBuffers, reinterpret_cast<void*>(::alGenBuffers));
	al_symbol_map_.emplace(AlSymbolsNames::alDeleteBuffers, reinterpret_cast<void*>(::alDeleteBuffers));
	al_symbol_map_.emplace(AlSymbolsNames::alIsBuffer, reinterpret_cast<void*>(::alIsBuffer));
	al_symbol_map_.emplace(AlSymbolsNames::alBufferData, reinterpret_cast<void*>(::alBufferData));
	al_symbol_map_.emplace(AlSymbolsNames::alBufferf, reinterpret_cast<void*>(::alBufferf));
	al_symbol_map_.emplace(AlSymbolsNames::alBuffer3f, reinterpret_cast<void*>(::alBuffer3f));
	al_symbol_map_.emplace(AlSymbolsNames::alBufferfv, reinterpret_cast<void*>(::alBufferfv));
	al_symbol_map_.emplace(AlSymbolsNames::alBufferi, reinterpret_cast<void*>(::alBufferi));
	al_symbol_map_.emplace(AlSymbolsNames::alBuffer3i, reinterpret_cast<void*>(::alBuffer3i));
	al_symbol_map_.emplace(AlSymbolsNames::alBufferiv, reinterpret_cast<void*>(::alBufferiv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBufferf, reinterpret_cast<void*>(::alGetBufferf));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBuffer3f, reinterpret_cast<void*>(::alGetBuffer3f));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBufferfv, reinterpret_cast<void*>(::alGetBufferfv));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBufferi, reinterpret_cast<void*>(::alGetBufferi));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBuffer3i, reinterpret_cast<void*>(::alGetBuffer3i));
	al_symbol_map_.emplace(AlSymbolsNames::alGetBufferiv, reinterpret_cast<void*>(::alGetBufferiv));
}

void AlApiImpl::initialize_efx_symbol_map(const EfxSymbols& symbols, AlSymbolMap& map) noexcept
{
	map.reserve(sizeof(EfxSymbols) / sizeof(void*));
	try_map_al_symbol(EfxSymbolsNames::alGenEffects, ::alGenEffects, symbols.alGenEffects, map);
	try_map_al_symbol(EfxSymbolsNames::alDeleteEffects, ::alDeleteEffects, symbols.alDeleteEffects, map);
	try_map_al_symbol(EfxSymbolsNames::alIsEffect, ::alIsEffect, symbols.alIsEffect, map);
	try_map_al_symbol(EfxSymbolsNames::alEffecti, ::alEffecti, symbols.alEffecti, map);
	try_map_al_symbol(EfxSymbolsNames::alEffectiv, ::alEffectiv, symbols.alEffectiv, map);
	try_map_al_symbol(EfxSymbolsNames::alEffectf, ::alEffectf, symbols.alEffectf, map);
	try_map_al_symbol(EfxSymbolsNames::alEffectfv, ::alEffectfv, symbols.alEffectfv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetEffecti, ::alGetEffecti, symbols.alGetEffecti, map);
	try_map_al_symbol(EfxSymbolsNames::alGetEffectiv, ::alGetEffectiv, symbols.alGetEffectiv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetEffectf, ::alGetEffectf, symbols.alGetEffectf, map);
	try_map_al_symbol(EfxSymbolsNames::alGetEffectfv, ::alGetEffectfv, symbols.alGetEffectfv, map);
	try_map_al_symbol(EfxSymbolsNames::alGenFilters, ::alGenFilters, symbols.alGenFilters, map);
	try_map_al_symbol(EfxSymbolsNames::alDeleteFilters, ::alDeleteFilters, symbols.alDeleteFilters, map);
	try_map_al_symbol(EfxSymbolsNames::alIsFilter, ::alIsFilter, symbols.alIsFilter, map);
	try_map_al_symbol(EfxSymbolsNames::alFilteri, ::alFilteri, symbols.alFilteri, map);
	try_map_al_symbol(EfxSymbolsNames::alFilteriv, ::alFilteriv, symbols.alFilteriv, map);
	try_map_al_symbol(EfxSymbolsNames::alFilterf, ::alFilterf, symbols.alFilterf, map);
	try_map_al_symbol(EfxSymbolsNames::alFilterfv, ::alFilterfv, symbols.alFilterfv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetFilteri, ::alGetFilteri, symbols.alGetFilteri, map);
	try_map_al_symbol(EfxSymbolsNames::alGetFilteriv, ::alGetFilteriv, symbols.alGetFilteriv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetFilterf, ::alGetFilterf, symbols.alGetFilterf, map);
	try_map_al_symbol(EfxSymbolsNames::alGetFilterfv, ::alGetFilterfv, symbols.alGetFilterfv, map);
	try_map_al_symbol(EfxSymbolsNames::alGenAuxiliaryEffectSlots, ::alGenAuxiliaryEffectSlots, symbols.alGenAuxiliaryEffectSlots, map);
	try_map_al_symbol(EfxSymbolsNames::alDeleteAuxiliaryEffectSlots, ::alDeleteAuxiliaryEffectSlots, symbols.alDeleteAuxiliaryEffectSlots, map);
	try_map_al_symbol(EfxSymbolsNames::alIsAuxiliaryEffectSlot, ::alIsAuxiliaryEffectSlot, symbols.alIsAuxiliaryEffectSlot, map);
	try_map_al_symbol(EfxSymbolsNames::alAuxiliaryEffectSloti, ::alAuxiliaryEffectSloti, symbols.alAuxiliaryEffectSloti, map);
	try_map_al_symbol(EfxSymbolsNames::alAuxiliaryEffectSlotiv, ::alAuxiliaryEffectSlotiv, symbols.alAuxiliaryEffectSlotiv, map);
	try_map_al_symbol(EfxSymbolsNames::alAuxiliaryEffectSlotf, ::alAuxiliaryEffectSlotf, symbols.alAuxiliaryEffectSlotf, map);
	try_map_al_symbol(EfxSymbolsNames::alAuxiliaryEffectSlotfv, ::alAuxiliaryEffectSlotfv, symbols.alAuxiliaryEffectSlotfv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetAuxiliaryEffectSloti, ::alGetAuxiliaryEffectSloti, symbols.alGetAuxiliaryEffectSloti, map);
	try_map_al_symbol(EfxSymbolsNames::alGetAuxiliaryEffectSlotiv, ::alGetAuxiliaryEffectSlotiv, symbols.alGetAuxiliaryEffectSlotiv, map);
	try_map_al_symbol(EfxSymbolsNames::alGetAuxiliaryEffectSlotf, ::alGetAuxiliaryEffectSlotf, symbols.alGetAuxiliaryEffectSlotf, map);
	try_map_al_symbol(EfxSymbolsNames::alGetAuxiliaryEffectSlotfv, ::alGetAuxiliaryEffectSlotfv, symbols.alGetAuxiliaryEffectSlotfv, map);
}

void AlApiImpl::initialize_efx(Context& context)
{
	if (log_level_ > log_level_none)
	{
		logger_.info("Load EFX symbols.");
	}

	al_loader_->resolve_efx_symbols(context.efx_symbols);
	initialize_efx_symbol_map(context.efx_symbols, context.efx_symbol_map);
}

void AlApiImpl::initialize_xram(Context& context)
{
	if (log_level_ > log_level_none)
	{
		logger_.info("Initialize X-RAM.");
	}

	context.has_xram = false;

	if (al_symbols_.alIsExtensionPresent("EAX-RAM") == AL_FALSE)
	{
		if (log_level_ > log_level_none)
		{
			logger_.warning("X-RAM extension string not found.");
		}

		return;
	}

	al_loader_->resolve_xram_symbols(context.xram_symbols);

	if (context.xram_symbols.EAXSetBufferMode == nullptr ||
		context.xram_symbols.EAXGetBufferMode == nullptr)
	{
		if (log_level_ > log_level_none)
		{
			logger_.warning("Missing X-RAM symbol(s).");
		}

		return;
	}

	constexpr auto al_storage_accessible_name = "AL_STORAGE_ACCESSIBLE";
	context.xram_al_storage_accessible = al_symbols_.alGetEnumValue(al_storage_accessible_name);

	if (context.xram_al_storage_accessible == AL_NONE)
	{
		al_symbols_.alGetError();

		if (log_level_ > log_level_none)
		{
			string_buffer_1_.clear();
			string_buffer_1_ += "Missing X-RAM enum value for ";
			string_buffer_1_ += al_storage_accessible_name;
			string_buffer_1_ += '.';
			logger_.warning(string_buffer_1_.c_str());
		}

		return;
	}

	context.has_xram = true;

	if (log_level_ > log_level_none)
	{
		logger_.info("X-RAM initialized.");
	}
}

void AlApiImpl::initialize_eax_symbol_map(const EaxSymbols& symbols, AlSymbolMap& map) noexcept
{
	map.reserve(sizeof(EaxSymbols) / sizeof(void*));
	try_map_al_symbol(EaxSymbolsNames::EAXSet, ::EAXSet, symbols.EAXSet, map);
	try_map_al_symbol(EaxSymbolsNames::EAXGet, ::EAXGet, symbols.EAXGet, map);
}

void AlApiImpl::initialize_eax(Context& context)
{
	if (log_level_ > log_level_none)
	{
		logger_.info("Load EAX symbols.");
	}

	al_loader_->resolve_eax_symbols(context.eax_symbols);
	initialize_eax_symbol_map(context.eax_symbols, context.eax_symbol_map);
}

MoveableMutexLock AlApiImpl::initialize_invalid_state()
{
	fail(ErrorMessages::invalid_state);
}

MoveableMutexLock AlApiImpl::initialize_not_initialized()
{
	fail(ErrorMessages::not_initialized);
}

MoveableMutexLock AlApiImpl::initialize_get_lock()
{
	assert(mutex_);
	return MoveableMutexLock{*mutex_};
}

MoveableMutexLock AlApiImpl::initialize_try_to_initialize()
try
{
	assert(mutex_);
	auto mt_lock = MoveableMutexLock{*mutex_};
	process_id_ = process::get_current_id();
	string_buffer_1_.reserve(min_string_buffer_capacity);
	string_buffer_2_.reserve(min_string_buffer_capacity);
	initialize_logger();
	initialize_al_driver();
	initialize_al_symbols();
	initialize_alc_symbol_map();
	initialize_al_symbol_map();
	initialize_func_ = &AlApiImpl::initialize_get_lock;
	return mt_lock;
}
catch (...)
{
	initialize_func_ = &AlApiImpl::initialize_not_initialized;
	throw;
}

MoveableMutexLock AlApiImpl::initialize()
{
	assert(initialize_func_);
	return (this->*initialize_func_)();
}

MoveableMutexLock AlApiImpl::get_lock()
{
	return initialize();
}

AlApiImpl::Device* AlApiImpl::find_device(ALCdevice* al_device) noexcept
{
	for (auto& device : devices_)
	{
		if (device.al_device == al_device)
		{
			return &device;
		}
	}

	return nullptr;
}

AlApiImpl::Device& AlApiImpl::get_device()
{
	auto& context = get_context();
	assert(context.device != nullptr);
	return *context.device;
}

AlApiImpl::Device& AlApiImpl::get_device(ALCdevice* al_device)
{
	const auto device = find_device(al_device);

	if (device == nullptr)
	{
		fail("Device not found.");
	}

	return *device;
}

AlApiImpl::Context& AlApiImpl::get_context()
{
	if (current_context_ == nullptr)
	{
		fail("No current context.");
	}

	return *current_context_;
}

AlApiImpl::Context& AlApiImpl::get_context(ALCcontext* al_context)
{
	if (al_context == nullptr)
	{
		fail("Null context.");
	}

	for (auto& device : devices_)
	{
		for (auto& context : device.contexts)
		{
			if (context.al_context == al_context)
			{
				return context;
			}
		}
	}

	fail("Unregistered context.");
}

AlApiImpl::Buffer& AlApiImpl::get_buffer(ALuint bid)
{
	auto& device = get_device();
	auto& buffer_map = device.buffer_map;
	const auto buffer_it = buffer_map.find(bid);

	if (buffer_it == buffer_map.end())
	{
		string_buffer_1_.clear();
		string_buffer_1_ += "Unmapped buffer ";
		string_buffer_1_ += to_string(bid, string_buffer_2_);
		string_buffer_1_ += '.';
		fail(string_buffer_1_.c_str());
	}

	return buffer_it->second;
}

AlApiImpl::Source& AlApiImpl::get_source(ALuint sid)
{
	auto& context = get_context();
	auto& source_map = context.source_map;
	const auto source_it = source_map.find(sid);

	if (source_it == source_map.end())
	{
		string_buffer_1_.clear();
		string_buffer_1_ += "Unmapped source ";
		string_buffer_1_ += to_string(sid, string_buffer_2_);
		string_buffer_1_ += '.';
		fail(string_buffer_1_.c_str());
	}

	return source_it->second;
}

void AlApiImpl::set_al_invalid_enum()
{
	assert(al_symbols_.alGetError != nullptr);
	al_symbols_.alGetError();
	assert(al_symbols_.alGetString != nullptr);
	al_symbols_.alGetString(-1);
}

void* AlApiImpl::get_symbol(const AlSymbolMap& al_symbol_map, const ALchar* name)
{
	if (name == nullptr)
	{
		fail("Null symbol name.");
	}

	const auto name_view = std::string_view{name};
	const auto symbol_map_it = al_symbol_map.find(name_view);

	if (symbol_map_it == al_symbol_map.cend())
	{
		return nullptr;
	}

	return symbol_map_it->second;
}

void* AlApiImpl::get_alc_symbol(const ALchar* name) const
{
	return get_symbol(alc_symbol_map_, name);
}

void* AlApiImpl::get_al_symbol(const ALchar* name) const
{
	return get_symbol(al_symbol_map_, name);
}

void AlApiImpl::mark_source_as_monitoring(Context& context, Source& source)
{
	context.monitoring_sources.insert(&source);
}

void AlApiImpl::mark_source_as_non_monitoring(Context& context, Source& source)
{
	context.monitoring_sources.erase(&source);
}

void AlApiImpl::update_source_monitoring(Source& source)
{
	auto processed = ALint{-1};
	al_symbols_.alGetSourcei(source.id, AL_BUFFERS_PROCESSED, &processed);

	if (processed < 0)
	{
		fail("Failed to get source's processed buffer count.");
	}

	auto offset = ALint{-1};
	al_symbols_.alGetSourcei(source.id, AL_SAMPLE_OFFSET, &offset);
	
	if (offset < 0)
	{
		fail("Failed to get source's sample offset.");
	}

	auto total_frame_count = 0;
	auto& queue = source.queue;

	std::for_each(queue.begin() + processed, queue.end(),
		[&total_frame_count](Buffer* buffer)
		{
			total_frame_count += (buffer != nullptr ? buffer->frame_count : 0);
		}
	);

	constexpr auto extra_delay = std::chrono::milliseconds{5};
	source.monitoring_time_point = Clock::now() + extra_delay;
	const auto remain_frame_count = total_frame_count - offset;

	if (remain_frame_count > 0)
	{
		const auto delay_ms = static_cast<int>(remain_frame_count * source.r_pitch_1000 * source.r_rate);
		const auto delay = std::chrono::milliseconds{delay_ms};
		source.monitoring_time_point += delay;
	}
}

void AlApiImpl::handle_al_source_ix(ALuint sid, ALenum param, const ALint* values)
{
	// alSourcei(v)

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		fail("Failed to set integer param.");
	}

	if (param == AL_BUFFER)
	{
		auto& source = get_source(sid);
		source.is_streaming = false;
		const auto bid = *values;

		if (bid != AL_NONE)
		{
			auto& buffer = get_buffer(bid);
			source.queue.push(&buffer);
			source.r_rate = 1.0 / buffer.rate;
		}
		else
		{
			source.queue.clear();
			source.r_rate = 0.0;
		}
	}
	else if (param == AL_LOOPING)
	{
		auto& context = get_context();
		auto& source = get_source(sid);
		const auto is_looping = ((*values) == AL_TRUE);

		if (is_looping)
		{
			source.is_looping = true;
			mark_source_as_non_monitoring(context, source);
		}
		else
		{
			source.is_looping = false;

			if (source.is_monitorable())
			{
				update_source_monitoring(source);
				mark_source_as_monitoring(context, source);
			}
		}
	}
}

void AlApiImpl::handle_al_source_fx(ALuint sid, ALenum param, const ALfloat* values)
{
	// alSourcef(v)

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		fail("Failed to set floating-point param.");
	}

	if (param == AL_PITCH)
	{
		auto& source = get_source(sid);
		const auto pitch = *values;
		source.r_pitch_1000 = 1'000.0 / pitch;

		if (source.is_monitorable())
		{
			update_source_monitoring(source);
		}
	}
}

void AlApiImpl::handle_al_get_source_ix(ALuint sid, ALenum param, ALint* values)
{
	// alGetSourcei(v)

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		fail("Failed to get integer parameter.");
	}

	if (param != AL_SOURCE_STATE)
	{
		return;
	}

	const auto state = *values;

	if (state == AL_PLAYING)
	{
		return;
	}

	auto& context = get_context();
	auto& source = get_source(sid);
	source.is_playing = false;
	mark_source_as_non_monitoring(context, source);
}

void AlApiImpl::handle_al_source_state(ALsizei ns, const ALuint *sids, ALenum state)
{
	// alSourcePlay(v), alSourcePause(v), alSourceStop(v), alSourceRewind(v)

	if (al_symbols_.alGetError() != AL_NO_ERROR)
	{
		fail("Failed to set source state.");
	}

	if (ns <= 0 || sids == nullptr)
	{
		return;
	}

	const auto sids_span = make_span(sids, ns);
	auto& context = get_context();

	if (state != AL_PLAYING)
	{
		// Pause, stop or rewind issued.

		for (const auto sid : sids_span)
		{
			auto& source = get_source(sid);
			source.is_playing = false;
			mark_source_as_non_monitoring(context, source);
		}

		return;
	}

	// Play issued.

	for (const auto sid : sids_span)
	{
		auto& source = get_source(sid);
		source.is_playing = true;

		if (source.is_looping)
		{
			mark_source_as_non_monitoring(context, source);
		}
		else
		{
			update_source_monitoring(source);
			mark_source_as_monitoring(context, source);
		}
	}
}

void AlApiImpl::handle_monitoring_source(MonitoringContext& monitoring_context)
{
	auto& context = *monitoring_context.context;
	auto& source = *monitoring_context.source;
	const auto current_time_point = monitoring_context.time_point;

	if (source.monitoring_time_point < current_time_point)
	{
		return;
	}

	const auto was_error = (al_symbols_.alGetError() != AL_NO_ERROR);

	const auto error_sentinel = [was_error, this]()
	{
		al_symbols_.alGetError();

		if (!was_error)
		{
			return;
		}

		set_al_invalid_enum();
	};

	ALint source_state;
	al_symbols_.alGetSourcei(source.id, AL_SOURCE_STATE, &source_state);

	if (source_state != AL_PLAYING)
	{
		source.is_playing = false;
		mark_source_as_non_monitoring(context, source);
		return;
	}

	if (log_level_ >= log_level_warning)
	{
		auto& string_buffer_1 = *monitoring_context.string_buffer_1;
		auto& string_buffer_2 = *monitoring_context.string_buffer_2;
		string_buffer_1.clear();
		string_buffer_1 += "Stuck ";
		string_buffer_1 += (source.is_streaming ? "streaming" : "static");
		string_buffer_1 += " source ";
		string_buffer_1 += to_string(source.id, string_buffer_2);
		string_buffer_1 += '.';
		logger_.warning(string_buffer_1.c_str());
	}

	if (source.is_streaming)
	{
		al_symbols_.alSourcePause(source.id);
		al_symbols_.alSourcePlay(source.id);
		update_source_monitoring(source);
	}
	else
	{
		al_symbols_.alSourceStop(source.id);
		source.is_playing = false;
		mark_source_as_non_monitoring(context, source);
	}
}

void AlApiImpl::handle_monitoring_sources(MonitoringContext& monitoring_context)
{
	if (current_context_ != monitoring_context.context)
	{
		return;
	}

	auto& monitoring_sources = *monitoring_context.monitoring_sources;

	monitoring_sources.clear();
	monitoring_sources.insert(monitoring_sources.cbegin(), monitoring_sources.cend());

	monitoring_context.time_point = Clock::now();

	for (auto source_ptr : monitoring_sources)
	{
		monitoring_context.source = source_ptr;
		handle_monitoring_source(monitoring_context);
	}
}

void AlApiImpl::thread_func_proxy(void* user_data)
{
	assert(user_data != nullptr);
	auto& context_thread = *static_cast<ContextThread*>(user_data);
	context_thread.al_api->thread_func(context_thread);
}

void AlApiImpl::thread_func(ContextThread& context_thread)
{
	auto string_buffer_1 = String{};
	string_buffer_1.reserve(256);

	auto string_buffer_2 = String{};
	string_buffer_2.reserve(256);

	auto monitoring_sources = MonitoringSources{};
	monitoring_sources.reserve(sources_capacity);

	{
		const auto lock = get_lock();

		if (log_level_ > log_level_none)
		{
			string_buffer_1.clear();
			string_buffer_1 += "Monitoring thread ";
			string_buffer_1 += to_string_hex(context_thread.thread.get(), string_buffer_2);
			string_buffer_1 += " started.";
			logger_.info(string_buffer_1.c_str());
		}
	}

	constexpr auto sleep_duration_ms = 10;

	auto& context = *context_thread.context;

	auto monitoring_context = MonitoringContext{};
	monitoring_context.context = context_thread.context;
	monitoring_context.monitoring_sources = &monitoring_sources;
	monitoring_context.string_buffer_1 = &string_buffer_1;
	monitoring_context.string_buffer_2 = &string_buffer_2;

	while (true)
	{
		{
			const auto lock = get_lock();

			if (context_thread.quit_flag)
			{
				if (log_level_ > log_level_none)
				{
					string_buffer_1.clear();
					string_buffer_1 += "Quitting the monitoring thread ";
					string_buffer_1 += to_string_hex(context_thread.thread.get(), string_buffer_2);
					string_buffer_1 += '.';
					logger_.info(string_buffer_1.c_str());
				}

				break;
			}

			if (!context.monitoring_sources.empty())
			{
				handle_monitoring_sources(monitoring_context);
			}
		}

		this_thread::sleep_for_ms(sleep_duration_ms);
	}
}

// ==========================================================================

AlApiImpl g_al_api_impl{};
AlApi& g_al_api = g_al_api_impl;

// ==========================================================================

void on_thread_detach() noexcept
{
	g_al_api_impl.on_thread_detach();
}

void on_process_detach() noexcept
{
	g_al_api_impl.on_process_detach();
}

} // namespace xfitssfix

// ==========================================================================

extern "C"
{

ALC_API ALCcontext* ALC_APIENTRY alcCreateContext(ALCdevice* device, const ALCint* attrlist)
{
	return xfitssfix::g_al_api_impl.alcCreateContext(device, attrlist);
}

ALC_API ALCboolean ALC_APIENTRY alcMakeContextCurrent(ALCcontext* context)
{
	return xfitssfix::g_al_api_impl.alcMakeContextCurrent(context);
}

ALC_API void ALC_APIENTRY alcProcessContext(ALCcontext* context)
{
	xfitssfix::g_al_api_impl.alcProcessContext(context);
}

ALC_API void ALC_APIENTRY alcSuspendContext(ALCcontext* context)
{
	xfitssfix::g_al_api_impl.alcSuspendContext(context);
}

ALC_API void ALC_APIENTRY alcDestroyContext(ALCcontext* context)
{
	xfitssfix::g_al_api_impl.alcDestroyContext(context);
}

ALC_API ALCcontext* ALC_APIENTRY alcGetCurrentContext()
{
	return xfitssfix::g_al_api_impl.alcGetCurrentContext();
}

ALC_API ALCdevice* ALC_APIENTRY alcGetContextsDevice(ALCcontext* context)
{
	return xfitssfix::g_al_api_impl.alcGetContextsDevice(context);
}

ALC_API ALCdevice* ALC_APIENTRY alcOpenDevice(const ALCchar* devicename)
{
	return xfitssfix::g_al_api_impl.alcOpenDevice(devicename);
}

ALC_API ALCboolean ALC_APIENTRY alcCloseDevice(ALCdevice* device)
{
	return xfitssfix::g_al_api_impl.alcCloseDevice(device);
}

ALC_API ALCenum ALC_APIENTRY alcGetError(ALCdevice* device)
{
	return xfitssfix::g_al_api_impl.alcGetError(device);
}

ALC_API ALCboolean ALC_APIENTRY alcIsExtensionPresent(ALCdevice* device, const ALCchar* extname)
{
	return xfitssfix::g_al_api_impl.alcIsExtensionPresent(device, extname);
}

ALC_API void* ALC_APIENTRY alcGetProcAddress(ALCdevice* device, const ALCchar* funcname)
{
	return xfitssfix::g_al_api_impl.alcGetProcAddress(device, funcname);
}

ALC_API ALCenum ALC_APIENTRY alcGetEnumValue(ALCdevice* device, const ALCchar* enumname)
{
	return xfitssfix::g_al_api_impl.alcGetEnumValue(device, enumname);
}

ALC_API const ALCchar* ALC_APIENTRY alcGetString(ALCdevice* device, ALCenum param)
{
	return xfitssfix::g_al_api_impl.alcGetString(device, param);
}

ALC_API void ALC_APIENTRY alcGetIntegerv(ALCdevice* device, ALCenum param, ALCsizei size, ALCint* values)
{
	xfitssfix::g_al_api_impl.alcGetIntegerv(device, param, size, values);
}

ALC_API ALCdevice* ALC_APIENTRY alcCaptureOpenDevice(
	const ALCchar* devicename,
	ALCuint frequency,
	ALCenum format,
	ALCsizei buffersize)
{
	return xfitssfix::g_al_api_impl.alcCaptureOpenDevice(devicename, frequency, format, buffersize);
}

ALC_API ALCboolean ALC_APIENTRY alcCaptureCloseDevice(ALCdevice* device)
{
	return xfitssfix::g_al_api_impl.alcCaptureCloseDevice(device);
}

ALC_API void ALC_APIENTRY alcCaptureStart(ALCdevice* device)
{
	xfitssfix::g_al_api_impl.alcCaptureStart(device);
}

ALC_API void ALC_APIENTRY alcCaptureStop(ALCdevice* device)
{
	xfitssfix::g_al_api_impl.alcCaptureStop(device);
}

ALC_API void ALC_APIENTRY alcCaptureSamples(ALCdevice* device, ALCvoid* buffer, ALCsizei samples)
{
	xfitssfix::g_al_api_impl.alcCaptureSamples(device, buffer, samples);
}

AL_API void AL_APIENTRY alDopplerFactor(ALfloat value)
{
	xfitssfix::g_al_api_impl.alDopplerFactor(value);
}

AL_API void AL_APIENTRY alDopplerVelocity(ALfloat value)
{
	xfitssfix::g_al_api_impl.alDopplerVelocity(value);
}

AL_API void AL_APIENTRY alSpeedOfSound(ALfloat value)
{
	xfitssfix::g_al_api_impl.alSpeedOfSound(value);
}

AL_API void AL_APIENTRY alDistanceModel(ALenum distanceModel)
{
	xfitssfix::g_al_api_impl.alDistanceModel(distanceModel);
}

AL_API void AL_APIENTRY alEnable(ALenum capability)
{
	xfitssfix::g_al_api_impl.alEnable(capability);
}

AL_API void AL_APIENTRY alDisable(ALenum capability)
{
	xfitssfix::g_al_api_impl.alDisable(capability);
}

AL_API ALboolean AL_APIENTRY alIsEnabled(ALenum capability)
{
	return xfitssfix::g_al_api_impl.alIsEnabled(capability);
}

AL_API const ALchar* AL_APIENTRY alGetString(ALenum param)
{
	return xfitssfix::g_al_api_impl.alGetString(param);
}

AL_API void AL_APIENTRY alGetBooleanv(ALenum param, ALboolean* values)
{
	xfitssfix::g_al_api_impl.alGetBooleanv(param, values);
}

AL_API void AL_APIENTRY alGetIntegerv(ALenum param, ALint* values)
{
	xfitssfix::g_al_api_impl.alGetIntegerv(param, values);
}

AL_API void AL_APIENTRY alGetFloatv(ALenum param, ALfloat* values)
{
	xfitssfix::g_al_api_impl.alGetFloatv(param, values);
}

AL_API void AL_APIENTRY alGetDoublev(ALenum param, ALdouble* values)
{
	xfitssfix::g_al_api_impl.alGetDoublev(param, values);
}

AL_API ALboolean AL_APIENTRY alGetBoolean(ALenum param)
{
	return xfitssfix::g_al_api_impl.alGetBoolean(param);
}

AL_API ALint AL_APIENTRY alGetInteger(ALenum param)
{
	return xfitssfix::g_al_api_impl.alGetInteger(param);
}

AL_API ALfloat AL_APIENTRY alGetFloat(ALenum param)
{
	return xfitssfix::g_al_api_impl.alGetFloat(param);
}

AL_API ALdouble AL_APIENTRY alGetDouble(ALenum param)
{
	return xfitssfix::g_al_api_impl.alGetDouble(param);
}

AL_API ALenum AL_APIENTRY alGetError()
{
	return xfitssfix::g_al_api_impl.alGetError();
}

AL_API ALboolean AL_APIENTRY alIsExtensionPresent(const ALchar* extname)
{
	return xfitssfix::g_al_api_impl.alIsExtensionPresent(extname);
}

AL_API void* AL_APIENTRY alGetProcAddress(const ALchar* fname)
{
	return xfitssfix::g_al_api_impl.alGetProcAddress(fname);
}

AL_API ALenum AL_APIENTRY alGetEnumValue(const ALchar* ename)
{
	return xfitssfix::g_al_api_impl.alGetEnumValue(ename);
}

AL_API void AL_APIENTRY alListenerf(ALenum param, ALfloat value)
{
	xfitssfix::g_al_api_impl.alListenerf(param, value);
}

AL_API void AL_APIENTRY alListener3f(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
	xfitssfix::g_al_api_impl.alListener3f(param, value1, value2, value3);
}

AL_API void AL_APIENTRY alListenerfv(ALenum param, const ALfloat* values)
{
	xfitssfix::g_al_api_impl.alListenerfv(param, values);
}

AL_API void AL_APIENTRY alListeneri(ALenum param, ALint value)
{
	xfitssfix::g_al_api_impl.alListeneri(param, value);
}

AL_API void AL_APIENTRY alListener3i(ALenum param, ALint value1, ALint value2, ALint value3)
{
	xfitssfix::g_al_api_impl.alListener3i(param, value1, value2, value3);
}

AL_API void AL_APIENTRY alListeneriv(ALenum param, const ALint* values)
{
	xfitssfix::g_al_api_impl.alListeneriv(param, values);
}

AL_API void AL_APIENTRY alGetListenerf(ALenum param, ALfloat* value)
{
	xfitssfix::g_al_api_impl.alGetListenerf(param, value);
}

AL_API void AL_APIENTRY alGetListener3f(ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3)
{
	xfitssfix::g_al_api_impl.alGetListener3f(param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetListenerfv(ALenum param, ALfloat* values)
{
	xfitssfix::g_al_api_impl.alGetListenerfv(param, values);
}

AL_API void AL_APIENTRY alGetListeneri(ALenum param, ALint* value)
{
	xfitssfix::g_al_api_impl.alGetListeneri(param, value);
}

AL_API void AL_APIENTRY alGetListener3i(ALenum param, ALint* value1, ALint* value2, ALint* value3)
{
	xfitssfix::g_al_api_impl.alGetListener3i(param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetListeneriv(ALenum param, ALint* values)
{
	xfitssfix::g_al_api_impl.alGetListeneriv(param, values);
}

AL_API void AL_APIENTRY alGenSources(ALsizei n, ALuint* sources)
{
	xfitssfix::g_al_api_impl.alGenSources(n, sources);
}

AL_API void AL_APIENTRY alDeleteSources(ALsizei n, const ALuint* sources)
{
	xfitssfix::g_al_api_impl.alDeleteSources(n, sources);
}

AL_API ALboolean AL_APIENTRY alIsSource(ALuint source)
{
	return xfitssfix::g_al_api_impl.alIsSource(source);
}

AL_API void AL_APIENTRY alSourcef(ALuint source, ALenum param, ALfloat value)
{
	xfitssfix::g_al_api_impl.alSourcef(source, param, value);
}

AL_API void AL_APIENTRY alSource3f(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
	xfitssfix::g_al_api_impl.alSource3f(source, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alSourcefv(ALuint source, ALenum param, const ALfloat* values)
{
	xfitssfix::g_al_api_impl.alSourcefv(source, param, values);
}

AL_API void AL_APIENTRY alSourcei(ALuint source, ALenum param, ALint value)
{
	xfitssfix::g_al_api_impl.alSourcei(source, param, value);
}

AL_API void AL_APIENTRY alSource3i(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3)
{
	xfitssfix::g_al_api_impl.alSource3i(source, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alSourceiv(ALuint source, ALenum param, const ALint* values)
{
	xfitssfix::g_al_api_impl.alSourceiv(source, param, values);
}

AL_API void AL_APIENTRY alGetSourcef(ALuint source, ALenum param, ALfloat* value)
{
	xfitssfix::g_al_api_impl.alGetSourcef(source, param, value);
}

AL_API void AL_APIENTRY alGetSource3f(ALuint source, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3)
{
	xfitssfix::g_al_api_impl.alGetSource3f(source, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetSourcefv(ALuint source, ALenum param, ALfloat* values)
{
	xfitssfix::g_al_api_impl.alGetSourcefv(source, param, values);
}

AL_API void AL_APIENTRY alGetSourcei(ALuint source, ALenum param, ALint* value)
{
	xfitssfix::g_al_api_impl.alGetSourcei(source, param, value);
}

AL_API void AL_APIENTRY alGetSource3i(ALuint source, ALenum param, ALint* value1, ALint* value2, ALint* value3)
{
	xfitssfix::g_al_api_impl.alGetSource3i(source, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetSourceiv(ALuint source, ALenum param, ALint* values)
{
	xfitssfix::g_al_api_impl.alGetSourceiv(source, param, values);
}

AL_API void AL_APIENTRY alSourcePlayv(ALsizei n, const ALuint* sources)
{
	xfitssfix::g_al_api_impl.alSourcePlayv(n, sources);
}

AL_API void AL_APIENTRY alSourceStopv(ALsizei n, const ALuint* sources)
{
	xfitssfix::g_al_api_impl.alSourceStopv(n, sources);
}

AL_API void AL_APIENTRY alSourceRewindv(ALsizei n, const ALuint* sources)
{
	xfitssfix::g_al_api_impl.alSourceRewindv(n, sources);
}

AL_API void AL_APIENTRY alSourcePausev(ALsizei n, const ALuint* sources)
{
	xfitssfix::g_al_api_impl.alSourcePausev(n, sources);
}

AL_API void AL_APIENTRY alSourcePlay(ALuint source)
{
	xfitssfix::g_al_api_impl.alSourcePlay(source);
}

AL_API void AL_APIENTRY alSourceStop(ALuint source)
{
	xfitssfix::g_al_api_impl.alSourceStop(source);
}

AL_API void AL_APIENTRY alSourceRewind(ALuint source)
{
	xfitssfix::g_al_api_impl.alSourceRewind(source);
}

AL_API void AL_APIENTRY alSourcePause(ALuint source)
{
	xfitssfix::g_al_api_impl.alSourcePause(source);
}

AL_API void AL_APIENTRY alSourceQueueBuffers(ALuint source, ALsizei nb, const ALuint* buffers)
{
	xfitssfix::g_al_api_impl.alSourceQueueBuffers(source, nb, buffers);
}

AL_API void AL_APIENTRY alSourceUnqueueBuffers(ALuint source, ALsizei nb, ALuint* buffers)
{
	xfitssfix::g_al_api_impl.alSourceUnqueueBuffers(source, nb, buffers);
}

AL_API void AL_APIENTRY alGenBuffers(ALsizei n, ALuint* buffers)
{
	xfitssfix::g_al_api_impl.alGenBuffers(n, buffers);
}

AL_API void AL_APIENTRY alDeleteBuffers(ALsizei n, const ALuint* buffers)
{
	xfitssfix::g_al_api_impl.alDeleteBuffers(n, buffers);
}

AL_API ALboolean AL_APIENTRY alIsBuffer(ALuint buffer)
{
	return xfitssfix::g_al_api_impl.alIsBuffer(buffer);
}

AL_API void AL_APIENTRY alBufferData(
	ALuint buffer,
	ALenum format,
	const ALvoid* data,
	ALsizei size,
	ALsizei freq)
{
	xfitssfix::g_al_api_impl.alBufferData(buffer, format, data, size, freq);
}

AL_API void AL_APIENTRY alBufferf(ALuint buffer, ALenum param, ALfloat value)
{
	xfitssfix::g_al_api_impl.alBufferf(buffer, param, value);
}

AL_API void AL_APIENTRY alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
	xfitssfix::g_al_api_impl.alBuffer3f(buffer, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alBufferfv(ALuint buffer, ALenum param, const ALfloat* values)
{
	xfitssfix::g_al_api_impl.alBufferfv(buffer, param, values);
}

AL_API void AL_APIENTRY alBufferi(ALuint buffer, ALenum param, ALint value)
{
	xfitssfix::g_al_api_impl.alBufferi(buffer, param, value);
}

AL_API void AL_APIENTRY alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3)
{
	xfitssfix::g_al_api_impl.alBuffer3i(buffer, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alBufferiv(ALuint buffer, ALenum param, const ALint* values)
{
	xfitssfix::g_al_api_impl.alBufferiv(buffer, param, values);
}

AL_API void AL_APIENTRY alGetBufferf(ALuint buffer, ALenum param, ALfloat* value)
{
	xfitssfix::g_al_api_impl.alGetBufferf(buffer, param, value);
}

AL_API void AL_APIENTRY alGetBuffer3f(ALuint buffer, ALenum param, ALfloat* value1, ALfloat* value2, ALfloat* value3)
{
	xfitssfix::g_al_api_impl.alGetBuffer3f(buffer, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetBufferfv(ALuint buffer, ALenum param, ALfloat* values)
{
	xfitssfix::g_al_api_impl.alGetBufferfv(buffer, param, values);
}

AL_API void AL_APIENTRY alGetBufferi(ALuint buffer, ALenum param, ALint* value)
{
	xfitssfix::g_al_api_impl.alGetBufferi(buffer, param, value);
}

AL_API void AL_APIENTRY alGetBuffer3i(ALuint buffer, ALenum param, ALint* value1, ALint* value2, ALint* value3)
{
	xfitssfix::g_al_api_impl.alGetBuffer3i(buffer, param, value1, value2, value3);
}

AL_API void AL_APIENTRY alGetBufferiv(ALuint buffer, ALenum param, ALint* values)
{
	xfitssfix::g_al_api_impl.alGetBufferiv(buffer, param, values);
}

} // extern "C"

namespace {

void AL_APIENTRY alGenEffects(ALsizei n, ALuint* effects) noexcept
{
	xfitssfix::g_al_api_impl.alGenEffects(n, effects);
}

void AL_APIENTRY alDeleteEffects(ALsizei n, ALuint* effects) noexcept
{
	xfitssfix::g_al_api_impl.alDeleteEffects(n, effects);
}

ALboolean AL_APIENTRY alIsEffect(ALuint eid) noexcept
{
	return xfitssfix::g_al_api_impl.alIsEffect(eid);
}

void AL_APIENTRY alEffecti(ALuint eid, ALenum param, ALint value) noexcept
{
	xfitssfix::g_al_api_impl.alEffecti(eid, param, value);
}

void AL_APIENTRY alEffectiv(ALuint eid, ALenum param, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alEffectiv(eid, param, values);
}

void AL_APIENTRY alEffectf(ALuint eid, ALenum param, ALfloat value) noexcept
{
	xfitssfix::g_al_api_impl.alEffectf(eid, param, value);
}

void AL_APIENTRY alEffectfv(ALuint eid, ALenum param, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alEffectfv(eid, param, values);
}

void AL_APIENTRY alGetEffecti(ALuint eid, ALenum pname, ALint* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetEffecti(eid, pname, value);
}

void AL_APIENTRY alGetEffectiv(ALuint eid, ALenum pname, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetEffectiv(eid, pname, values);
}

void AL_APIENTRY alGetEffectf(ALuint eid, ALenum pname, ALfloat* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetEffectf(eid, pname, value);
}

void AL_APIENTRY alGetEffectfv(ALuint eid, ALenum pname, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetEffectfv(eid, pname, values);
}

void AL_APIENTRY alGenFilters(ALsizei n, ALuint* filters) noexcept
{
	xfitssfix::g_al_api_impl.alGenFilters(n, filters);
}

void AL_APIENTRY alDeleteFilters(ALsizei n, ALuint* filters) noexcept
{
	xfitssfix::g_al_api_impl.alDeleteFilters(n, filters);
}

ALboolean AL_APIENTRY alIsFilter(ALuint fid) noexcept
{
	return xfitssfix::g_al_api_impl.alIsFilter(fid);
}

void AL_APIENTRY alFilteri(ALuint fid, ALenum param, ALint value) noexcept
{
	xfitssfix::g_al_api_impl.alFilteri(fid, param, value);
}

void AL_APIENTRY alFilteriv(ALuint fid, ALenum param, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alFilteriv(fid, param, values);
}

void AL_APIENTRY alFilterf(ALuint fid, ALenum param, ALfloat value) noexcept
{
	xfitssfix::g_al_api_impl.alFilterf(fid, param, value);
}

void AL_APIENTRY alFilterfv(ALuint fid, ALenum param, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alFilterfv(fid, param, values);
}

void AL_APIENTRY alGetFilteri(ALuint fid, ALenum pname, ALint* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetFilteri(fid, pname, value);
}

void AL_APIENTRY alGetFilteriv(ALuint fid, ALenum pname, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetFilteriv(fid, pname, values);
}

void AL_APIENTRY alGetFilterf(ALuint fid, ALenum pname, ALfloat* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetFilterf(fid, pname, value);
}

void AL_APIENTRY alGetFilterfv(ALuint fid, ALenum pname, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetFilterfv(fid, pname, values);
}

void AL_APIENTRY alGenAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept
{
	xfitssfix::g_al_api_impl.alGenAuxiliaryEffectSlots(n, slots);
}

void AL_APIENTRY alDeleteAuxiliaryEffectSlots(ALsizei n, ALuint* slots) noexcept
{
	xfitssfix::g_al_api_impl.alDeleteAuxiliaryEffectSlots(n, slots);
}

ALboolean AL_APIENTRY alIsAuxiliaryEffectSlot(ALuint slot) noexcept
{
	return xfitssfix::g_al_api_impl.alIsAuxiliaryEffectSlot(slot);
}

void AL_APIENTRY alAuxiliaryEffectSloti(ALuint asid, ALenum param, ALint value) noexcept
{
	xfitssfix::g_al_api_impl.alAuxiliaryEffectSloti(asid, param, value);
}

void AL_APIENTRY alAuxiliaryEffectSlotiv(ALuint asid, ALenum param, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alAuxiliaryEffectSlotiv(asid, param, values);
}

void AL_APIENTRY alAuxiliaryEffectSlotf(ALuint asid, ALenum param, ALfloat value) noexcept
{
	xfitssfix::g_al_api_impl.alAuxiliaryEffectSlotf(asid, param, value);
}

void AL_APIENTRY alAuxiliaryEffectSlotfv(ALuint asid, ALenum param, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alAuxiliaryEffectSlotfv(asid, param, values);
}

void AL_APIENTRY alGetAuxiliaryEffectSloti(ALuint asid, ALenum pname, ALint* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetAuxiliaryEffectSloti(asid, pname, value);
}

void AL_APIENTRY alGetAuxiliaryEffectSlotiv(ALuint asid, ALenum pname, ALint* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetAuxiliaryEffectSlotiv(asid, pname, values);
}

void AL_APIENTRY alGetAuxiliaryEffectSlotf(ALuint asid, ALenum pname, ALfloat* value) noexcept
{
	xfitssfix::g_al_api_impl.alGetAuxiliaryEffectSlotf(asid, pname, value);
}

void AL_APIENTRY alGetAuxiliaryEffectSlotfv(ALuint asid, ALenum pname, ALfloat* values) noexcept
{
	xfitssfix::g_al_api_impl.alGetAuxiliaryEffectSlotfv(asid, pname, values);
}

ALenum AL_APIENTRY EAXSet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept
{
	return xfitssfix::g_al_api_impl.EAXSet(psid, pid, sid, data, size);
}

ALenum AL_APIENTRY EAXGet(const void* psid, ALuint pid, ALuint sid, ALvoid* data, ALuint size) noexcept
{
	return xfitssfix::g_al_api_impl.EAXGet(psid, pid, sid, data, size);
}

} // namespace
