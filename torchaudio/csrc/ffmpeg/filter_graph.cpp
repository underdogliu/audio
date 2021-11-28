#include <torchaudio/csrc/ffmpeg/filter_graph.h>
#include <iostream>
#include <stdexcept>

namespace torchaudio {
namespace ffmpeg {

////////////////////////////////////////////////////////////////////////////////
// Low-level configuration methods
////////////////////////////////////////////////////////////////////////////////
namespace {
std::string get_audio_src_args(
    AVRational time_base,
    int sample_rate,
    AVSampleFormat sample_fmt,
    uint64_t channel_layout) {
  char args[512];
  std::snprintf(
      args,
      sizeof(args),
      "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=0x%" PRIx64,
      time_base.num,
      time_base.den,
      sample_rate,
      av_get_sample_fmt_name(sample_fmt),
      channel_layout);
  return std::string(args);
}

std::string get_video_src_args(
    int width,
    int height,
    int pix_fmt,
    AVRational time_base,
    AVRational sample_aspect_ratio) {
  char args[512];
  std::snprintf(
      args,
      sizeof(args),
      "video_size=%dx%d:pix_fmt=%d:time_base=%d/%d:pixel_aspect=%d/%d",
      width,
      height,
      pix_fmt,
      time_base.num,
      time_base.den,
      sample_aspect_ratio.num,
      sample_aspect_ratio.den);
  return std::string(args);
}

} // namespace

void FilterGraph::add_audio_src(
    AVRational time_base,
    int sample_rate,
    AVSampleFormat sample_fmt,
    uint64_t channel_layout) {
  if (buffersrc_ctx)
    throw std::runtime_error("Source buffer is already allocated.");
  const AVFilter* buffersrc = avfilter_get_by_name("abuffer");
  std::string args =
      get_audio_src_args(time_base, sample_rate, sample_fmt, channel_layout);
  int ret = avfilter_graph_create_filter(
      &buffersrc_ctx, buffersrc, "in", args.c_str(), NULL, pFilterGraph);
  if (ret < 0)
    throw std::runtime_error("Failed to create input filter: \"" + args + "\"");
}

void FilterGraph::add_video_src(
    int width,
    int height,
    AVPixelFormat pix_fmt,
    AVRational time_base,
    AVRational sample_aspect_ratio) {
  if (buffersrc_ctx)
    throw std::runtime_error("Source buffer is already allocated.");
  const AVFilter* buffersrc = avfilter_get_by_name("buffer");
  std::string args = get_video_src_args(
      width, height, pix_fmt, time_base, sample_aspect_ratio);
  int ret = avfilter_graph_create_filter(
      &buffersrc_ctx, buffersrc, "in", args.c_str(), NULL, pFilterGraph);
  if (ret < 0)
    throw std::runtime_error("Failed to create input filter: \"" + args + "\"");
}

void FilterGraph::add_audio_sink() {
  if (buffersink_ctx)
    throw std::runtime_error("Sink buffer is already allocated.");
  const AVFilter* buffersink = avfilter_get_by_name("abuffersink");
  // Note
  // Originally, the code here followed the example
  // https://ffmpeg.org/doxygen/4.1/filtering_audio_8c-example.html
  // which sets option for `abuffersink`, which caused an issue where the
  // `abuffersink` parameters set for the first time survive across multiple
  // fitler generations.
  // According to the other example
  // https://ffmpeg.org/doxygen/4.1/filter_audio_8c-example.html
  // `abuffersink` should not take options, and this resolved issue.
  int ret = avfilter_graph_create_filter(
      &buffersink_ctx, buffersink, "out", NULL, NULL, pFilterGraph);
  if (ret < 0) {
    throw std::runtime_error("Failed to create output filter.");
  }
}

void FilterGraph::add_video_sink() {
  if (buffersink_ctx)
    throw std::runtime_error("Sink buffer is already allocated.");
  const AVFilter* buffersink = avfilter_get_by_name("buffersink");
  // Note:
  // In example
  // https://ffmpeg.org/doxygen/4.1/filtering_video_8c-example.html
  // It sets `pix_fmt` option. But in the case of audio, setting option
  // on `buffersink` caused overwrite of global structure (if not memory leak)
  // and led to automatic insertion of unwanted resampling.
  // Here, I assume that the same thing could happen for video buffersink,
  // so I am not setting the option on it.
  // Currently `pix_fmt` is set via `add_process` by caller side, as
  // `format=pix_fmts=FOO`.
  // If something about format conversion is wrong, we need to check if
  // this produces the expected pixel format across multiple filter generations
  // with different parameters.
  int ret = avfilter_graph_create_filter(
      &buffersink_ctx, buffersink, "out", NULL, NULL, pFilterGraph);
  if (ret < 0)
    throw std::runtime_error("Failed to create output filter.");
}

namespace {

// Encapsulating AVFilterInOut* with handy methods since
// we need to deal with multiple of them at the same time.
class InOuts {
  AVFilterInOut* p = NULL;
  // Disable copy constructor/assignment just in case.
  InOuts(const InOuts&) = delete;
  InOuts& operator=(const InOuts&) = delete;

 public:
  InOuts(const char* name, AVFilterContext* pCtx) {
    p = avfilter_inout_alloc();
    if (!p)
      throw std::runtime_error("Failed to allocate AVFilterInOut.");
    p->name = av_strdup(name);
    p->filter_ctx = pCtx;
    p->pad_idx = 0;
    p->next = NULL;
  };
  ~InOuts() {
    avfilter_inout_free(&p);
  }
  operator AVFilterInOut**() {
    return &p;
  }
};

} // namespace

void FilterGraph::add_process(std::string filter_desc) {
  // Note
  // The official example and other derived codes out there use
  // https://ffmpeg.org/doxygen/4.1/filtering_audio_8c-example.html#_a37
  // variable name `in` for "out"/buffersink, and `out` for "in"/buffersrc.
  // If you are debugging this part of the code, you might get confused.
  InOuts in{"in", buffersrc_ctx}, out{"out", buffersink_ctx};

  int ret = avfilter_graph_parse_ptr(
      pFilterGraph, filter_desc.c_str(), out, in, NULL);

  if (ret < 0)
    throw std::runtime_error("Failed to create the filter.");
}

void FilterGraph::create_filter() {
  if (avfilter_graph_config(pFilterGraph, NULL) < 0)
    throw std::runtime_error("Failed to configure the graph.");
  // char* desc = avfilter_graph_dump(pFilterGraph.get(), NULL);
  // std::cerr << "Filter created:\n" << desc << std::endl;
  // av_free(static_cast<void*>(desc));
}

////////////////////////////////////////////////////////////////////////////////
// Streaming process
//////////////////////////////////////////////////////////////////////////////
int FilterGraph::add_frame(AVFrame* pInputFrame) {
  return av_buffersrc_add_frame_flags(
      buffersrc_ctx, pInputFrame, AV_BUFFERSRC_FLAG_KEEP_REF);
}

int FilterGraph::get_frame(AVFrame* pOutputFrame) {
  return av_buffersink_get_frame(buffersink_ctx, pOutputFrame);
}

} // namespace ffmpeg
} // namespace torchaudio
