/*************************************************************************/
/*  video_stream_theora.h                                                */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2017 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2017 Godot Engine contributors (cf. AUTHORS.md)    */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#pragma once

#include "io/resource_loader.h"
#include "os/file_access.h"
#include "os/semaphore.h"
#include "os/thread.h"
#include "ring_buffer.h"
#include "scene/resources/video_stream.h"

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavformat/avformat.h>
}


class VideoStreamPlaybackFFMPEG : public VideoStreamPlayback {
	OBJ_TYPE(VideoStreamPlaybackFFMPEG, VideoStreamPlayback);

	enum {
		MAX_FRAMES = 4,
	};

	//Image frames[MAX_FRAMES];
	DVector<uint8_t> frame_data;
	int frames_pending;
	int audio_frames_wrote;

	float get_time() const;

	double videobuf_time;
	int pp_inc;

	int pp_level_max;
	int pp_level;
	int videobuf_ready;

	bool playing;
	bool buffering;

	double last_update_time;
	double delay_compensation;

	AudioMixCallback mix_callback;
	void *mix_udata;
	bool paused;

	int audio_track;

protected:
	void clear();

public:
	virtual void play();
	virtual void stop();
	virtual bool is_playing() const;

	virtual void set_paused(bool p_paused);
	virtual bool is_paused(bool p_paused) const;

	virtual void set_loop(bool p_enable);
	virtual bool has_loop() const;

	virtual float get_length() const;

	virtual String get_stream_name() const;

	virtual int get_loop_count() const;

	virtual float get_pos() const;
	virtual void seek_pos(float p_time);

	void set_file(const String &p_file);

	virtual Ref<Texture> get_texture();
	virtual void update(float p_delta);

	virtual void set_mix_callback(AudioMixCallback p_callback, void *p_userdata);
	virtual int get_channels() const;
	virtual int get_mix_rate() const;

	virtual void set_audio_track(int p_idx);

	VideoStreamPlaybackFFMPEG();
	~VideoStreamPlaybackFFMPEG();

private:
	void decode_frame_from_packet(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);
	void write_frame_to_texture(void);

private:
	String				m_fileName;					// 文件名称
	AVFormatContext*	m_formatCtx;
	int					m_videoStreamIdx;
	AVCodec*			m_videoCodec;
	AVCodecParameters*	m_videoCodecpar;
	AVCodecContext*		m_videoCodecCtx;
	AVPacket			m_packet;
	AVFrame*			m_videoFrame;
	int					m_videoWidth;				// 视频宽
	int					m_videoHeight;				// 视频高
	Image::Format		m_videoTextureFormat;
	Ref<ImageTexture>	m_videoTexture;

	double				m_time;
	double				m_videoFrameTime;
};

class VideoStreamFFMPEG : public VideoStream {

	OBJ_TYPE(VideoStreamFFMPEG, VideoStream);

	String file;
	int audio_track;

public:
	Ref<VideoStreamPlayback> instance_playback() {
		Ref<VideoStreamPlaybackFFMPEG> pb = memnew(VideoStreamPlaybackFFMPEG);
		pb->set_audio_track(audio_track);
		pb->set_file(file);
		return pb;
	}

	void set_file(const String &p_file) { file = p_file; }
	void set_audio_track(int p_track) { audio_track = p_track; }

	VideoStreamFFMPEG() { audio_track = 0; }
};

class ResourceFormatLoaderVideoStreamFFMPEG : public ResourceFormatLoader {
public:
	virtual RES load(const String &p_path, const String &p_original_path = "", Error *r_error = NULL);
	virtual void get_recognized_extensions(List<String> *p_extensions) const;
	virtual bool handles_type(const String &p_type) const;
	virtual String get_resource_type(const String &p_path) const;
};
