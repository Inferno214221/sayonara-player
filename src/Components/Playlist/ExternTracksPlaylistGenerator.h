#ifndef EXTERNTRACKSPLAYLISTGENERATOR_H
#define EXTERNTRACKSPLAYLISTGENERATOR_H

#include "Utils/Pimpl.h"
#include "Utils/Singleton.h"

class QStringList;
class MetaDataList;

class ExternTracksPlaylistGenerator
{
	PIMPL(ExternTracksPlaylistGenerator)
	SINGLETON(ExternTracksPlaylistGenerator)

public:
	void add_paths(const QStringList& paths);
	void change_track();
	bool is_play_allowed() const;
};

#endif // EXTERNTRACKSPLAYLISTGENERATOR_H
