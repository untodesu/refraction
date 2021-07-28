/*
 * al.h
 * Created: 2021-07-26, 23:42:42.
 * Copyright (C) 2021, Kirill GPRB. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef AL_AL_H
#define AL_AL_H 1
#include <AL/salad.h>
#include <stdint.h>

#define AL_API      SALAD_ALX_API
#define AL_APIENTRY SALAD_ALX_APIENTRY

#define OPENAL      1
#define ALAPI       SALAD_ALX_API
#define ALAPIENTRY  SALAD_ALX_APIENTRY

#define AL_VERSION_1_0 1
#define AL_VERSION_1_1 1

typedef salad_uint8_t   ALboolean;
typedef char            ALchar;
typedef salad_float64_t ALdouble;
typedef salad_float32_t ALfloat;
typedef salad_int16_t   ALshort;
typedef salad_int32_t   ALint;
typedef salad_int32_t   ALsizei;
typedef salad_int8_t    ALbyte;
typedef salad_uint16_t  ALushort;
typedef salad_uint32_t  ALenum;
typedef salad_uint32_t  ALuint;
typedef salad_uint8_t   ALubyte;
typedef void            ALvoid;

#define AL_INVALID                      0xFFFFFFFF
#define AL_NONE                         0
#define AL_FALSE                        0
#define AL_TRUE                         1
#define AL_SOURCE_RELATIVE              0x0202
#define AL_CONE_INNER_ANGLE             0x1001
#define AL_CONE_OUTER_ANGLE             0x1002
#define AL_PITCH                        0x1003
#define AL_POSITION                     0x1004
#define AL_DIRECTION                    0x1005
#define AL_VELOCITY                     0x1006
#define AL_LOOPING                      0x1007
#define AL_BUFFER                       0x1009
#define AL_GAIN                         0x100A
#define AL_MIN_GAIN                     0x100D
#define AL_MAX_GAIN                     0x100E
#define AL_ORIENTATION                  0x100F
#define AL_CHANNEL_MASK                 0x3000
#define AL_SOURCE_STATE                 0x1010
#define AL_INITIAL                      0x1011
#define AL_PLAYING                      0x1012
#define AL_PAUSED                       0x1013
#define AL_STOPPED                      0x1014
#define AL_BUFFERS_QUEUED               0x1015
#define AL_BUFFERS_PROCESSED            0x1016
#define AL_SEC_OFFSET                   0x1024
#define AL_SAMPLE_OFFSET                0x1025
#define AL_BYTE_OFFSET                  0x1026
#define AL_SOURCE_TYPE                  0x1027
#define AL_STATIC                       0x1028
#define AL_STREAMING                    0x1029
#define AL_UNDETERMINED                 0x1030
#define AL_FORMAT_MONO8                 0x1100
#define AL_FORMAT_MONO16                0x1101
#define AL_FORMAT_STEREO8               0x1102
#define AL_FORMAT_STEREO16              0x1103
#define AL_REFERENCE_DISTANCE           0x1020
#define AL_ROLLOFF_FACTOR               0x1021
#define AL_CONE_OUTER_GAIN              0x1022
#define AL_MAX_DISTANCE                 0x1023
#define AL_FREQUENCY                    0x2001
#define AL_BITS                         0x2002
#define AL_CHANNELS                     0x2003
#define AL_SIZE                         0x2004
#define AL_UNUSED                       0x2010
#define AL_PENDING                      0x2011
#define AL_PROCESSED                    0x2012
#define AL_NO_ERROR                     0x0000
#define AL_INVALID_NAME                 0xA001
#define AL_INVALID_ENUM                 0xA002
#define AL_INVALID_VALUE                0xA003
#define AL_INVALID_OPERATION            0xA004
#define AL_ILLEGAL_COMMAND              AL_INVALID_OPERATION
#define AL_ILLEGAL_ENUM                 AL_INVALID_ENUM
#define AL_OUT_OF_MEMORY                0xA005
#define AL_VENDOR                       0xB001
#define AL_VERSION                      0xB002
#define AL_RENDERER                     0xB003
#define AL_EXTENSIONS                   0xB004
#define AL_DOPPLER_FACTOR               0xC000
#define AL_DOPPLER_VELOCITY             0xC001
#define AL_SPEED_OF_SOUND               0xC003
#define AL_DISTANCE_MODEL               0xD000
#define AL_INVERSE_DISTANCE             0xD001
#define AL_INVERSE_DISTANCE_CLAMPED     0xD002
#define AL_LINEAR_DISTANCE              0xD003
#define AL_LINEAR_DISTANCE_CLAMPED      0xD004
#define AL_EXPONENT_DISTANCE            0xD005
#define AL_EXPONENT_DISTANCE_CLAMPED    0xD006

typedef void(AL_APIENTRY *PFN_alEnable)(ALenum capability);
typedef void(AL_APIENTRY *PFN_alDisable)(ALenum capability);
typedef ALboolean(AL_APIENTRY *PFN_alIsEnabled)(ALenum capability);
typedef const ALchar *(AL_APIENTRY *PFN_alGetString)(ALenum param);
typedef void(AL_APIENTRY *PFN_alGetBooleanv)(ALenum param, ALboolean *values);
typedef void(AL_APIENTRY *PFN_alGetIntegerv)(ALenum param, ALint *values);
typedef void(AL_APIENTRY *PFN_alGetFloatv)(ALenum param, ALfloat *values);
typedef void(AL_APIENTRY *PFN_alGetDoublev)(ALenum param, ALdouble *values);
typedef ALboolean(AL_APIENTRY *PFN_alGetBoolean)(ALenum param);
typedef ALint(AL_APIENTRY *PFN_alGetInteger)(ALenum param);
typedef ALfloat(AL_APIENTRY *PFN_alGetFloat)(ALenum param);
typedef ALdouble(AL_APIENTRY *PFN_alGetDouble)(ALenum param);
typedef ALenum(AL_APIENTRY *PFN_alGetError)(void);
typedef ALboolean(AL_APIENTRY *PFN_alIsExtensionPresent)(const ALchar *extname);
typedef void*(AL_APIENTRY *PFN_alGetProcAddress)(const ALchar *fname);
typedef ALenum(AL_APIENTRY *PFN_alGetEnumValue)(const ALchar *ename);
typedef void(AL_APIENTRY *PFN_alListenerf)(ALenum param, ALfloat value);
typedef void(AL_APIENTRY *PFN_alListener3f)(ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
typedef void(AL_APIENTRY *PFN_alListenerfv)(ALenum param, const ALfloat *values);
typedef void(AL_APIENTRY *PFN_alListeneri)(ALenum param, ALint value);
typedef void(AL_APIENTRY *PFN_alListener3i)(ALenum param, ALint value1, ALint value2, ALint value3);
typedef void(AL_APIENTRY *PFN_alListeneriv)(ALenum param, const ALint *values);
typedef void(AL_APIENTRY *PFN_alGetListenerf)(ALenum param, ALfloat *value);
typedef void(AL_APIENTRY *PFN_alGetListener3f)(ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
typedef void(AL_APIENTRY *PFN_alGetListenerfv)(ALenum param, ALfloat *values);
typedef void(AL_APIENTRY *PFN_alGetListeneri)(ALenum param, ALint *value);
typedef void(AL_APIENTRY *PFN_alGetListener3i)(ALenum param, ALint *value1, ALint *value2, ALint *value3);
typedef void(AL_APIENTRY *PFN_alGetListeneriv)(ALenum param, ALint *values);
typedef void(AL_APIENTRY *PFN_alGenSources)(ALsizei n, ALuint *sources);
typedef void(AL_APIENTRY *PFN_alDeleteSources)(ALsizei n, const ALuint *sources);
typedef ALboolean(AL_APIENTRY *PFN_alIsSource)(ALuint source);
typedef void(AL_APIENTRY *PFN_alSourcef)(ALuint source, ALenum param, ALfloat value);
typedef void(AL_APIENTRY *PFN_alSource3f)(ALuint source, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
typedef void(AL_APIENTRY *PFN_alSourcefv)(ALuint source, ALenum param, const ALfloat *values);
typedef void(AL_APIENTRY *PFN_alSourcei)(ALuint source, ALenum param, ALint value);
typedef void(AL_APIENTRY *PFN_alSource3i)(ALuint source, ALenum param, ALint value1, ALint value2, ALint value3);
typedef void(AL_APIENTRY *PFN_alSourceiv)(ALuint source, ALenum param, const ALint *values);
typedef void(AL_APIENTRY *PFN_alGetSourcef)(ALuint source, ALenum param, ALfloat *value);
typedef void(AL_APIENTRY *PFN_alGetSource3f)(ALuint source, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
typedef void(AL_APIENTRY *PFN_alGetSourcefv)(ALuint source, ALenum param, ALfloat *values);
typedef void(AL_APIENTRY *PFN_alGetSourcei)(ALuint source,  ALenum param, ALint *value);
typedef void(AL_APIENTRY *PFN_alGetSource3i)(ALuint source, ALenum param, ALint *value1, ALint *value2, ALint *value3);
typedef void(AL_APIENTRY *PFN_alGetSourceiv)(ALuint source,  ALenum param, ALint *values);
typedef void(AL_APIENTRY *PFN_alSourcePlayv)(ALsizei n, const ALuint *sources);
typedef void(AL_APIENTRY *PFN_alSourceStopv)(ALsizei n, const ALuint *sources);
typedef void(AL_APIENTRY *PFN_alSourceRewindv)(ALsizei n, const ALuint *sources);
typedef void(AL_APIENTRY *PFN_alSourcePausev)(ALsizei n, const ALuint *sources);
typedef void(AL_APIENTRY *PFN_alSourcePlay)(ALuint source);
typedef void(AL_APIENTRY *PFN_alSourceStop)(ALuint source);
typedef void(AL_APIENTRY *PFN_alSourceRewind)(ALuint source);
typedef void(AL_APIENTRY *PFN_alSourcePause)(ALuint source);
typedef void(AL_APIENTRY *PFN_alSourceQueueBuffers)(ALuint source, ALsizei nb, const ALuint *buffers);
typedef void(AL_APIENTRY *PFN_alSourceUnqueueBuffers)(ALuint source, ALsizei nb, ALuint *buffers);
typedef void(AL_APIENTRY *PFN_alGenBuffers)(ALsizei n, ALuint *buffers);
typedef void(AL_APIENTRY *PFN_alDeleteBuffers)(ALsizei n, const ALuint *buffers);
typedef ALboolean(AL_APIENTRY *PFN_alIsBuffer)(ALuint buffer);
typedef void(AL_APIENTRY *PFN_alBufferData)(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq);
typedef void(AL_APIENTRY *PFN_alBufferf)(ALuint buffer, ALenum param, ALfloat value);
typedef void(AL_APIENTRY *PFN_alBuffer3f)(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3);
typedef void(AL_APIENTRY *PFN_alBufferfv)(ALuint buffer, ALenum param, const ALfloat *values);
typedef void(AL_APIENTRY *PFN_alBufferi)(ALuint buffer, ALenum param, ALint value);
typedef void(AL_APIENTRY *PFN_alBuffer3i)(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3);
typedef void(AL_APIENTRY *PFN_alBufferiv)(ALuint buffer, ALenum param, const ALint *values);
typedef void(AL_APIENTRY *PFN_alGetBufferf)(ALuint buffer, ALenum param, ALfloat *value);
typedef void(AL_APIENTRY *PFN_alGetBuffer3f)(ALuint buffer, ALenum param, ALfloat *value1, ALfloat *value2, ALfloat *value3);
typedef void(AL_APIENTRY *PFN_alGetBufferfv)(ALuint buffer, ALenum param, ALfloat *values);
typedef void(AL_APIENTRY *PFN_alGetBufferi)(ALuint buffer, ALenum param, ALint *value);
typedef void(AL_APIENTRY *PFN_alGetBuffer3i)(ALuint buffer, ALenum param, ALint *value1, ALint *value2, ALint *value3);
typedef void(AL_APIENTRY *PFN_alGetBufferiv)(ALuint buffer, ALenum param, ALint *values);

SALAD_EXTERN PFN_alEnable SALAD_alEnable;
SALAD_EXTERN PFN_alDisable SALAD_alDisable;
SALAD_EXTERN PFN_alIsEnabled SALAD_alIsEnabled;
SALAD_EXTERN PFN_alGetString SALAD_alGetString;
SALAD_EXTERN PFN_alGetBooleanv SALAD_alGetBooleanv;
SALAD_EXTERN PFN_alGetIntegerv SALAD_alGetIntegerv;
SALAD_EXTERN PFN_alGetFloatv SALAD_alGetFloatv;
SALAD_EXTERN PFN_alGetDoublev SALAD_alGetDoublev;
SALAD_EXTERN PFN_alGetBoolean SALAD_alGetBoolean;
SALAD_EXTERN PFN_alGetInteger SALAD_alGetInteger;
SALAD_EXTERN PFN_alGetFloat SALAD_alGetFloat;
SALAD_EXTERN PFN_alGetDouble SALAD_alGetDouble;
SALAD_EXTERN PFN_alGetError SALAD_alGetError;
SALAD_EXTERN PFN_alIsExtensionPresent SALAD_alIsExtensionPresent;
SALAD_EXTERN PFN_alGetProcAddress SALAD_alGetProcAddress;
SALAD_EXTERN PFN_alGetEnumValue SALAD_alGetEnumValue;
SALAD_EXTERN PFN_alListenerf SALAD_alListenerf;
SALAD_EXTERN PFN_alListener3f SALAD_alListener3f;
SALAD_EXTERN PFN_alListenerfv SALAD_alListenerfv;
SALAD_EXTERN PFN_alListeneri SALAD_alListeneri;
SALAD_EXTERN PFN_alListener3i SALAD_alListener3i;
SALAD_EXTERN PFN_alListeneriv SALAD_alListeneriv;
SALAD_EXTERN PFN_alGetListenerf SALAD_alGetListenerf;
SALAD_EXTERN PFN_alGetListener3f SALAD_alGetListener3f;
SALAD_EXTERN PFN_alGetListenerfv SALAD_alGetListenerfv;
SALAD_EXTERN PFN_alGetListeneri SALAD_alGetListeneri;
SALAD_EXTERN PFN_alGetListener3i SALAD_alGetListener3i;
SALAD_EXTERN PFN_alGetListeneriv SALAD_alGetListeneriv;
SALAD_EXTERN PFN_alGenSources SALAD_alGenSources;
SALAD_EXTERN PFN_alDeleteSources SALAD_alDeleteSources;
SALAD_EXTERN PFN_alIsSource SALAD_alIsSource;
SALAD_EXTERN PFN_alSourcef SALAD_alSourcef;
SALAD_EXTERN PFN_alSource3f SALAD_alSource3f;
SALAD_EXTERN PFN_alSourcefv SALAD_alSourcefv;
SALAD_EXTERN PFN_alSourcei SALAD_alSourcei;
SALAD_EXTERN PFN_alSource3i SALAD_alSource3i;
SALAD_EXTERN PFN_alSourceiv SALAD_alSourceiv;
SALAD_EXTERN PFN_alGetSourcef SALAD_alGetSourcef;
SALAD_EXTERN PFN_alGetSource3f SALAD_alGetSource3f;
SALAD_EXTERN PFN_alGetSourcefv SALAD_alGetSourcefv;
SALAD_EXTERN PFN_alGetSourcei SALAD_alGetSourcei;
SALAD_EXTERN PFN_alGetSource3i SALAD_alGetSource3i;
SALAD_EXTERN PFN_alGetSourceiv SALAD_alGetSourceiv;
SALAD_EXTERN PFN_alSourcePlayv SALAD_alSourcePlayv;
SALAD_EXTERN PFN_alSourceStopv SALAD_alSourceStopv;
SALAD_EXTERN PFN_alSourceRewindv SALAD_alSourceRewindv;
SALAD_EXTERN PFN_alSourcePausev SALAD_alSourcePausev;
SALAD_EXTERN PFN_alSourcePlay SALAD_alSourcePlay;
SALAD_EXTERN PFN_alSourceStop SALAD_alSourceStop;
SALAD_EXTERN PFN_alSourceRewind SALAD_alSourceRewind;
SALAD_EXTERN PFN_alSourcePause SALAD_alSourcePause;
SALAD_EXTERN PFN_alSourceQueueBuffers SALAD_alSourceQueueBuffers;
SALAD_EXTERN PFN_alSourceUnqueueBuffers SALAD_alSourceUnqueueBuffers;
SALAD_EXTERN PFN_alGenBuffers SALAD_alGenBuffers;
SALAD_EXTERN PFN_alDeleteBuffers SALAD_alDeleteBuffers;
SALAD_EXTERN PFN_alIsBuffer SALAD_alIsBuffer;
SALAD_EXTERN PFN_alBufferData SALAD_alBufferData;
SALAD_EXTERN PFN_alBufferf SALAD_alBufferf;
SALAD_EXTERN PFN_alBuffer3f SALAD_alBuffer3f;
SALAD_EXTERN PFN_alBufferfv SALAD_alBufferfv;
SALAD_EXTERN PFN_alBufferi SALAD_alBufferi;
SALAD_EXTERN PFN_alBuffer3i SALAD_alBuffer3i;
SALAD_EXTERN PFN_alBufferiv SALAD_alBufferiv;
SALAD_EXTERN PFN_alGetBufferf SALAD_alGetBufferf;
SALAD_EXTERN PFN_alGetBuffer3f SALAD_alGetBuffer3f;
SALAD_EXTERN PFN_alGetBufferfv SALAD_alGetBufferfv;
SALAD_EXTERN PFN_alGetBufferi SALAD_alGetBufferi;
SALAD_EXTERN PFN_alGetBuffer3i SALAD_alGetBuffer3i;
SALAD_EXTERN PFN_alGetBufferiv SALAD_alGetBufferiv;

#define alEnable SALAD_alEnable
#define alDisable SALAD_alDisable
#define alIsEnabled SALAD_alIsEnabled
#define alGetString SALAD_alGetString
#define alGetBooleanv SALAD_alGetBooleanv
#define alGetIntegerv SALAD_alGetIntegerv
#define alGetFloatv SALAD_alGetFloatv
#define alGetDoublev SALAD_alGetDoublev
#define alGetBoolean SALAD_alGetBoolean
#define alGetInteger SALAD_alGetInteger
#define alGetFloat SALAD_alGetFloat
#define alGetDouble SALAD_alGetDouble
#define alGetError SALAD_alGetError
#define alIsExtensionPresent SALAD_alIsExtensionPresent
#define alGetProcAddress SALAD_alGetProcAddress
#define alGetEnumValue SALAD_alGetEnumValue
#define alListenerf SALAD_alListenerf
#define alListener3f SALAD_alListener3f
#define alListenerfv SALAD_alListenerfv
#define alListeneri SALAD_alListeneri
#define alListener3i SALAD_alListener3i
#define alListeneriv SALAD_alListeneriv
#define alGetListenerf SALAD_alGetListenerf
#define alGetListener3f SALAD_alGetListener3f
#define alGetListenerfv SALAD_alGetListenerfv
#define alGetListeneri SALAD_alGetListeneri
#define alGetListener3i SALAD_alGetListener3i
#define alGetListeneriv SALAD_alGetListeneriv
#define alGenSources SALAD_alGenSources
#define alDeleteSources SALAD_alDeleteSources
#define alIsSource SALAD_alIsSource
#define alSourcef SALAD_alSourcef
#define alSource3f SALAD_alSource3f
#define alSourcefv SALAD_alSourcefv
#define alSourcei SALAD_alSourcei
#define alSource3i SALAD_alSource3i
#define alSourceiv SALAD_alSourceiv
#define alGetSourcef SALAD_alGetSourcef
#define alGetSource3f SALAD_alGetSource3f
#define alGetSourcefv SALAD_alGetSourcefv
#define alGetSourcei SALAD_alGetSourcei
#define alGetSource3i SALAD_alGetSource3i
#define alGetSourceiv SALAD_alGetSourceiv
#define alSourcePlayv SALAD_alSourcePlayv
#define alSourceStopv SALAD_alSourceStopv
#define alSourceRewindv SALAD_alSourceRewindv
#define alSourcePausev SALAD_alSourcePausev
#define alSourcePlay SALAD_alSourcePlay
#define alSourceStop SALAD_alSourceStop
#define alSourceRewind SALAD_alSourceRewind
#define alSourcePause SALAD_alSourcePause
#define alSourceQueueBuffers SALAD_alSourceQueueBuffers
#define alSourceUnqueueBuffers SALAD_alSourceUnqueueBuffers
#define alGenBuffers SALAD_alGenBuffers
#define alDeleteBuffers SALAD_alDeleteBuffers
#define alIsBuffer SALAD_alIsBuffer
#define alBufferData SALAD_alBufferData
#define alBufferf SALAD_alBufferf
#define alBuffer3f SALAD_alBuffer3f
#define alBufferfv SALAD_alBufferfv
#define alBufferi SALAD_alBufferi
#define alBuffer3i SALAD_alBuffer3i
#define alBufferiv SALAD_alBufferiv
#define alGetBufferf SALAD_alGetBufferf
#define alGetBuffer3f SALAD_alGetBuffer3f
#define alGetBufferfv SALAD_alGetBufferfv
#define alGetBufferi SALAD_alGetBufferi
#define alGetBuffer3i SALAD_alGetBuffer3i
#define alGetBufferiv SALAD_alGetBufferiv

#endif
