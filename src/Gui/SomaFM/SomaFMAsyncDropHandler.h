#ifndef SOMAFMASYNCDROPHANDLER_H
#define SOMAFMASYNCDROPHANDLER_H

#include "Utils/MimeData/DragDropAsyncHandler.h"

#include <QList>
class QUrl;

namespace SomaFM
{
	class Station;
	class AsyncDropHandler : public Gui::AsyncDropHandler
	{
		Q_OBJECT
		PIMPL(AsyncDropHandler)

		public:
			AsyncDropHandler(const SomaFM::Station& station, QObject* parent);
			~AsyncDropHandler() override;

		public slots:
			void start() override;

		private slots:
			void streamParserFinished(bool success);
	};
}

#endif // SOMAFMASYNCDROPHANDLER_H
