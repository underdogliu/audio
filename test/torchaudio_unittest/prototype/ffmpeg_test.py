import torch
from torchaudio.prototype import ffmpeg
from torchaudio_unittest.common_utils import (
    TorchaudioTestCase,
    TempDirMixin,
    get_asset_path,
    get_wav_data,
    save_wav,
    nested_params,
)
from PIL import Image


path = get_asset_path("bipbopall.mp4")


class FFmpegLoadTest(TempDirMixin, TorchaudioTestCase):
    @nested_params(
        ["int16", "uint8", "int32"],  # "float", "double", "int64"]
        [1, 2, 4, 8],
    )
    def test_load_wav(self, dtype, num_channels):
        expected = get_wav_data(
            dtype, num_channels=num_channels, normalize=False, num_frames=15, channels_first=True)
        sample_rate = 8000
        path = self.get_temp_path("test.wav")
        save_wav(path, expected, sample_rate)

        output, sr = ffmpeg.load(path)

        assert sample_rate == sr
        self.assertEqual(output.T, expected)


class FFmpegStreamerTest(TempDirMixin, TorchaudioTestCase):
    def test_info(self):
        for sinfo in ffmpeg.info(path):
            print(sinfo)

    def test_src_info(self):
        s = ffmpeg.Streamer(path)
        for i in range(s.num_src_streams):
            print(s.get_src_stream_info(i))

    def test_out_info(self):
        s = ffmpeg.Streamer(path)
<<<<<<< HEAD
        s.add_audio_stream(0, 8000)
        s.add_audio_stream(0, 16000)
        s.add_video_stream(1, 32, 32, 30)
        s.add_video_stream(1, 16, 16, 10)
=======
        s.add_basic_audio_stream(0, 8000)
        s.add_basic_audio_stream(0, 16000)
        s.add_basic_video_stream(1, 32, 32, 30)
        s.add_basic_video_stream(1, 16, 16, 10)
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43

        sinfo = [s.get_out_stream_info(i) for i in range(s.num_out_streams)]
        assert sinfo[0].source_index == 0
        assert sinfo[1].source_index == 0
        assert sinfo[2].source_index == 1
        assert sinfo[3].source_index == 1

    def test_modify_stream(self):
        s = ffmpeg.Streamer(path)
<<<<<<< HEAD
        s.add_audio_stream(0, 8000)
        s.add_audio_stream(0, 16000)
        s.add_video_stream(1, 32, 32, 30)
        s.add_video_stream(1, 16, 16, 10)
=======
        s.add_basic_audio_stream(0, 8000)
        s.add_basic_audio_stream(0, 16000)
        s.add_basic_video_stream(1, 32, 32, 30)
        s.add_basic_video_stream(1, 16, 16, 10)
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        s.remove_stream(3)
        s.remove_stream(2)
        s.remove_stream(1)
        s.remove_stream(0)
<<<<<<< HEAD
        s.add_audio_stream(0, 8000)
        s.remove_stream(0)
        s.add_audio_stream(0, 8000)
=======
        s.add_basic_audio_stream(0, 8000)
        s.remove_stream(0)
        s.add_basic_audio_stream(0, 8000)
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        # TODO: Add invalid operations

    def _save_wav(self, data, sample_rate):
        path = self.get_temp_path("test.wav")
        save_wav(path, data, sample_rate)
        return path

    def _test_wav(self, path, original, dtype):
        s = ffmpeg.Streamer(path)
<<<<<<< HEAD
        s.add_audio_stream(0, dtype=dtype)
        for _ in s:
            pass
=======
        s.add_basic_audio_stream(0, dtype=dtype)
        s.process_all_packets()
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        output = s.get_chunks()[0].T
        # print(output)
        self.assertEqual(original, output)

    @nested_params(
        ["int16", "uint8", "int32"],  # "float", "double", "int64"]
    )
    def test_wav_dtypes(self, dtype):
        original = get_wav_data(
            dtype, num_channels=1, normalize=False, num_frames=15, channels_first=True)
        path = self._save_wav(original, 8000)
        # provide the matching dtype
        self._test_wav(path, original, getattr(torch, dtype))
        # use the internal dtype ffmpeg picks
        self._test_wav(path, original, None)

    @nested_params([2, 4, 8])
    def test_wav_multichannels(self, num_channels):
        dtype = torch.int16
        original = torch.randint(low=-32768, high=32767, size=[num_channels, 256], dtype=dtype)
        path = self._save_wav(original, 8000)
        # provide the matching dtype
        self._test_wav(path, original, dtype)
        # use the internal dtype ffmpeg picks
        self._test_wav(path, original, None)

