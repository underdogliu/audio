#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
#include <torchaudio/csrc/ffmpeg/streamer.h>
#include <sstream>
#include <stdexcept>

namespace torchaudio {
namespace ffmpeg {

//////////////////////////////////////////////////////////////////////////////
// Helper methods
//////////////////////////////////////////////////////////////////////////////
void Streamer::validate_open_stream() const {
  if (!pFormatContext)
    throw std::runtime_error("Stream is not open.");
}

void Streamer::validate_src_stream_index(int i) const {
  validate_open_stream();
  if (i < 0 || i >= static_cast<int>(pFormatContext->nb_streams))
    throw std::out_of_range("Source stream index out of range");
}

void Streamer::validate_output_stream_index(int i) const {
  if (i < 0 || i >= static_cast<int>(stream_indices.size()))
    throw std::out_of_range("Output stream index out of range");
}

void Streamer::validate_src_stream_type(int i, AVMediaType type) {
  validate_src_stream_index(i);
  if (pFormatContext->streams[i]->codecpar->codec_type != type) {
    std::ostringstream oss;
    oss << "Stream " << i << " is not " << av_get_media_type_string(type)
        << " stream.";
    throw std::runtime_error(oss.str());
  }
}

//////////////////////////////////////////////////////////////////////////////
// Initialization / resource allocations
//////////////////////////////////////////////////////////////////////////////
Streamer::Streamer(const std::string& src) : pFormatContext(src) {
  processors =
      std::vector<std::unique_ptr<StreamProcessor>>(pFormatContext->nb_streams);
  for (int i = 0; i < pFormatContext->nb_streams; ++i) {
    switch (pFormatContext->streams[i]->codecpar->codec_type) {
      case AVMEDIA_TYPE_AUDIO:
      case AVMEDIA_TYPE_VIDEO:
        break;
      default:
        pFormatContext->streams[i]->discard = AVDISCARD_ALL;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Query methods
////////////////////////////////////////////////////////////////////////////////
int Streamer::num_src_streams() const {
  return pFormatContext->nb_streams;
}

SrcStreamInfo Streamer::get_src_stream_info(int i) const {
  validate_src_stream_index(i);
  AVStream* stream = pFormatContext->streams[i];
  AVCodecParameters* codecpar = stream->codecpar;

  SrcStreamInfo ret;
  ret.media_type = codecpar->codec_type;
  ret.bit_rate = codecpar->bit_rate;
  const AVCodecDescriptor* desc = avcodec_descriptor_get(codecpar->codec_id);
  if (desc) {
    ret.codec_name = desc->name;
    ret.codec_long_name = desc->long_name;
  }
  switch (codecpar->codec_type) {
    case AVMEDIA_TYPE_AUDIO:
      ret.fmt_name =
          av_get_sample_fmt_name(static_cast<AVSampleFormat>(codecpar->format));
      ret.sample_rate = static_cast<double>(codecpar->sample_rate);
      ret.num_channels = codecpar->channels;
      break;
    case AVMEDIA_TYPE_VIDEO:
      ret.fmt_name =
          av_get_pix_fmt_name(static_cast<AVPixelFormat>(codecpar->format));
      ret.width = codecpar->width;
      ret.height = codecpar->height;
      ret.frame_rate = av_q2d(stream->r_frame_rate);
      break;
    default:;
  }
  return ret;
}

int Streamer::num_out_streams() const {
  return stream_indices.size();
}

OutputStreamInfo Streamer::get_out_stream_info(int i) const {
  validate_output_stream_index(i);
  OutputStreamInfo ret;
  ret.source_index = stream_indices[i].first;
  // TODO:
  // Add the output configuration.
  return ret;
}

int Streamer::find_best_audio_stream() const {
  return av_find_best_stream(
      pFormatContext, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
}

int Streamer::find_best_video_stream() const {
  return av_find_best_stream(
      pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////
// Configure methods
////////////////////////////////////////////////////////////////////////////////
void Streamer::add_audio_stream(int i, int sample_rate, AVSampleFormat fmt) {
  validate_src_stream_type(i, AVMEDIA_TYPE_AUDIO);
  AVStream* stream = pFormatContext->streams[i];
  stream->discard = AVDISCARD_DEFAULT;
  if (!processors[i])
    processors[i] = std::make_unique<StreamProcessor>(stream->codecpar);
  int key = processors[i]->add_audio_stream(
      stream->codecpar, stream->time_base, fmt, sample_rate);
  stream_indices.push_back(std::make_pair<>(i, key));
}

void Streamer::add_video_stream(
    int i,
    int width,
    int height,
    double frame_rate,
    AVPixelFormat fmt) {
  validate_src_stream_type(i, AVMEDIA_TYPE_VIDEO);
  AVStream* stream = pFormatContext->streams[i];
  stream->discard = AVDISCARD_DEFAULT;
  if (!processors[i])
    processors[i] = std::make_unique<StreamProcessor>(stream->codecpar);
  int key = processors[i]->add_video_stream(
      stream->codecpar, stream->time_base, fmt, width, height, frame_rate);
  stream_indices.push_back(std::make_pair<>(i, key));
}

void Streamer::remove_stream(int i) {
  auto it = stream_indices.begin() + i;
  int iP = it->first;
  processors[iP]->remove_stream(it->second);
  stream_indices.erase(it);

  // Check if the processor is still refered and if not, disable the processor
  bool still_used = false;
  for (auto& p : stream_indices) {
    still_used |= (iP == p.first);
    if (still_used)
      break;
  }
  if (!still_used)
    processors[iP].reset(NULL);
}

////////////////////////////////////////////////////////////////////////////////
// Stream methods
////////////////////////////////////////////////////////////////////////////////
int Streamer::process_packet() {
  int ret = av_read_frame(pFormatContext, pPacket);
  if (ret < 0)
    return 1;
  AutoPacketUnref packet{pPacket};
  int i = pPacket->stream_index;
  if (!processors[i])
    return 0;
  return processors[i]->process_packet(packet);
}

int Streamer::process_all_packets() {
  int ret = 0;
  do {
    ret = process_packet();
  } while (!ret);
  return ret;
}

std::vector<torch::Tensor> Streamer::get_chunks() {
  std::vector<torch::Tensor> ret;
  for (auto& i : stream_indices) {
    ret.push_back(processors[i.first]->get_chunk(i.second));
  }
  return ret;
}

} // namespace ffmpeg
} // namespace torchaudio
