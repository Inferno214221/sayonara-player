#ifndef COVEREXTRACTOR_H
#define COVEREXTRACTOR_H

#include <QObject>
#include "Utils/Pimpl.h"
class QPixmap;

namespace Cover
{
	class Location;
	class Extractor : public QObject
	{
		Q_OBJECT
		PIMPL(Extractor)

		signals:
			void sig_finished();

		public:
			Extractor(const Cover::Location& cl, QObject* parent);
			~Extractor();

			QPixmap pixmap();

		public slots:
			void start();
	};
}

#endif // COVEREXTRACTOR_H
