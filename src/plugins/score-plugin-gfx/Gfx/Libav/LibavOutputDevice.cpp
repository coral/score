#include "LibavOutputDevice.hpp"

#if SCORE_HAS_LIBAV
#include <State/MessageListSerialization.hpp>
#include <State/Widgets/AddressFragmentLineEdit.hpp>

#include <Gfx/GfxApplicationPlugin.hpp>
#include <Gfx/GfxParameter.hpp>
#include <Gfx/Libav/LibavEncoder.hpp>
#include <Gfx/Libav/LibavEncoderNode.hpp>
#include <Gfx/Libav/LibavIntrospection.hpp>

#include <score/serialization/MimeVisitor.hpp>

#include <ossia/network/generic/generic_node.hpp>

#include <ossia-qt/name_utils.hpp>

#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QMenu>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QSpinBox>

#include <wobjectimpl.h>

SCORE_SERALIZE_DATASTREAM_DEFINE(Gfx::LibavOutputSettings);
namespace Gfx
{
// ffmpeg -y  -hwaccel cuda -hwaccel_output_format cuda -f video4linux2 -input_format mjpeg -framerate 30 -i  /dev/video0
// -fflags nobuffer
// -c:v hevc_nvenc
// -preset:v llhq
// -rc constqp
// -zerolatency 1
// -delay 0
// -forced-idr 1
// -g 1
// -cbr 1
// -qp 10
// -f matroska  -

// ffmpeg -y
//   -hwaccel cuda
//   -hwaccel_output_format cuda
//   -f video4linux2
//   -input_format mjpeg
//   -framerate 30
//   -i  /dev/video0
//   -fflags nobuffer
//   -c:v hevc_nvenc
//   -preset:v llhq
//   -rc constqp
//   -zerolatency 1
//   -delay 0
//   -forced-idr 1
//   -g 1
//   -cbr 1
//   -qp 10
//   -f matroska  -
// |
// ffplay
//   -probesize 32
//   -analyzeduration 0
//   -fflags nobuffer
//   -flags low_delay
//   -framedrop
//   -vf setpts=0
//   -sync ext -

class libav_output_protocol : public Gfx::gfx_protocol_base
{
public:
  LibavEncoder encoder;
  explicit libav_output_protocol(const LibavOutputSettings& set, GfxExecutionAction& ctx)
      : gfx_protocol_base{ctx}
      , encoder{set}
  {
  }
  ~libav_output_protocol() { }

  void start_execution() override { encoder.start(); }
  void stop_execution() override { encoder.stop(); }
};

class libav_output_device : public ossia::net::device_base
{
  ossia::net::generic_node root;

public:
  libav_output_device(
      const LibavOutputSettings& set, LibavEncoder& enc,
      std::unique_ptr<gfx_protocol_base> proto, std::string name)
      : ossia::net::device_base{std::move(proto)}
      , root{name, *this}
  {
    auto& p = *static_cast<gfx_protocol_base*>(m_protocol.get());
    auto node = new LibavEncoderNode{set, enc, 0};
    root.add_child(std::make_unique<gfx_node_base>(*this, p, node, "Video"));
  }

  const ossia::net::generic_node& get_root_node() const override { return root; }
  ossia::net::generic_node& get_root_node() override { return root; }
};
class LibavOutputDevice final : public GfxOutputDevice
{
  W_OBJECT(LibavOutputDevice)
public:
  using GfxOutputDevice::GfxOutputDevice;
  ~LibavOutputDevice();

private:
  bool reconnect() override;
  ossia::net::device_base* getDevice() const override { return m_dev.get(); }

  mutable std::unique_ptr<libav_output_device> m_dev;
};

class LibavOutputSettingsWidget final : public Gfx::SharedOutputSettingsWidget
{
public:
  LibavOutputSettingsWidget(QWidget* parent = nullptr);

  Device::DeviceSettings getSettings() const override;
  void setSettings(const Device::DeviceSettings& settings) override;
  void loadPreset(const LibavOutputSettings& settings);

private:
  void on_presetChange(int index);
  void on_muxerChange(int index);
  void on_acodecChange(int index);
  void on_vcodecChange(int index);

