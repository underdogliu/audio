#include <torchaudio/csrc/ffmpeg/stream_processor.h>
#include "libavutil/frame.h"

namespace torchaudio {
namespace ffmpeg {

using KeyType = StreamProcessor::KeyType;

Sink::Sink(
    AVRational input_time_base,
    AVCodecParameters* codecpar,
    const std::string& filter_description,
    double output_time_base)
    : filter(input_time_base, codecpar, filter_description),
      buffer(codecpar->codec_type),
      time_base(output_time_base) {}

StreamProcessor::StreamProcessor(AVCodecParameters* codecpar)
    : media_type(codecpar->codec_type), decoder(codecpar) {}

////////////////////////////////////////////////////////////////////////////////
// Configurations
////////////////////////////////////////////////////////////////////////////////
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
  KeyType key = current_key++;
  sinks.emplace(
      std::piecewise_construct,
      std::forward_as_tuple(key),
      std::forward_as_tuple(
          input_time_base,
          codecpar,
          filter_description,
          (output_rate > 0) ? 1 / output_rate : av_q2d(input_time_base)));
  decoder_time_base = av_q2d(input_time_base);
  return key;
}

void StreamProcessor::remove_stream(KeyType key) {
  sinks.erase(key);
}

////////////////////////////////////////////////////////////////////////////////
// The streaming process
////////////////////////////////////////////////////////////////////////////////
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
}

////////////////////////////////////////////////////////////////////////////////
// Retrieval
////////////////////////////////////////////////////////////////////////////////
torch::Tensor StreamProcessor::get_chunk(KeyType key) {
  return sinks.at(key).buffer.pop_all();
}

} // namespace ffmpeg
} // namespace torchaudio
