#ifndef COVEREXTRACTOR_H
#define COVEREXTRACTOR_H

#include <QObject>
#include "Utils/Pimpl.h"
class QPixmap;

namespace Cover
{
	class Extractor : public QObject
	{
		Q_OBJECT
		PIMPL(Extractor)

		signals:
			void sig_finished();

		public:
			Extractor(const QString& filepath, QObject* parent);
			~Extractor();

			QPixmap pixmap();

		public slots:
			void start();
	};
}

#endif // COVEREXTRACTOR_H
