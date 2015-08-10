#include "AreaModel.hpp"
#include "AreaPresenter.hpp"
#include "AreaView.hpp"

#include <DeviceExplorer/Node/Node.hpp>
AreaModel::AreaModel(
        std::unique_ptr<spacelib::area>&& area,
        const SpaceModel& space,
        const id_type<AreaModel> & id,
        QObject *parent):
    IdentifiedObject{id, staticMetaObject.className(), parent},
    m_space{space},
    m_area{std::move(area)}
{
    /*
    for(const auto& sym : m_area->symbols())
    {
        m_parameterMap.insert(
                    sym.get_name().c_str(),
                    {sym, iscore::FullAddressSettings{}});
    }
    */
}


void AreaModel::setSpaceMapping(const GiNaC::exmap& mapping)
{
    m_spaceMap = mapping;
}

void AreaModel::setParameterMapping(const AreaModel::ParameterMap &mapping)
{
    m_parameterMap = mapping;
}

spacelib::projected_area AreaModel::projectedArea() const
{
    return spacelib::projected_area(*m_area.get(), m_spaceMap);
}

spacelib::valued_area AreaModel::valuedArea() const
{
    GiNaC::exmap mapping;
    for(const auto& elt : m_parameterMap)
    {
        if(elt.second.address.device.isEmpty()) // We use the value
        {
            mapping[elt.first] = elt.second.value.val.toDouble();
        }
        else // We fetch it from the device tree
        {
            ISCORE_TODO
        }
    }
    return spacelib::valued_area(projectedArea(), mapping);
}

QString AreaModel::toString() const
{
    std::stringstream s;
    s << static_cast<const GiNaC::ex&>(m_area->rel());
    return QString::fromStdString(s.str());
}


AreaPresenter*AreaModel::makePresenter(QGraphicsItem* parentItem, QObject* parentObject) const
{
    auto pres = new AreaPresenter{new AreaView{parentItem}, *this, parentObject};
    connect(this, &AreaModel::areaChanged,
            pres, &AreaPresenter::on_areaChanged);
    return pres;
}

