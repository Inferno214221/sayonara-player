#ifndef SESSIONTYPES_H
#define SESSIONTYPES_H

#include <QMap>
#include <QList>

#include "Utils/typedefs.h"
#include "Utils/MetaData/MetaData.h"

namespace Session
{
	using Timecode=uint64_t;
	using Id=Timecode;

	struct Entry
	{
		Session::Id sessionId;
		Timecode timecode;
		MetaData md;

		bool operator==(const Entry& other) const;
	};

	using EntryList=QList<Entry>;
	using EntryListMap=QMap<Session::Id, EntryList>;

	Session::Timecode day_begin(Session::Id id);
	Session::Timecode day_end(Session::Id id);
	Session::Timecode now();
}

#endif // SESSIONTYPES_H
