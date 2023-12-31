#pragma once
#include <Gfx/SharedOutputSettings.hpp>

#include <QSize>
#include <QString>

#include <Media/Libav.hpp>
extern "C" {
#include <libavutil/pixfmt.h>
}

namespace Gfx
{
struct LibavOutputSettings : SharedOutputSettings
{
  AVPixelFormat hardwareAcceleration{AV_PIX_FMT_NONE};
  QString audio_encoder_short, audio_encoder_long;
  QString video_encoder_short, video_encoder_long;
  QString video_input_pixfmt;
  QString video_converted_pixfmt;
  QString muxer, muxer_long;
  std::map<QString, QString> options;
  int threads{};
};
}