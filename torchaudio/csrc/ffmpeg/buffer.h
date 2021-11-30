#pragma once
#include <torch/torch.h>
#include <torchaudio/csrc/ffmpeg/ffmpeg.h>
#include <deque>

namespace torchaudio {
namespace ffmpeg {

class Buffer {
  std::deque<torch::Tensor> chunks;
  AVMediaType media_type;

<<<<<<< HEAD
 public:
  Buffer(AVMediaType type);
  void push_audio_frame(AVFrame* pFrame);
  void push_video_frame(AVFrame* pFrame);

=======
  void push_audio_frame(AVFrame* pFrame);
  void push_video_frame(AVFrame* pFrame);

 public:
  Buffer(AVMediaType type);

  void push_frame(AVFrame* pFrame);
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
  torch::Tensor pop_all();
};

} // namespace ffmpeg
} // namespace torchaudio
