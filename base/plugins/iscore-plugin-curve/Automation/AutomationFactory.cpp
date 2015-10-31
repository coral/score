#include "AutomationFactory.hpp"
#include "AutomationModel.hpp"
#include "AutomationView.hpp"
#include "AutomationPresenter.hpp"


Process* AutomationFactory::makeModel(
        const TimeValue& duration,
        const Id<Process>& id,
        QObject* parent)
{
    return new AutomationModel{duration, id, parent};
}

LayerView* AutomationFactory::makeLayerView(
        const LayerModel&,
        QGraphicsItem* parent)
{
    return new AutomationView{parent};
}


LayerPresenter* AutomationFactory::makeLayerPresenter(
        const LayerModel& viewModel,
        LayerView* view,
        QObject* parent)
{
    return new AutomationPresenter {
        safe_cast<const AutomationLayerModel&>(viewModel),
                safe_cast<AutomationView*>(view),
                parent};
}


QByteArray AutomationFactory::makeStaticLayerConstructionData() const
{
    return {};
}
