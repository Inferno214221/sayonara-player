#ifndef DIRECTORYFILESCANNER_H
#define DIRECTORYFILESCANNER_H

#include "Utils/Pimpl.h"
#include <QObject>

class MetaDataList;

namespace Directory
{
	class MetaDataScanner :
		public QObject
	{
		Q_OBJECT
		PIMPL(MetaDataScanner)

		signals:
			void sig_finished();
			void sig_current_path(const QString& path);

		public:
			explicit MetaDataScanner(const QStringList& files, bool recursive, QObject *parent=nullptr);
			~MetaDataScanner() override;

			MetaDataList metadata() const;
			QStringList files() const;

			void set_scan_audio_files(bool b);
			void set_scan_playlist_files(bool b);

			void set_data(void* data_object);
			void* data() const;

		public slots:
			void start();
	};
}


#endif // DIRECTORYFILESCANNER_H
