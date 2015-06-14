#include "CopyConstraintContent.hpp"

#include "Document/Constraint/ConstraintModel.hpp"
#include "Document/Constraint/Box/BoxModel.hpp"
#include "Document/Constraint/Box/Deck/DeckModel.hpp"
#include "ProcessInterface/ProcessModel.hpp"
#include "ProcessInterface/ProcessViewModel.hpp"
#include <iscore/tools/SettableIdentifierGeneration.hpp>
#include <QJsonDocument>

using namespace iscore;
using namespace Scenario::Command;

// TODO rename in SetConstraintContent ?
CopyConstraintContent::CopyConstraintContent(QJsonObject&& sourceConstraint,
                                             ObjectPath&& targetConstraint,
                                             ExpandMode mode) :
    SerializableCommand {"ScenarioControl",
                         className(),
                         description()},
    m_source{sourceConstraint},
    m_target{targetConstraint},
    m_mode{mode}
{
    auto& trg_constraint = m_target.find<ConstraintModel>();
    ConstraintModel src_constraint{
            Deserializer<JSONObject>{m_source},
            &trg_constraint}; // Temporary parent

    // For all boxes in source, generate new id's
    auto target_boxes = trg_constraint.boxes();
    QVector<id_type<BoxModel>> target_boxes_ids;
    std::transform(target_boxes.begin(), target_boxes.end(),
                   std::back_inserter(target_boxes_ids),
                   [] (BoxModel* box) { return box->id(); });

    for(const auto& box : src_constraint.boxes())
    {
        auto newId = getStrongId(target_boxes_ids);
        m_boxIds.insert(box->id(), newId);
        target_boxes_ids.append(newId);
    }

    // Same for processes
    auto target_processes = trg_constraint.processes();
    QVector<id_type<ProcessModel>> target_processes_ids;
    std::transform(target_processes.begin(), target_processes.end(),
                   std::back_inserter(target_processes_ids),
                   [] (ProcessModel* proc) { return proc->id(); });

    for(const auto& proc : src_constraint.processes())
    {
        auto newId = getStrongId(target_processes_ids);
        m_processIds.insert(proc->id(), newId);
        target_processes_ids.append(newId);
    }
}

void CopyConstraintContent::undo()
{
    // We just have to remove what we added
    auto& trg_constraint = m_target.find<ConstraintModel>();

    for(const auto& proc_id : m_processIds)
    {
        trg_constraint.removeProcess(proc_id);
    }

    for(const auto& box_id : m_boxIds)
    {
        trg_constraint.removeBox(box_id);
    }
}


void CopyConstraintContent::redo()
{
    auto& trg_constraint = m_target.find<ConstraintModel>();
    ConstraintModel src_constraint{
            Deserializer<JSONObject>{m_source},
            &trg_constraint}; // Temporary parent

    std::map<const ProcessModel*, ProcessModel*> processPairs;

    // Clone the processes
    auto src_procs = src_constraint.processes();
    for(const auto& sourceproc : src_procs)
    {
        auto newproc = sourceproc->clone(m_processIds[sourceproc->id()], &trg_constraint);

        processPairs.insert(std::make_pair(sourceproc, newproc));
        trg_constraint.addProcess(newproc);

        // Resize the processes according to the new constraint.
        if(m_mode == ExpandMode::Scale)
        {
            newproc->setDurationAndScale(trg_constraint.defaultDuration());
        }
        else if (m_mode == ExpandMode::Grow)
        {
            newproc->setDurationAndGrow(trg_constraint.defaultDuration());
        }
    }

    // Clone the boxes
    auto src_boxes = src_constraint.boxes();
    for(const auto& sourcebox: src_boxes)
    {
        // A note about what happens here :
        // Since we want to duplicate our process view models using
        // the target constraint's cloned shared processes (they might setup some specific data),
        // we maintain a pair mapping each original process to their cloned counterpart.
        // We can then use the correct cloned process to clone the process view model.
        auto newbox = new BoxModel{
                *sourcebox,
                m_boxIds[sourcebox->id()],
                [&] (const DeckModel& source, DeckModel& target)
                {
                    for(const auto& pvm : source.processViewModels())
                    {
                        // We can safely reuse the same id since it's in a different deck.
                        auto proc = processPairs[&pvm->sharedProcessModel()];
                        // TODO harmonize the order of parameters (source first, then new id)
                        target.addProcessViewModel(proc->cloneViewModel(pvm->id(), *pvm, &target));
                    }
                },
                &trg_constraint};
        trg_constraint.addBox(newbox);
    }
}

void CopyConstraintContent::serializeImpl(QDataStream& s) const
{
    s << m_source << m_target << m_boxIds << m_processIds << (int) m_mode;
}

void CopyConstraintContent::deserializeImpl(QDataStream& s)
{
    int mode;
    s >> m_source >> m_target >> m_boxIds >> m_processIds >> mode;
    m_mode = static_cast<ExpandMode>(mode);
}
