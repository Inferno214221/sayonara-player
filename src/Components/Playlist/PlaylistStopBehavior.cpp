#include "PlaylistStopBehavior.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"

using Playlist::StopBehavior;

struct StopBehavior::Private
{
	int idx_before_stop;
	Id	id_before_stop;
};

Playlist::StopBehavior::StopBehavior()
{
	m = Pimpl::make<Private>();
}

Playlist::StopBehavior::~StopBehavior() {}

int Playlist::StopBehavior::restore_track_before_stop()
{
	const MetaDataList& v_md = metadata();
	auto it = Util::find(v_md, [=](const MetaData& md){
		return (md.id == m->id_before_stop);
	});

	if(it == v_md.end()){
		set_track_idx_before_stop(-1);
		return -1;
	}

	else
	{
		m->idx_before_stop = std::distance(v_md.begin(), it);
	}

	return m->idx_before_stop;
}

int Playlist::StopBehavior::track_idx_before_stop() const
{
	return m->idx_before_stop;
}

void Playlist::StopBehavior::set_track_idx_before_stop(int idx)
{
	bool valid = between(idx, metadata().count());
	if(valid)
	{
		m->idx_before_stop = idx;
		m->id_before_stop = metadata().at(idx).id;
	}

	else {
		m->idx_before_stop = -1;
		m->id_before_stop = -1;
	}

	Settings::instance()->set<Set::PL_LastTrackBeforeStop>(m->idx_before_stop);
}
