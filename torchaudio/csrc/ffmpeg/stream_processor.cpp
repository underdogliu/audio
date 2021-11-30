#include <torchaudio/csrc/ffmpeg/stream_processor.h>
<<<<<<< HEAD
=======
#include "libavutil/frame.h"
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43

namespace torchaudio {
namespace ffmpeg {

using KeyType = StreamProcessor::KeyType;

<<<<<<< HEAD
Sink::Sink(AVMediaType media_type) : buffer(media_type) {}
=======
Sink::Sink(
    AVRational input_time_base,
    AVCodecParameters* codecpar,
    const std::string& filter_description,
    double output_time_base)
    : filter(input_time_base, codecpar, filter_description),
      buffer(codecpar->codec_type),
      time_base(output_time_base) {}
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43

StreamProcessor::StreamProcessor(AVCodecParameters* codecpar)
    : media_type(codecpar->codec_type), decoder(codecpar) {}

////////////////////////////////////////////////////////////////////////////////
// Configurations
////////////////////////////////////////////////////////////////////////////////
<<<<<<< HEAD
KeyType StreamProcessor::init_stream(AVMediaType media_type) {
=======
KeyType StreamProcessor::add_stream(
    AVRational input_time_base,
    AVCodecParameters* codecpar,
    const std::string& filter_description,
    double output_rate) {
  switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
    case AVMEDIA_TYPE_VIDEO:
      break;
    default:
      throw std::runtime_error("Only Audio and Video are supported");
  }
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
  KeyType key = current_key++;
  sinks.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(key),
<<<<<<< HEAD
      std::forward_as_tuple(media_type));
  return key;
}

namespace {

template <typename... Args>
std::string string_format(const std::string& format, Args... args) {
  char buffer[512];
  std::snprintf(buffer, sizeof(buffer), format.c_str(), args...);
  return std::string(buffer);
}

std::string join(
    const std::vector<std::string>& components,
    const std::string& delim) {
  std::ostringstream s;
  for (int i = 0; i < components.size(); ++i) {
    if (i)
      s << delim;
    s << components[i];
  }
  return s.str();
}

std::string get_afilter_desc(AVSampleFormat sample_fmt, double sample_rate) {
  std::vector<std::string> components;
  if (sample_rate > 0) {
    // TODO: test float sample rate
    components.emplace_back(
        string_format("aresample=%d", static_cast<int>(sample_rate)));
  }
  components.emplace_back(string_format(
      "aformat=sample_fmts=%s", av_get_sample_fmt_name(sample_fmt)));
  return join(components, ",");
}

FilterGraph get_basic_audio_filter(
    AVRational time_base,
    AVSampleFormat in_sample_fmt,
    int in_sample_rate,
    uint64_t in_channel_layout,
    AVSampleFormat out_sample_fmt,
    double out_sample_rate) {
  auto f = get_afilter_desc(out_sample_fmt, out_sample_rate);
  FilterGraph g;
  g.add_audio_src(time_base, in_sample_rate, in_sample_fmt, in_channel_layout);
  g.add_audio_sink();
  g.add_process(f);
  g.create_filter();
  return g;
}
} // namespace

KeyType StreamProcessor::add_audio_stream(
    AVCodecParameters* codecpar,
    AVRational time_base,
    AVSampleFormat out_fmt,
    int sample_rate) {
  if (media_type != AVMEDIA_TYPE_AUDIO)
    throw std::runtime_error("Stream is not audio.");
  KeyType key = init_stream(AVMEDIA_TYPE_AUDIO);
  Sink& s = sinks.at(key);
  AVSampleFormat in_fmt = static_cast<AVSampleFormat>(codecpar->format);
  int out_sample_rate = sample_rate > 0 ? sample_rate : codecpar->sample_rate;
  s.filter = get_basic_audio_filter(
      time_base,
      in_fmt,
      codecpar->sample_rate,
      codecpar->channel_layout,
      out_fmt == AV_SAMPLE_FMT_NONE ? in_fmt : out_fmt,
      out_sample_rate);
  s.time_base = 1 / static_cast<double>(out_sample_rate);
  decoder_time_base = av_q2d(time_base);
  return key;
}

namespace {
std::string get_vfilter_desc(
    int width,
    int height,
    double frame_rate,
    AVPixelFormat pix_fmt) {
  // TODO:
  // - Add `flags` for different scale algorithm
  //   https://ffmpeg.org/ffmpeg-filters.html#scale
  // - Consider `framerate` as well
  //   https://ffmpeg.org/ffmpeg-filters.html#framerate

  // - scale
  //   https://ffmpeg.org/ffmpeg-filters.html#scale-1
  //   https://ffmpeg.org/ffmpeg-scaler.html#toc-Scaler-Options
  // - framerate
  //   https://ffmpeg.org/ffmpeg-filters.html#framerate

  // TODO:
  // - format
  //   https://ffmpeg.org/ffmpeg-filters.html#toc-format-1
  // - fps
  //   https://ffmpeg.org/ffmpeg-filters.html#fps-1
  std::vector<std::string> components;
  if (frame_rate > 0)
    components.emplace_back(string_format("fps=%lf", frame_rate));
  if (width > 0 && height > 0) {
    components.emplace_back(
        string_format("scale=width=%d:height=%d", width, height));
  } else if (width > 0) {
    components.emplace_back(string_format("scale=width=%d", width));
  } else if (height > 0) {
    components.emplace_back(string_format("scale=height=%d", height));
  }
  components.emplace_back(string_format("format=pix_fmts=%d", pix_fmt));
  return join(components, ",");
};

FilterGraph get_basic_video_filter(
    AVRational time_base,
    AVPixelFormat in_pix_fmt,
    int in_width,
    int in_height,
    AVRational in_sample_aspect_ratio,
    int out_width,
    int out_height,
    double out_frame_rate,
    AVPixelFormat out_pix_fmt) {
  auto f = get_vfilter_desc(out_width, out_height, out_frame_rate, out_pix_fmt);
  FilterGraph g;
  g.add_video_src(
      in_width, in_height, in_pix_fmt, time_base, in_sample_aspect_ratio);
  g.add_video_sink();
  g.add_process(f);
  g.create_filter();
  return g;
}
} // namespace

KeyType StreamProcessor::add_video_stream(
    AVCodecParameters* codecpar,
    AVRational time_base,
    AVPixelFormat format,
    int width,
    int height,
    double frame_rate) {
  if (media_type != AVMEDIA_TYPE_VIDEO)
    throw std::runtime_error("Stream is not video.");
  KeyType key = init_stream(AVMEDIA_TYPE_VIDEO);

  AVPixelFormat in_fmt = static_cast<AVPixelFormat>(codecpar->format);
  AVPixelFormat out_fmt = (format == AV_PIX_FMT_NONE) ? in_fmt : format;

  Sink& s = sinks.at(key);
  s.filter = get_basic_video_filter(
      time_base,
      in_fmt,
      codecpar->width,
      codecpar->height,
      codecpar->sample_aspect_ratio,
      width,
      height,
      frame_rate,
      out_fmt);
  s.time_base = (frame_rate > 0) ? 1 / frame_rate : av_q2d(time_base);
  decoder_time_base = av_q2d(time_base);
=======
      std::forward_as_tuple(
          input_time_base,
          codecpar,
          filter_description,
          (output_rate > 0) ? 1 / output_rate : av_q2d(input_time_base)));
  decoder_time_base = av_q2d(input_time_base);
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
  return key;
}

