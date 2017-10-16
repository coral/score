#pragma once
#include <Device/Protocol/DeviceInterface.hpp>
#include <QString>
#include <algorithm>
#include <functional>
#include <vector>

#include <Device/Protocol/DeviceSettings.hpp>

namespace Device
{

/**
 * @brief The DeviceList class
 *
 * Once a device is added with addDevice, the DeviceList
 * has ownership.
 *
 * This is not the case for the local device, which is owned
 * by LocalTreeDocumentPlugin
 */
class SCORE_LIB_DEVICE_EXPORT DeviceList : public QObject
{
  Q_OBJECT
public:
  DeviceInterface& device(const QString& name) const;
  DeviceInterface& device(const Device::Node& name) const;

  DeviceInterface* findDevice(const QString& name) const;

  void addDevice(DeviceInterface* dev);
  void removeDevice(const QString& name);

  void apply(std::function<void(Device::DeviceInterface&)>);

  void setLogging(bool);

  void setLocalDevice(DeviceInterface*);
  void setAudioDevice(DeviceInterface* dev) { m_audioDevice = dev; }
  void setMidiDevice(DeviceInterface* dev) { m_midiDevice = dev; }
signals:
  void logInbound(const QString&) const;
  void logOutbound(const QString&) const;
  void logError(const QString&) const;

private:
  const std::vector<DeviceInterface*>& devices() const;
  std::vector<DeviceInterface*> m_devices;
  DeviceInterface* m_localDevice{}, *m_audioDevice{}, *m_midiDevice{};
  bool m_logging = false;
};
}
