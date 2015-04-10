#include "ResizeDeckVertically.hpp"

#include "Document/Constraint/Box/Deck/DeckModel.hpp"

using namespace iscore;
using namespace Scenario::Command;

ResizeDeckVertically::ResizeDeckVertically(ObjectPath&& deckPath,
                                           double newSize) :
    SerializableCommand {"ScenarioControl",
                         className(),
                         description()},
    m_path {deckPath},
    m_newSize {newSize}
{
    auto deck = m_path.find<DeckModel>();
    m_originalSize = deck->height();
}

void ResizeDeckVertically::undo()
{
    auto deck = m_path.find<DeckModel>();
    deck->setHeight(m_originalSize);
}

void ResizeDeckVertically::redo()
{
    auto deck = m_path.find<DeckModel>();
    deck->setHeight(m_newSize);
}

bool ResizeDeckVertically::mergeWith(const Command* other)
{
    if(other->uid() == this->uid())
    {
        auto other_c = static_cast<const ResizeDeckVertically*>(other);
        m_newSize = other_c->m_newSize;
        return true;
    }
    return false;
}

void ResizeDeckVertically::serializeImpl(QDataStream& s) const
{
    s << m_path << m_originalSize << m_newSize;
}

// Would be better in a ctor ?
void ResizeDeckVertically::deserializeImpl(QDataStream& s)
{
    s >> m_path >> m_originalSize >> m_newSize;
}
