#ifndef SESSIONTYPES_H
#define SESSIONTYPES_H

#include <QMap>
#include <QVector>

#include "Utils/typedefs.h"
#include "Utils/MetaData/MetaData.h"

namespace Session
{
	using Timecode = uint64_t;
	using Id = Timecode;

	struct Entry
	{
		Session::Id sessionId;
		Timecode timecode;
		MetaData track;

		bool operator==(const Entry& other) const;
	};

	using EntryList = QVector<Entry>;
	using EntryListMap = QMap<Session::Id, EntryList>;

	Timecode dayBegin(Session::Id id);
	Timecode dayEnd(Session::Id id);
	Timecode now();
}

Q_DECLARE_METATYPE(Session::Id)

#endif // SESSIONTYPES_H
