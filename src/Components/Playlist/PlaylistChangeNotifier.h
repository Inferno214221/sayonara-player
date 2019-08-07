#ifndef PLAYLISTCHANGENOTIFIER_H
#define PLAYLISTCHANGENOTIFIER_H

#include "Utils/Singleton.h"
#include <QObject>

class PlaylistChangeNotifier : public QObject
{
	Q_OBJECT
	SINGLETON(PlaylistChangeNotifier)

	signals:
		void sig_playlist_renamed(int id, const QString& old_name, const QString& new_name);
		void sig_playlist_added(int id, const QString& name);
		void sig_playlist_deleted(int id);

	public:
		void delete_playlist(int id);
		void add_playlist(int id, const QString& name);
		void rename_playlist(int id, const QString& old_name, const QString& new_name);
};

#endif // PLAYLISTCHANGENOTIFIER_H
