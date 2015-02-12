#pragma once

#include <core/presenter/command/SerializableCommand.hpp>
#include <QMap>
#include <QString>
class BlacklistCommand : public iscore::SerializableCommand
{
		// QUndoCommand interface
	public:
		BlacklistCommand(QString name, bool value);

		virtual void undo();
		virtual void redo();
		virtual bool mergeWith(const QUndoCommand* other);

	protected:
		virtual void serializeImpl(QDataStream&) const override { }
		virtual void deserializeImpl(QDataStream&) override { }

		QMap<QString, bool> m_blacklistedState;
};
