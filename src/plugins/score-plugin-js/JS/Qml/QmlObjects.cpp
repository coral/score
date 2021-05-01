#include "QmlObjects.hpp"

#include <wobjectimpl.h>
W_OBJECT_IMPL(JS::Inlet)
W_OBJECT_IMPL(JS::Outlet)
W_OBJECT_IMPL(JS::ValueInlet)
W_OBJECT_IMPL(JS::ValueOutlet)
W_OBJECT_IMPL(JS::ControlInlet)
W_OBJECT_IMPL(JS::MidiInlet)
W_REGISTER_ARGTYPE(QVector<int>)
W_REGISTER_ARGTYPE(QVector<double>)
W_OBJECT_IMPL(JS::MidiOutlet)
W_OBJECT_IMPL(JS::AudioInlet)
W_OBJECT_IMPL(JS::AudioOutlet)
W_OBJECT_IMPL(JS::FloatSlider)
W_OBJECT_IMPL(JS::IntSlider)
W_OBJECT_IMPL(JS::Enum)
W_OBJECT_IMPL(JS::Toggle)
W_OBJECT_IMPL(JS::LineEdit)

W_GADGET_IMPL(JS::ValueMessage)
W_GADGET_IMPL(JS::MidiMessage)

auto disregard_me = &JS::ValueMessage::staticMetaObject;
auto disregard_me_2 = &JS::MidiMessage::staticMetaObject;
namespace JS
{

ValueInlet::ValueInlet(QObject* parent)
    : Inlet{parent}
{
}

ValueInlet::~ValueInlet() { }

QVariant ValueInlet::value() const
{
  return m_value;
}

void ValueInlet::setValue(QVariant value)
{
  if (m_value == value)
    return;

  m_value = value;
}

ControlInlet::ControlInlet(QObject* parent)
    : Inlet{parent}
{
}

ControlInlet::~ControlInlet() { }

QVariant ControlInlet::value() const
{
  return m_value;
}

void ControlInlet::setValue(QVariant value)
{
  if (m_value == value)
    return;

  m_value = value;
}

ValueOutlet::ValueOutlet(QObject* parent)
    : Outlet{parent}
{
}

ValueOutlet::~ValueOutlet() { }

QVariant ValueOutlet::value() const
{
  return m_value;
}

void ValueOutlet::setValue(QVariant value)
{
  if (m_value == value)
    return;

  m_value = value;
}

void ValueOutlet::addValue(qreal timestamp, QVariant t)
{
  values.push_back({timestamp, std::move(t)});
}

AudioInlet::AudioInlet(QObject* parent)
    : Inlet{parent}
{
}

AudioInlet::~AudioInlet() { }

const QVector<QVector<double>>& AudioInlet::audio() const
{
  return m_audio;
}

void AudioInlet::setAudio(const QVector<QVector<double>>& audio)
{
  m_audio = audio;
}

AudioOutlet::AudioOutlet(QObject* parent)
    : Outlet{parent}
{
}

AudioOutlet::~AudioOutlet() { }

const QVector<QVector<double>>& AudioOutlet::audio() const
{
  return m_audio;
}

MidiInlet::MidiInlet(QObject* parent)
    : Inlet{parent}
{
}

MidiInlet::~MidiInlet() { }

MidiOutlet::MidiOutlet(QObject* parent)
    : Outlet{parent}
{
}

MidiOutlet::~MidiOutlet() { }

void MidiOutlet::clear()
{
  m_midi.clear();
}

const QVector<QVector<int>>& MidiOutlet::midi() const
{
  return m_midi;
}

Inlet::~Inlet() { }

Outlet::~Outlet() { }

FloatSlider::~FloatSlider() = default;
IntSlider::~IntSlider() = default;
Toggle::~Toggle() = default;
Enum::~Enum() = default;
LineEdit::~LineEdit() = default;
}
