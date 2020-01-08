#ifndef DIRECTORYSELECTIONHANDLER_H
#define DIRECTORYSELECTIONHANDLER_H

#include "Utils/Pimpl.h"
#include <QObject>

namespace Library
{
	class Info;
}

class LocalLibrary;
class QStringList;

class DirectorySelectionHandler :
	public QObject
{
	Q_OBJECT
	PIMPL(DirectorySelectionHandler)

signals:
	void sig_libraries_changed();
	void sig_import_dialog_requested(const QString& target_path);

public:
	DirectorySelectionHandler(QObject* parent=nullptr);
	~DirectorySelectionHandler();

	void play_next(const QStringList& paths);
	void create_playlist(const QStringList& paths, bool create_new_playlist);
	void append_tracks(const QStringList& paths);
	void prepare_tracks_for_playlist(const QStringList& paths, bool create_new_playlist);

	void import_requested(LibraryId lib_id, const QStringList& paths, const QString& target_dir);
	void delete_paths(const QStringList& paths);

	void set_library_id(LibraryId lib_id);
	LibraryId library_id() const;

	void create_new_library(const QString& name, const QString& path);

	Library::Info library_info() const;
	LocalLibrary* library_instance() const;

	void set_search_text(const QString& text);

private slots:
	void scanner_delete_finished();
	void libraries_changed();

private:
	void create_delete_filescanner(const QStringList& files);
};


#endif // DIRECTORYSELECTIONHANDLER_H
