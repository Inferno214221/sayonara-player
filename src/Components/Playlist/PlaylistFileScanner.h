#ifndef FILESCANNER_H
#define FILESCANNER_H

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

		public:
			explicit FileScanner(const QStringList& paths, int playlist_id);
			~FileScanner() override;

			int playlist_id() const;
			MetaDataList metadata() const;

		public slots:
			void start();
	};
}

#endif // FILESCANNER_H
