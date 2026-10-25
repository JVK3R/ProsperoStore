// ps5-homebrew-ui - Background music: a shuffled playlist of OGG Vorbis songs.
// Copyright (C) 2026 BlackBearReloaded
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "audio/mixer.hpp"
#include "audio/stream_ring.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct stb_vorbis;

namespace hui::audio
{

// One Vorbis or MP3 file decoded from memory (MP3 is resampled to the mixer's rate). By default it
// loops forever (LOOPSTART / LOOPLENGTH comments pick the loop, else the whole file); a playlist
// song plays once instead and decode() reports the end by returning short.
class MusicTrack
{
  public:
    ~MusicTrack();

    // Takes the whole file; returns an error, or "" on success. Vorbis must be 48 kHz; an MP3
    // (anything that is not an Ogg file) may be at any rate the format allows.
    std::string open(std::string data);
    void set_looping(bool looping)
    {
        looping_ = looping;
    }
    // Decodes stereo frames into out (mono is duplicated). Returns fewer than
    // frames only at the end of a non-looping song or on a broken stream.
    int decode(float *out, int frames);

    unsigned loop_start() const
    {
        return loop_start_;
    }
    unsigned loop_end() const
    {
        return loop_end_;
    }

  private:
    struct Mp3;
    std::string open_mp3();
    int decode_mp3(float *out, int frames);

    std::string data_;
    stb_vorbis *vorbis_ = nullptr;
    Mp3 *mp3_ = nullptr;
    std::vector<float> mp3_frames_; // decoded stereo frames, read ahead of mp3_position_
    std::size_t mp3_have_ = 0;      // frames in mp3_frames_
    double mp3_position_ = 0.0;     // next output frame's place in mp3_frames_, in source frames
    int channels_ = 0;
    bool looping_ = true;
    unsigned position_ = 0; // sample frames decoded so far
    unsigned loop_start_ = 0;
    unsigned loop_end_ = 0; // exclusive
    std::vector<float> scratch_;
};

// Plays every song in the music folder one after another, with a short pause
// between them, then starts over. The order is shuffled once per app launch.
// Decoding runs on the caller's thread (pump, once per frame) about 0.7 s
// ahead of the audio thread; completion stings duck the music by 6 dB.
class MusicPlayer
{
  public:
    MusicPlayer();

    // Lists the songs in directory (*.ogg, in a shuffled order from seed) and
    // attaches the music stream to the mixer (call before the audio thread
    // starts). Returns how many songs were found. With only set, that one song (a name without
    // extension, e.g. "background.mp3") is the whole playlist, and nothing else in directory
    // is played.
    int init(Mixer &mixer, const std::string &directory, std::uint64_t seed,
             const std::string &only = {});

    // Lowers the music under a sting for a moment.
    void duck();
    void pump(float dt);

    // The song playing now (file name without .ogg), or "".
    const std::string &current() const
    {
        return current_;
    }
    int songs() const
    {
        return static_cast<int>(playlist_.size());
    }
    // The shuffled play order (file names without .ogg).
    const std::vector<std::string> &playlist() const
    {
        return playlist_;
    }

  private:
    // Opens the next playable song; false when none of them can be played.
    bool next_song();
    void apply_gain(float seconds);

    Mixer *mixer_ = nullptr;
    std::string directory_;
    std::string suffix_ = ".ogg"; // what the playlist's names lack: ".ogg", or ".mp3" for one MP3
    std::vector<std::string> playlist_;
    std::size_t next_ = 0; // index in playlist_ of the song after this one
    StreamRing ring_{1u << 16};
    std::unique_ptr<MusicTrack> track_;
    std::string current_;
    int gap_frames_ = 0; // silence still to write before the next song
    int failures_ = 0;   // songs in a row that could not be opened
    float duck_ = 0.0f;  // seconds of ducking left
    std::vector<float> buffer_;
};

} // namespace hui::audio
