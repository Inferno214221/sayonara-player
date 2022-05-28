#include "SessionUtils.h"
#include "Utils/Utils.h"

#include <QDateTime>

bool Session::Entry::operator==(const Session::Entry& other) const
{
	return
		(sessionId == other.sessionId) &&
		(timecode == other.timecode) &&
		(track.title() == other.track.title());
}

Session::Timecode Session::dayBegin(Session::Id id)
{
	QDateTime dt = Util::intToDate(id);
	dt.setTime(QTime(0, 0, 0));

	return Util::dateToInt(dt);
}

Session::Timecode Session::dayEnd(Session::Id id)
{
	QDateTime dt = Util::intToDate(id);
	dt.setTime(QTime(23, 59, 59));

	return Util::dateToInt(dt);
}

Session::Timecode Session::now()
{
	QDateTime dt = QDateTime::currentDateTime().toUTC();

	return Util::dateToInt(dt);
}
