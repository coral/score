#include <QDebug>
#include <algorithm>

#include "ClearState.hpp"
#include <Scenario/Document/State/ItemModel/MessageItemModel.hpp>
#include <Scenario/Document/State/ItemModel/MessageItemModelAlgorithms.hpp>
#include <Scenario/Document/State/StateModel.hpp>
#include <iscore/serialization/DataStreamVisitor.hpp>
#include <iscore/serialization/VisitorCommon.hpp>
#include <iscore/model/path/Path.hpp>
#include <iscore/model/path/PathSerialization.hpp>

namespace Scenario
{
namespace Command
{

ClearState::ClearState(const StateModel& state) : m_path{state}
{
  m_oldState = Process::getUserMessages(state.messages().rootNode());
}

void ClearState::undo(const iscore::DocumentContext& ctx) const
{
  auto& state = m_path.find(ctx);

  Process::MessageNode n = state.messages().rootNode();
  updateTreeWithMessageList(n, m_oldState);

  state.messages() = std::move(n);
}

void ClearState::redo(const iscore::DocumentContext& ctx) const
{
  auto& state = m_path.find(ctx);

  Process::MessageNode n = state.messages().rootNode();
  removeAllUserMessages(n);
  state.messages() = std::move(n);
}

void ClearState::serializeImpl(DataStreamInput& s) const
{
  s << m_path << m_oldState;
}

void ClearState::deserializeImpl(DataStreamOutput& s)
{
  s >> m_path >> m_oldState;
}
}
}
