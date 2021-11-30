#include <torchaudio/csrc/ffmpeg/sink.h>

namespace torchaudio {
namespace ffmpeg {

// 0: some kind of success
// <0: Some error happened
int Sink::process_frame(AVFrame* pFrame) {
  int ret = filter.add_frame(pFrame);
  while (ret >= 0) {
    ret = filter.get_frame(frame);
    //  AVERROR(EAGAIN) means that new input data is required to return new
    //  output.
    if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
      return 0;
    if (ret >= 0)
      buffer.push_frame(frame);
    av_frame_unref(frame);
  }
  return ret;
}
} // namespace ffmpeg
} // namespace torchaudio
