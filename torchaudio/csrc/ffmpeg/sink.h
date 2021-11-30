#pragma once

#include <torchaudio/csrc/ffmpeg/buffer.h>
#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
#include <torchaudio/csrc/ffmpeg/filter_graph.h>

namespace torchaudio {
namespace ffmpeg {

class Sink {
  AVFramePtr frame;

 public:
  FilterGraph filter;
  Buffer buffer;
  double time_base;
  Sink(
      AVRational input_time_base,
      AVCodecParameters* codecpar,
      const std::string& filter_description,
      double output_time_base);

  int process_frame(AVFrame* frame);
};

} // namespace ffmpeg
} // namespace torchaudio
