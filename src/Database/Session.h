#ifndef DB_SESSION_H
#define DB_SESSION_H

#include "Database/Module.h"
#include <QStringList>
#include <QDateTime>

class MetaData;
class MetaDataList;

namespace DB
{
	class Session :
			private DB::Module
	{

		public:
			Session(const QString& connection_name, DbId db_id);
			~Session();

			QMap<uint64_t, QList<TrackID>> get_sessions(uint64_t beginning);

			bool add_track(const QString& session_id, uint64_t current_date_time, const MetaData& md);
			void clear();
	};
}

#endif // DB_SESSION_H
