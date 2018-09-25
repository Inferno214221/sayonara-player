#include "Session.h"

#include "Components/PlayManager/PlayManager.h"

#include "Database/Connector.h"
#include "Database/Session.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"

#include <QDateTime>

struct Session::Private
{
	QString session_id;

	Private()
	{
		session_id = Util::random_string(32);
	}
};

Session::Session(QObject* parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	PlayManager* pm = PlayManager::instance();

	connect(pm, &PlayManager::sig_track_changed, this, &Session::track_changed);
}

Session::~Session() {}

void Session::track_changed(const MetaData& md)
{
	uint64_t cur_date = Util::current_date_to_int();

	DB::Connector* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();

	session_connector->add_track(m->session_id, cur_date, md);
}
