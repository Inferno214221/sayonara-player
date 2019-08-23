#ifndef PLAYLIST_FILESCANNER_H
#define PLAYLIST_FILESCANNER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "Utils/Playlist/PlaylistFwd.h"

namespace Playlist
{
	class FileScanner : public QObject
	{
		Q_OBJECT
		PIMPL(FileScanner)

		signals:
			void sig_finished();
			void sig_progress(const QString& path);

		public:
			explicit FileScanner(int playlist_id, const QStringList& paths, int target_row_index);
			~FileScanner() override;

			int playlist_id() const;
			int target_row_index() const;
			MetaDataList metadata() const;

		public slots:
			void start();
	};
}

#endif // FILESCANNER_H
