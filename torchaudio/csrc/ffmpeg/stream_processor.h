#pragma once

<<<<<<< HEAD
#include <torchaudio/csrc/ffmpeg/buffer.h>
#include <torchaudio/csrc/ffmpeg/decoder.h>
#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
#include <torchaudio/csrc/ffmpeg/filter_graph.h>
=======
#include <torch/torch.h>
#include <torchaudio/csrc/ffmpeg/decoder.h>
#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
#include <torchaudio/csrc/ffmpeg/sink.h>
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
#include <map>

namespace torchaudio {
namespace ffmpeg {

<<<<<<< HEAD
struct Sink {
  FilterGraph filter;
  Buffer buffer;
  double time_base;
  Sink(AVMediaType media_type);
};

=======
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
class StreamProcessor {
 public:
  using KeyType = int;

 private:
  AVMediaType media_type = AVMEDIA_TYPE_UNKNOWN;
  AVFramePtr pFrame1;
  AVFramePtr pFrame2;

  // Components for decoding source media
  double decoder_time_base; // for debug
  Decoder decoder;

  KeyType current_key = 0;
  std::map<KeyType, Sink> sinks;

 public:
  StreamProcessor(AVCodecParameters* codecpar);
  ~StreamProcessor() = default;
  // Non-copyable
  StreamProcessor(const StreamProcessor&) = delete;
  StreamProcessor& operator=(const StreamProcessor&) = delete;
  // Movable
  StreamProcessor(StreamProcessor&&) = default;
  StreamProcessor& operator=(StreamProcessor&&) = default;

  //////////////////////////////////////////////////////////////////////////////
  // Configurations
  //////////////////////////////////////////////////////////////////////////////
  // 1. Initialize decoder (if not initialized yet)
  // 2. Configure a new audio/video filter.
  //    If the custom parameter is provided, then perform resize, resample etc..
  //    otherwise, the filter only converts the sample type.
  // 3. Configure a buffer.
  // 4. Return filter ID.
<<<<<<< HEAD
  KeyType add_audio_stream(
      AVCodecParameters* codecpar,
      AVRational time_base,
      AVSampleFormat fmt,
      int sample_rate);
  KeyType add_video_stream(
      AVCodecParameters* codecpar,
      AVRational time_base,
      AVPixelFormat format,
      int width,
      int height,
      double frame_rate);

 private:
  KeyType init_stream(AVMediaType media_type);

 public:
  // 1. Remove the stream
  // 2. If there is no stream left, then remove decoder.
=======
  KeyType add_stream(
      AVRational input_time_base,
      AVCodecParameters* codecpar,
      const std::string& filter_description,
      double output_rate);

  // 1. Remove the stream
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
  void remove_stream(KeyType key);

  //////////////////////////////////////////////////////////////////////////////
  // The streaming process
  //////////////////////////////////////////////////////////////////////////////
  // 1. decode the input frame
  // 2. pass the decoded data to filters
  // 3. each filter store the result to the corresponding buffer
<<<<<<< HEAD
  int process_packet(AVPacket* packet);

  //////////////////////////////////////////////////////////////////////////////
  // Retrieval
  //////////////////////////////////////////////////////////////////////////////
=======
  // - Sending NULL will drain (flush) the internal
  int process_packet(AVPacket* packet);

 private:
  int send_frame(AVFrame* pFrame);

  //////////////////////////////////////////////////////////////////////////////
  // Retrieval
  //////////////////////////////////////////////////////////////////////////////
 public:
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
  // Get the chunk from the given filter result
  torch::Tensor get_chunk(KeyType key);
};

} // namespace ffmpeg
} // namespace torchaudio
