#ifndef DB_SESSION_H
#define DB_SESSION_H

#include "Database/Module.h"
#include <QStringList>

class MetaData;
namespace DB
{
	class Session :
			private DB::Module
	{
		public:
			Session(const QString& connection_name, DbId db_id);
			~Session();

			QStringList get_sessions();
			bool add_track(const QString& session_id, uint64_t current_date_time, const MetaData& md);
			void clear();
	};
}

#endif // DB_SESSION_H
