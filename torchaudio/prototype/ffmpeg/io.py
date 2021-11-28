from dataclasses import dataclass
from typing import Optional, Tuple

import torch


@dataclass
class SourceStream:
    media_type: str
    codec: str
    codec_long_name: str
    format: Optional[str]
    bit_rate: Optional[int]


@dataclass
class SourceAudioStream(SourceStream):
    sample_rate: float
    num_channels: int


@dataclass
class SourceVideoStream(SourceStream):
    width: int
    height: int
    frame_rate: float


def _parse_si(i):
    # - COMMON
    # media_type: str
    # codec name: str
    # codec long name: str
    # format name: str
    # bit_rate: int
    # - AUDIO
    # sample_rate: double
    # num_channels: int
    # - VIDEO
    # width: int64_t
    # height: int64_t
    # frame_rate: double
    if i[0] == "audio":
        return SourceAudioStream(i[0], i[1], i[2], i[3], i[4], i[5], i[6])
    if i[0] == "video":
        return SourceVideoStream(i[0], i[1], i[2], i[3], i[4], i[7], i[8], i[9])
    return SourceStream(i[0], i[1], i[2], None, None)


def info(src):
    s = Streamer(src)
    return [
        _parse_si(torch.ops.torchaudio.ffmpeg_streamer_get_src_stream_info(s._s, i))
        for i in range(s.num_src_streams)
    ]


@dataclass
class OutputStream:
    source_index: int


def _parse_oi(i):
    # - COMMON
    # source_idex: int
    return OutputStream(i[0])


def load(src: str) -> Tuple[torch.Tensor, int]:
    return torch.ops.torchaudio.ffmpeg_load(src)


class Streamer:
    def __init__(self, src: str):
        self._s = torch.ops.torchaudio.ffmpeg_streamer_init(src)

    @property
    def num_src_streams(self):
        return torch.ops.torchaudio.ffmpeg_streamer_num_src_streams(self._s)

    @property
    def num_out_streams(self):
        return torch.ops.torchaudio.ffmpeg_streamer_num_out_streams(self._s)

    def get_src_stream_info(self, i):
        return _parse_si(torch.ops.torchaudio.ffmpeg_streamer_get_src_stream_info(self._s, i))

    def get_out_stream_info(self, i):
        return _parse_oi(torch.ops.torchaudio.ffmpeg_streamer_get_out_stream_info(self._s, i))

    def find_best_audio_stream(self):
        return torch.ops.torchaudio.ffmpeg_streamer_find_best_audio_stream(self._s)

    def find_best_video_stream(self):
        return torch.ops.torchaudio.ffmpeg_streamer_find_best_video_stream(self._s)

    def add_audio_stream(
        self,
        i: int,
        sample_rate: Optional[int] = None,
        dtype: Optional[torch.dtype] = torch.float32,
    ):
        torch.ops.torchaudio.ffmpeg_streamer_add_audio_stream(
            self._s, i, sample_rate, dtype
        )

    def add_video_stream(
        self,
        i: int,
        width: Optional[int] = None,
        height: Optional[int] = None,
        frame_rate: Optional[int] = None,
        format: Optional[str] = "RGB",
    ):
        torch.ops.torchaudio.ffmpeg_streamer_add_video_stream(
            self._s, i, width, height, frame_rate, format,
        )

    def remove_stream(self, i: int):
        torch.ops.torchaudio.ffmpeg_streamer_remove_stream(self._s, i)

    def process_packet(self):
        return torch.ops.torchaudio.ffmpeg_streamer_process_packet(self._s)

    def process_all_packets(self):
        return torch.ops.torchaudio.ffmpeg_streamer_process_all_packets(self._s)

    def get_chunks(self):
        return torch.ops.torchaudio.ffmpeg_streamer_get_chunks(self._s)

    def __iter__(self):
        return self

    def __next__(self):
        res = torch.ops.torchaudio.ffmpeg_streamer_process_packet(self._s)
        if res > 0:
            raise StopIteration
