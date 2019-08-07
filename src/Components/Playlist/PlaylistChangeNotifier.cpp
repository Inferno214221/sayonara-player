#include "PlaylistChangeNotifier.h"

PlaylistChangeNotifier::PlaylistChangeNotifier() : QObject()
{}

PlaylistChangeNotifier::~PlaylistChangeNotifier() {}

void PlaylistChangeNotifier::delete_playlist(int id)
{
	emit sig_playlist_deleted(id);
}

void PlaylistChangeNotifier::add_playlist(int id, const QString& name)
{
	emit sig_playlist_added(id, name);
}

void PlaylistChangeNotifier::rename_playlist(int id, const QString& old_name, const QString& new_name)
{
	emit sig_playlist_renamed(id, old_name, new_name);
}