void StreamProcessor::remove_stream(KeyType key) {
  sinks.erase(key);
}

<<<<<<< HEAD
=======
////////////////////////////////////////////////////////////////////////////////
// The streaming process
////////////////////////////////////////////////////////////////////////////////
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
namespace {
void debug_print_frame(AVFrame* pFrame, double time_rate) {
  if (pFrame->sample_rate)
    std::cerr << " ---- format: "
              << av_get_sample_fmt_name(
                     static_cast<AVSampleFormat>(pFrame->format))
              << ", num_frames: " << pFrame->nb_samples
              << ", num_channels: " << pFrame->channels
              << ", num_samples: " << pFrame->nb_samples * pFrame->channels
              << ", sample_rate: " << pFrame->sample_rate
              << ", pts: " << pFrame->pts << ", pts/sample_rate: "
              << pFrame->pts / (double)pFrame->sample_rate
              << ", time: " << pFrame->pts * time_rate << std::endl;
  else
    std::cerr << " -------- format: "
              << av_get_pix_fmt_name(static_cast<AVPixelFormat>(pFrame->format))
              << ", width: " << pFrame->width << ", height: " << pFrame->height
              << ", pts: " << pFrame->pts
              << ", time: " << pFrame->pts * time_rate << std::endl;
}
} // namespace

<<<<<<< HEAD
////////////////////////////////////////////////////////////////////////////////
// The streaming process
////////////////////////////////////////////////////////////////////////////////
int StreamProcessor::process_packet(AVPacket* packet) {
  int ret = decoder.process_packet(packet);
  if (ret < 0)
    return ret;

  while (ret >= 0) {
    ret = decoder.get_frame(pFrame1);
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      break;
    AutoFrameUnref frame1{pFrame1};
    if (ret < 0) {
      return ret;
    }

    std::cerr << "src     ";
    debug_print_frame(frame1, decoder_time_base);

    // TODO
    // What should we do if one or more sink operations fail?
    for (auto& ite : sinks) {
      Sink& s = ite.second;
      ret = s.filter.add_frame(frame1);
      while (ret >= 0) {
        ret = s.filter.get_frame(pFrame2);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
          break;
        AutoFrameUnref frame2{pFrame2};
        if (ret >= 0) {
          std::cerr << "sink[" << ite.first << "] ";
          debug_print_frame(frame2, s.time_base);
          if (media_type == AVMEDIA_TYPE_AUDIO)
            s.buffer.push_audio_frame(frame2);
          else if (media_type == AVMEDIA_TYPE_VIDEO)
            s.buffer.push_video_frame(frame2);
        }
      }
    }
  }
  return 0;
=======
// 0: some kind of success
// <0: Some error happened
int StreamProcessor::process_packet(AVPacket* packet) {
  int ret = decoder.process_packet(packet);
  while (ret >= 0) {
    ret = decoder.get_frame(pFrame1);
    //  AVERROR(EAGAIN) means that new input data is required to return new
    //  output.
    if (ret == AVERROR(EAGAIN))
      return 0;
    if (ret == AVERROR_EOF)
      return send_frame(NULL);
    if (ret < 0)
      return ret;
    send_frame(pFrame1);
    av_frame_unref(pFrame1);
  }
  return ret;
}

// 0: some kind of success
// <0: Some error happened
int StreamProcessor::send_frame(AVFrame* pFrame) {
  int ret = 0;
  for (auto& ite : sinks) {
    int ret2 = ite.second.process_frame(pFrame);
    if (ret2 < 0)
      ret = ret2;
  }
  return ret;
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
}

////////////////////////////////////////////////////////////////////////////////
// Retrieval
////////////////////////////////////////////////////////////////////////////////
torch::Tensor StreamProcessor::get_chunk(KeyType key) {
  return sinks.at(key).buffer.pop_all();
}

} // namespace ffmpeg
} // namespace torchaudio
