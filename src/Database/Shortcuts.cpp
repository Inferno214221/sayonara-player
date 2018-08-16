#include "Shortcuts.h"
#include "Query.h"
#include "Utils/RawShortcutMap.h"

using DB::Shortcuts;
using DB::Query;

DB::Shortcuts::Shortcuts(const QString& connection_name, DbId db_id) :
	Module(connection_name, db_id)
{}

DB::Shortcuts::~Shortcuts() {}

RawShortcutMap DB::Shortcuts::getAllShortcuts()
{
	RawShortcutMap rsm;

	QString query = "SELECT identifier, shortcut from Shortcuts;";
	Query q(this);
	q.prepare(query);

	if(!q.exec())
	{
		q.show_error("Cannot fetch all shortcuts");
		return rsm;
	}

	while(q.next())
	{
		QString identifier = q.value(0).toString();
		QString shortcut = q.value(1).toString();

		QStringList shortcuts = rsm[identifier];
		shortcuts << shortcut;

		rsm[identifier] = shortcuts;
	}

	return rsm;
}

bool DB::Shortcuts::setShortcuts(const QString& identifier, const QStringList& shortcuts)
{
	clearShortcuts(identifier);
	db().transaction();

	for(const QString& shortcut : shortcuts)
	{
		QString query = "INSERT INTO Shortcuts (identifier, shortcut) VALUES (:identifier, :shortcut);";
		Query q(this);
		q.prepare(query);
		q.bindValue(":identifier", identifier);
		q.bindValue(":shortcut", shortcut);
		if(!q.exec())
		{
			q.show_error("Cannot insert shortcut " + identifier);
		}
	}

	return db().commit();
}

bool DB::Shortcuts::clearShortcuts(const QString& identifier)
{
	QString query = "DELETE FROM Shortcuts WHERE identifier=:identifier;";
	Query q(this);
	q.prepare(query);
	q.bindValue(":identifier", identifier);

	if(!q.exec())
	{
		q.show_error("Cannot clear shortcuts");
		return false;
	}

	return true;
}