<<<<<<< HEAD
=======
    def test_custom_audio_stream(self):
        original = torch.randint(low=-32768, high=32767, size=[3, 15], dtype=torch.int16)
        path = self._save_wav(original, 8000)
        expected = torch.flip(original, dims=(1, ))

        s = ffmpeg.Streamer(path)
        s.add_custom_audio_stream(0, "areverse")
        s.process_all_packets()
        output = s.get_chunks()[0].T
        # print(output)
        print("original:", original)
        print("expected:", expected)
        print("output:", output)
        self.assertEqual(expected, output)

>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
    def _save_png(self, data):
        path = self.get_temp_path("test.png")
        img = Image.fromarray(data.numpy())
        img.save(path)
        return path

    def _test_png(self, path, original, format):
        s = ffmpeg.Streamer(path)
<<<<<<< HEAD
        s.add_video_stream(0, format=format)
=======
        s.add_basic_video_stream(0, format=format)
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        s.process_all_packets()
        output = s.get_chunks()[0]
        self.assertEqual(original, output)

    def test_png_gray(self):
        h, w = 111, 250
        original = torch.arange(h * w, dtype=torch.int64).reshape(h, w) % 256
        original = original.to(torch.uint8)
        path = self._save_png(original)
        expected = original[None, None, ...]
        self._test_png(path, expected, format=None)

    def test_png_color(self):
<<<<<<< HEAD
=======
        # TODO:
        # Add test with alpha channel (RGBA, ARGB, BGRA, ABGR)
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        c, h, w = 3, 111, 250
        original = torch.arange(c * h * w, dtype=torch.int64).reshape(c, h, w) % 256
        original = original.to(torch.uint8)
        path = self._save_png(original.permute(1, 2, 0))
        expected = original[None, ...]
        self._test_png(path, expected, format=None)

    def test_image(self):
        s = ffmpeg.Streamer(path)
        i = s.find_best_video_stream()
        sinfo = s.get_src_stream_info(i)
<<<<<<< HEAD
        s.add_video_stream(i, frame_rate=5, format="RGB")
        s.add_video_stream(
=======
        s.add_basic_video_stream(i, frame_rate=5, format="RGB")
        s.add_basic_video_stream(
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
            i, frame_rate=5, width=sinfo.width//2, height=sinfo.height//2, format="RGB")
        for _ in s:
            pass
        # for i, images in enumerate(s.get_chunks()):
        #     for j, img in enumerate(images):
        #         im = Image.fromarray(img.numpy().transpose(1, 2, 0), 'RGB')
        #         im.save(f"foo/test_{i}_{j}.png")



'''
    def test_streamer(self):
        s = ffmpeg.Streamer(path)
        i = s.find_best_audio_stream()
        print(f"Configuring audio stream {i}")
        sinfo = s.get_src_stream_info(i)
        sample_rate = sinfo.sample_rate * 2
<<<<<<< HEAD
        s.add_audio_stream(i, sample_rate=int(sample_rate))
=======
        s.add_basic_audio_stream(i, sample_rate=int(sample_rate))
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        print(s.get_src_stream_info(i))
        i = s.find_best_video_stream()
        print(f"Configuring video stream {i}")
        sinfo = s.get_src_stream_info(i)
<<<<<<< HEAD
        s.add_video_stream(
            i, width=sinfo.width * 2, height=sinfo.height * 2, frame_rate=sinfo.frame_rate * 2. + 0.1)
        s.add_video_stream(
=======
        s.add_basic_video_stream(
            i, width=sinfo.width * 2, height=sinfo.height * 2, frame_rate=sinfo.frame_rate * 2. + 0.1)
        s.add_basic_video_stream(
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
            i, width=sinfo.width // 2, height=sinfo.height // 2, frame_rate=sinfo.frame_rate / 2. - 0.1)
        print(s.get_src_stream_info(i))
        for _ in s:
            pass
        for chunk in s.get_chunks():
            print(chunk.shape)

    def test_multichannel(self):
        sample_rate = 4000
        num_channels = 3
        waveform = torch.randint(-2**8, 2**8-1, (num_channels, sample_rate), dtype=torch.int16)
        path = self.get_temp_path("file.wav")
        torchaudio.backend.sox_io_backend.save(path, waveform, sample_rate, encoding="PCM_S", bits_per_sample=16)

        s = ffmpeg.Streamer(path)
        i = s.find_best_audio_stream()
        sinfo = s.get_src_stream_info(i)
<<<<<<< HEAD
        s.add_audio_stream(i, sample_rate=int(sinfo.sample_rate))
=======
        s.add_basic_audio_stream(i, sample_rate=int(sinfo.sample_rate))
>>>>>>> 248ae94c5670b9d85882067b148ba41f95bc9b43
        for _ in s:
            pass
        for chunk in s.get_chunks():
            print(chunk.shape)

'''
