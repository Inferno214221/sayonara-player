#ifndef SHORTCUTS_H
#define SHORTCUTS_H

#include "Database/Module.h"

class RawShortcutMap;

namespace DB
{
	class Shortcuts :
		private Module
	{
	public:
		Shortcuts(const QString& connection_name, DbId db_id);
		~Shortcuts();

		RawShortcutMap getAllShortcuts();
		bool setShortcuts(const QString& identifier, const QStringList& shortcuts);
		bool clearShortcuts(const QString& identifier);
	};
}

#endif // SHORTCUTS_H
