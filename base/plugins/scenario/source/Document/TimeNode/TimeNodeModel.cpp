#include "TimeNodeModel.hpp"

TimeNodeModel::TimeNodeModel(id_type<TimeNodeModel> id, QObject *parent):
	IdentifiedObject<TimeNodeModel>{id, "TimeNodeModel", parent}
{

}

TimeNodeModel::TimeNodeModel(id_type<TimeNodeModel> id, TimeValue date, QObject *parent):
	TimeNodeModel{id, parent}
{
	m_date = date;
    metadata.setName(QString("TimeNode.%1").arg(*this->id().val()));
    metadata.setLabel("TimeNode");
}

TimeNodeModel::~TimeNodeModel()
{

}

void TimeNodeModel::addEvent(id_type<EventModel> eventId)
{
	m_events.push_back(eventId);
}

bool TimeNodeModel::removeEvent(id_type<EventModel> eventId)
{
	if(m_events.indexOf(eventId) >= 0)
	{
		m_events.remove(m_events.indexOf(eventId));
		return true;
	}
	return false;
}

double TimeNodeModel::top() const
{
	return m_topY;
}

double TimeNodeModel::bottom() const
{
	return m_bottomY;
}

TimeValue TimeNodeModel::date() const
{
    return m_date;
}

void TimeNodeModel::setDate(TimeValue date)
{
    m_date = date;
    emit dateChanged();
}

bool TimeNodeModel::isEmpty()
{
    return (m_events.size() == 0);
}
double TimeNodeModel::y() const
{
	return m_y;
}

void TimeNodeModel::setY(double y)
{
	m_y = y;
}
QVector<id_type<EventModel> > TimeNodeModel::events() const
{
    return m_events;
}

void TimeNodeModel::setEvents(const QVector<id_type<EventModel> > &events)
{
    m_events = events;
}



