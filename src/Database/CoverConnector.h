#ifndef COVERCONNECTOR_H
#define COVERCONNECTOR_H

#include "Database/Module.h"

#include <QMap>
#include <QString>
#include <QPixmap>

namespace DB
{
	class Covers :
		private DB::Module
	{
		public:
			Covers(const QString& connection_name, DbId db_id);
			~Covers();

			bool exists(const QString& hash);
			bool get_cover(const QString& hash, QPixmap& data);
			bool set_cover(const QString& hash, const QPixmap& data);

			bool get_all_covers(QMap<QString, QPixmap>& covers);
			void clear();
	};
}

#endif // COVERCONNECTOR_H