  QComboBox* m_presets{};
  QComboBox* m_muxer{};
  QComboBox* m_vencoder{};
  QComboBox* m_aencoder{};
  QComboBox* m_pixfmt{};
  QComboBox* m_smpfmt{};
  QPlainTextEdit* m_options{};
  Device::DeviceSettings m_settings;
};

LibavOutputDevice::~LibavOutputDevice() { }

static const std::map<QString, LibavOutputSettings> libav_preset_list{
    {"UDP MJPEG streaming",
     LibavOutputSettings{
         {
             .path = "udp://192.168.1.80:8081",
             .width = 1280,
             .height = 720,
             .rate = 30,
         },
         .threads = 0,
         .video_encoder_short = "mjpeg",
         .video_encoder_long = "",
         .video_input_pixfmt = "rgba",
         .video_converted_pixfmt = "yuv420p",
         .muxer = "mjpeg",
         .options = {{"fflags", "+nobuffer+genpts"}, {"flags", "+low_delay"}}}},

    {"MKV H.264 recording",
     LibavOutputSettings{
         {
             .path = "<PROJECT_PATH>/renders/main.mkv",
             .width = 1280,
             .height = 720,
             .rate = 30,
         },
         .threads = 0,
         .video_encoder_short = "libx265",
         .video_encoder_long = "libx265",
         .video_input_pixfmt = "rgba",
         .video_converted_pixfmt = "yuv420p",
         .muxer = "matroska"}}

    // av_dict_set(&opt, "movflags", "frag_keyframe+empty_moov+default_base_moof", 0);
};

bool LibavOutputDevice::reconnect()
{
  disconnect();

  try
  {
    auto set = this->settings().deviceSpecificSettings.value<LibavOutputSettings>();
    // set = libav_preset_list.hevc_udp;
    auto plug = m_ctx.findPlugin<DocumentPlugin>();
    if(plug)
    {
      auto m_protocol = new libav_output_protocol{set, plug->exec};
      m_dev = std::make_unique<libav_output_device>(
          set, m_protocol->encoder, std::unique_ptr<libav_output_protocol>(m_protocol),
          this->settings().name.toStdString());
    }
  }
  catch(std::exception& e)
  {
    qDebug() << "Could not connect: " << e.what();
  }
  catch(...)
  {
    // TODO save the reason of the non-connection.
  }

  return connected();
}

QString LibavOutputProtocolFactory::prettyName() const noexcept
{
  return QObject::tr("Libav Output");
}

QString LibavOutputProtocolFactory::category() const noexcept
{
  return StandardCategories::util;
}

Device::DeviceInterface* LibavOutputProtocolFactory::makeDevice(
    const Device::DeviceSettings& settings, const Explorer::DeviceDocumentPlugin& plugin,
    const score::DocumentContext& ctx)
{
  return new LibavOutputDevice(settings, ctx);
}

const Device::DeviceSettings&
LibavOutputProtocolFactory::defaultSettings() const noexcept
{
  static const Device::DeviceSettings settings = [&]() {
    Device::DeviceSettings s;
    s.protocol = concreteKey();
    s.name = "Libav Output";
    LibavOutputSettings specif;
    s.deviceSpecificSettings = QVariant::fromValue(specif);
    return s;
  }();
  return settings;
}

Device::AddressDialog* LibavOutputProtocolFactory::makeAddAddressDialog(
    const Device::DeviceInterface& dev, const score::DocumentContext& ctx,
    QWidget* parent)
{
  return nullptr;
}

Device::AddressDialog* LibavOutputProtocolFactory::makeEditAddressDialog(
    const Device::AddressSettings& set, const Device::DeviceInterface& dev,
    const score::DocumentContext& ctx, QWidget* parent)
{
  return nullptr;
}

Device::ProtocolSettingsWidget* LibavOutputProtocolFactory::makeSettingsWidget()
{
  return new LibavOutputSettingsWidget;
}

QVariant LibavOutputProtocolFactory::makeProtocolSpecificSettings(
    const VisitorVariant& visitor) const
{
  return makeProtocolSpecificSettings_T<LibavOutputSettings>(visitor);
}

void LibavOutputProtocolFactory::serializeProtocolSpecificSettings(
    const QVariant& data, const VisitorVariant& visitor) const
{
  serializeProtocolSpecificSettings_T<LibavOutputSettings>(data, visitor);
}

bool LibavOutputProtocolFactory::checkCompatibility(
    const Device::DeviceSettings& a, const Device::DeviceSettings& b) const noexcept
{
  return a.name != b.name;
}

LibavOutputSettingsWidget::LibavOutputSettingsWidget(QWidget* parent)
    : SharedOutputSettingsWidget{parent}
{
  m_deviceNameEdit->setText("Libav Out");
  ((QLabel*)m_layout->labelForField(m_shmPath))->setText("Path");

  const auto& info = LibavIntrospection::instance();

  m_presets = new QComboBox{};
  {
    for(auto& [name, preset] : libav_preset_list)
      m_presets->addItem(name);
  }

  m_muxer = new QComboBox{};
  for(auto& mux : info.muxers)
  {
    QString name = mux.format->name;
    if(mux.format->long_name && strlen(mux.format->long_name) > 0)
    {
      name += " (";
      name += mux.format->long_name;
      name += ")";
    }
    m_muxer->addItem(name, QVariant::fromValue((void*)mux.format));
  }

  m_vencoder = new QComboBox{};
  m_aencoder = new QComboBox{};
  m_pixfmt = new QComboBox{};
  m_smpfmt = new QComboBox{};
  m_options = new QPlainTextEdit{};
  m_options->setFont(score::Skin::instance().MonoFontSmall);

  connect(
      m_presets, &QComboBox::currentIndexChanged, this,
      &LibavOutputSettingsWidget::on_presetChange);
  connect(
      m_muxer, &QComboBox::currentIndexChanged, this,
      &LibavOutputSettingsWidget::on_muxerChange);
  connect(
      m_vencoder, &QComboBox::currentIndexChanged, this,
      &LibavOutputSettingsWidget::on_vcodecChange);
  connect(
      m_aencoder, &QComboBox::currentIndexChanged, this,
      &LibavOutputSettingsWidget::on_acodecChange);

  m_layout->addRow(tr("Preset"), m_presets);
  m_layout->addRow(tr("Muxer"), m_muxer);
  m_layout->addRow(tr("Video encoder"), m_vencoder);
  m_layout->addRow(tr("Pixel format"), m_pixfmt);
  m_layout->addRow(tr("Audio encoder"), m_aencoder);
  m_layout->addRow(tr("Sample format"), m_smpfmt);
  m_layout->addRow(tr("Additional options"), m_options);

  m_width->setValue(1280);
  m_height->setValue(720);
  m_rate->setValue(30);
}

void LibavOutputSettingsWidget::on_presetChange(int preset_index)
{
  auto name = m_presets->itemText(preset_index);
  auto it = libav_preset_list.find(name);
  if(it != libav_preset_list.end())
  {
    loadPreset(it->second);
  }
}

void LibavOutputSettingsWidget::on_muxerChange(int muxer_index)
{
  m_vencoder->clear();
  m_aencoder->clear();
  m_pixfmt->clear();
  m_smpfmt->clear();
  const auto& info = LibavIntrospection::instance();
  if(muxer_index < 0 || muxer_index >= info.muxers.size())
    return;

  auto& mux = info.muxers[muxer_index];
  const AVCodec* default_vcodec = nullptr;
  const AVCodec* default_acodec = nullptr;
  for(auto& vcodec : mux.vcodecs)
  {
    QString name = vcodec->codec->name;
    if(vcodec->codec->long_name && strlen(vcodec->codec->long_name) > 0)
    {
      name += " (";
      name += vcodec->codec->long_name;
      name += ")";
    }
    if(!default_vcodec && mux.format->video_codec == vcodec->codec->id)
      default_vcodec = vcodec->codec;
    m_vencoder->addItem(name, QVariant::fromValue((void*)vcodec->codec));
  }

  for(auto& acodec : mux.acodecs)
  {
    QString name = acodec->codec->name;
    if(acodec->codec->long_name && strlen(acodec->codec->long_name) > 0)
    {
      name += " (";
      name += acodec->codec->long_name;
      name += ")";
    }
    if(!default_acodec && mux.format->audio_codec == acodec->codec->id)
      default_acodec = acodec->codec;
    m_aencoder->addItem(name, QVariant::fromValue((void*)acodec->codec));
  }

  if(default_vcodec)
  {
    int vc_index = m_vencoder->findData(QVariant::fromValue((void*)default_vcodec));
    if(vc_index != -1)
      m_vencoder->setCurrentIndex(vc_index);
  }
  if(default_acodec)
  {
    int ac_index = m_aencoder->findData(QVariant::fromValue((void*)default_acodec));
    if(ac_index != -1)
      m_aencoder->setCurrentIndex(ac_index);
  }
}

void LibavOutputSettingsWidget::on_vcodecChange(int codec_index)
{
  m_pixfmt->clear();
  const auto& info = LibavIntrospection::instance();
  int muxer_index = m_muxer->currentIndex();
  if(muxer_index < 0 || muxer_index >= info.muxers.size())
    return;

  auto& mux = info.muxers[muxer_index];
  if(codec_index < 0 || codec_index >= mux.vcodecs.size())
    return;

  auto codec = mux.vcodecs[codec_index];
  if(codec->codec->pix_fmts == nullptr)
    return;

  for(auto fmt = codec->codec->pix_fmts; *fmt != AVPixelFormat(-1); ++fmt)
  {
    if(auto desc = av_pix_fmt_desc_get(*fmt))
      m_pixfmt->addItem(desc->name);
  }
}

void LibavOutputSettingsWidget::on_acodecChange(int codec_index)
{
  m_smpfmt->clear();
  const auto& info = LibavIntrospection::instance();
  int muxer_index = m_muxer->currentIndex();
  if(muxer_index < 0 || muxer_index >= info.muxers.size())
    return;

  auto& mux = info.muxers[muxer_index];
  if(codec_index < 0 || codec_index >= mux.vcodecs.size())
    return;

  auto codec = mux.acodecs[codec_index];
  if(codec->codec->sample_fmts == nullptr)
    return;

  for(auto fmt = codec->codec->sample_fmts; *fmt != AVSampleFormat(-1); ++fmt)
  {
    if(auto desc = av_get_sample_fmt_name(*fmt))
      m_smpfmt->addItem(desc);
  }
}
Device::DeviceSettings LibavOutputSettingsWidget::getSettings() const
{
  Device::DeviceSettings s = m_settings;
  s.name = m_deviceNameEdit->text();
  s.protocol = LibavOutputProtocolFactory::static_concreteKey();
  return s;
}

void LibavOutputSettingsWidget::loadPreset(const LibavOutputSettings& settings)
{
  const auto& info = LibavIntrospection::instance();
  auto muxer = info.findMuxer(settings.muxer, settings.muxer_long);
  if(!muxer)
    return;

  auto vcodec
      = info.findVideoCodec(settings.video_encoder_short, settings.video_encoder_long);
  auto acodec
      = info.findAudioCodec(settings.audio_encoder_short, settings.audio_encoder_long);

  if(!vcodec && !acodec)
    return;

  {
    int mux_index = m_muxer->findData(QVariant::fromValue((void*)muxer->format));
    if(mux_index != -1)
      m_muxer->setCurrentIndex(mux_index);
  }
  if(vcodec)
  {
    int vc_index = m_vencoder->findData(QVariant::fromValue((void*)vcodec->codec));
    if(vc_index != -1)
      m_vencoder->setCurrentIndex(vc_index);
  }
  if(acodec)
  {
    int ac_index = m_aencoder->findData(QVariant::fromValue((void*)acodec->codec));
    if(ac_index != -1)
      m_aencoder->setCurrentIndex(ac_index);
  }
}

void LibavOutputSettingsWidget::setSettings(const Device::DeviceSettings& settings)
{
  m_settings = settings;

  auto prettyName = settings.name;
  m_deviceNameEdit->setText(prettyName);
}
}

template <>
void DataStreamReader::read(const Gfx::LibavOutputSettings& n)
{
  insertDelimiter();
}

template <>
void DataStreamWriter::write(Gfx::LibavOutputSettings& n)
{
  checkDelimiter();
}

template <>
void JSONReader::read(const Gfx::LibavOutputSettings& n)
{
}

template <>
void JSONWriter::write(Gfx::LibavOutputSettings& n)
{
}

W_OBJECT_IMPL(Gfx::LibavOutputDevice)
#endif