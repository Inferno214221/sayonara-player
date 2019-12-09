#include "SessionUtils.h"
#include "Utils/Utils.h"

#include <QDateTime>

bool Session::Entry::operator==(const Session::Entry& other) const
{
	return
			(sessionId == other.sessionId) &&
			(timecode == other.timecode) &&
			(md.title() == other.md.title());
}

Session::Timecode Session::day_begin(Session::Id id)
{
	QDateTime dt = Util::int_to_date(id);
	dt.setTime(QTime(0, 0, 0));

	return Util::date_to_int(dt);
}

Session::Timecode Session::day_end(Session::Id id)
{
	QDateTime dt = Util::int_to_date(id);
	dt.setTime(QTime(23, 59, 59));

	return Util::date_to_int(dt);
}

Session::Timecode Session::now()
{
	QDateTime dt = QDateTime::currentDateTime().toUTC();

	return Util::date_to_int(dt);
}
