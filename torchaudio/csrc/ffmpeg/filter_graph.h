#pragma once

#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
namespace torchaudio {
namespace ffmpeg {

class FilterGraph {
  AVFilterGraphPtr pFilterGraph;
  // AVFilterContext is freed as a part of AVFilterGraph
  // so we do not manage the resource.
  AVFilterContext* buffersrc_ctx = NULL;
  AVFilterContext* buffersink_ctx = NULL;

 public:
  FilterGraph() = default;
  // Custom destructor to release AVFilterGraph*
  ~FilterGraph() = default;
  // Non-copyable
  FilterGraph(const FilterGraph&) = delete;
  FilterGraph& operator=(const FilterGraph&) = delete;
  // Movable
  FilterGraph(FilterGraph&&) = default;
  FilterGraph& operator=(FilterGraph&&) = default;

  //////////////////////////////////////////////////////////////////////////////
  // Low-level configuration methods
  //////////////////////////////////////////////////////////////////////////////
  void add_audio_src(
      AVRational time_base,
      int sample_rate,
      AVSampleFormat sample_fmt,
      uint64_t channel_layout);

  void add_video_src(
      int width,
      int height,
      AVPixelFormat pix_fmt,
      AVRational time_base,
      AVRational sample_aspect_ratio);

  void add_audio_sink();

  void add_video_sink();

  void add_process(std::string filter_desc);

  void create_filter();

  //////////////////////////////////////////////////////////////////////////////
  // Streaming process
  //////////////////////////////////////////////////////////////////////////////
  int add_frame(AVFrame* pInputFrame);
  int get_frame(AVFrame* pOutputFrame);
};

} // namespace ffmpeg
} // namespace torchaudio
