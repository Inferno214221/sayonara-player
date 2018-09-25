#include "Session.h"

#include "Components/PlayManager/PlayManager.h"

#include "Database/Connector.h"
#include "Database/Session.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
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

	QMap<uint64_t, QList<TrackID>> history = session_connector->get_sessions(start);

	MetaDataList v_md;
	MetaDataList v_md_ret;
	track_connector->getAllTracks(v_md);

	QMap<TrackID, int> track_map;

	for(int i=0; i<v_md.count(); i++)
	{
		TrackID id = v_md[i].id;
		track_map[id] = i;
	}

	for(auto it=history.begin(); it != history.end(); it++)
	{
		uint64_t date_int = it.key();
		QDateTime date_time = Util::int_to_date(date_int);

		sp_log(Log::Debug, "SESSION") << "";
		sp_log(Log::Debug, "SESSION") << date_int;
		const QList<TrackID>& history_list = it.value();
		for(const TrackID& id : history_list)
		{
			if(!track_map.contains(id)){
				continue;
			}

			int idx = track_map[id];
			const MetaData& md = v_md[idx];
			ret[date_time].push_back(md);
			sp_log(Log::Debug, "SESSION") << "   " << md.filepath();
		}
	}

	return ret;
}
