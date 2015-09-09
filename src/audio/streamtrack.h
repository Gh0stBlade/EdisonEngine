#pragma once

#include "audio/audio.h"

namespace audio
{
// Audio stream type defines stream behaviour. While background track
// loops forever until interrupted by other background track, one-shot
// and chat tracks doesn't interrupt them, playing in parallel instead.
// However, all stream types could be interrupted by next pending track
// with same type.
enum class StreamType
{
    Any,
    Background,    // BGM tracks.
    Oneshot,       // One-shot music pieces.
    Chat          // Chat tracks.
};

// Stream loading method describes the way audiotracks are loaded.
// There are either seperate track files or single CDAUDIO.WAD file.
enum class StreamMethod
{
    Any,
    Track,  // Separate tracks. Used in TR 1, 2, 4, 5.
    WAD    // WAD file.  Used in TR3.
};

// Main stream track class is used to create multi-channel soundtrack player,
// which differs from classic TR scheme, where each new soundtrack interrupted
// previous one. With flexible class handling, we now can implement multitrack
// player with automatic channel and crossfade management.
class StreamTrack
{
public:
    StreamTrack();      // Stream track constructor.
    ~StreamTrack();      // Stream track destructor.

     // Load routine prepares track for playing. Arguments are track index,
     // stream type (background, one-shot or chat) and load method, which
     // differs for TR1-2, TR3 and TR4-5.

    bool load(const char *path, const int index, const StreamType type, const StreamMethod load_method);
    bool unload();

    bool play(bool fade_in = false);     // Begins to play track.
    void pause();                        // Pauses track, preserving position.
    void end();                          // End track with fade-out.
    void stop();                         // Immediately stop track.
    bool update();                       // Update track and manage streaming.

    bool isTrack(const int track_index) const; // Checks desired track's index.
    bool isType(const StreamType track_type);   // Checks desired track's type.
    bool isPlaying() const;                    // Checks if track is playing.
    bool isActive();                     // Checks if track is still active.
    bool isDampable();                   // Checks if track is dampable.

    void setFX();                        // Set reverb FX, according to room flag.
    void unsetFX();                      // Remove any reverb FX from source.

    static bool damp_active;             // Global flag for damping BGM tracks.

private:
    bool loadTrack(const char *path);                     // Track loading.
    bool loadWad(uint8_t index, const char *filename);    // Wad loading.

    bool stream(ALuint buffer);          // General stream routine.

    FILE*           m_wadFile;   //!< General handle for opened wad file.
    SNDFILE*        m_sndFile;   //!< Sndfile file reader needs its own handle.
    SF_INFO         m_sfInfo;

    // General OpenAL fields

    ALuint          m_source;
    ALuint          m_buffers[StreamBufferCount];
    ALenum          m_format;
    ALsizei         m_rate;
    ALfloat         m_currentVolume;     //!< Stream volume, considering fades.
    ALfloat         m_dampedVolume;      //!< Additional damp volume multiplier.

    bool            m_active;            //!< If track is active or not.
    bool            m_ending;            //!< Used when track is being faded by other one.
    bool            m_dampable;          //!< Specifies if track can be damped by others.
    StreamType      m_streamType;        //!< Either BACKGROUND, ONESHOT or CHAT.
    int             m_currentTrack;      //!< Needed to prevent same track sending.
    StreamMethod    m_method;            //!< TRACK (TR1-2/4-5) or WAD (TR3).
};

void updateStreamsDamping();                   // See if there any damping tracks playing.

} // namespace audio