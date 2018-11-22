#include "Session.h"

#include "Components/PlayManager/PlayManager.h"

#include "Database/Connector.h"
#include "Database/Session.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"


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

QMap<QDateTime, MetaDataList> Session::get_history(QDateTime beginning)
{
	DB::Connector* db = DB::Connector::instance();
	DB::Session* session_connector = db->session_connector();
	DB::LibraryDatabase* track_connector = db->library_db(-1, db->db_id());

	QMap<QDateTime, MetaDataList> ret;

	uint64_t start = Util::date_to_int(beginning);
	if(!beginning.isValid()){
		start = 0;
	}

	PairList<uint64_t, TrackID> history = session_connector->get_sessions(start);

	QList<TrackID> track_ids;
	for(auto it=history.begin(); it != history.end(); it++)
	{
		track_ids << it->second;
	}

	MetaDataList v_md;
	track_connector->getTracksbyIds(track_ids, v_md);

	for(auto it=v_md.begin(); it != v_md.end(); it++)
	{
		sp_log(Log::Debug, "Session") << "History Filepath: " << it->filepath();
	}


	return ret;
}
