#ifndef TAGGING_FILESCANNER_H
#define TAGGING_FILESCANNER_H

#include <QObject>
#include "Utils/Pimpl.h"

namespace Tagging
{
	/**
	 * @brief Asynchrously extracts Metadata from directory
	 * @ingroup Tagging
	 */
	class FileScanner : public QObject
	{
		Q_OBJECT
		PIMPL(FileScanner)

		signals:
			void sig_finished();

		public:
			explicit FileScanner(const QString& path);
			~FileScanner() override;

			/**
			 * @brief Retrieve metadata that was extracted from the path
			 * @return
			 */
			MetaDataList metadata() const;

		public slots:
			void start();
	};
}

#endif // FILESCANNER_H
