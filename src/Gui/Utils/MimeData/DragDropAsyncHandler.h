#ifndef DRAGDROPASYNCHANDLER_H
#define DRAGDROPASYNCHANDLER_H

#include <QObject>
#include "Utils/Pimpl.h"

class MetaDataList;

namespace Gui
{
	class AsyncDropHandler : public QObject
	{
		Q_OBJECT
		PIMPL(AsyncDropHandler)

		signals:
			void sigFinished();

		public:
			explicit AsyncDropHandler(QObject* parent=nullptr);
			~AsyncDropHandler() override;

			void setTargetIndex(int index);
			[[nodiscard]] int targetIndex() const;
			[[nodiscard]] virtual MetaDataList tracks() const;

		protected:
			void setTracks(const MetaDataList& tracks);

		public slots:
			virtual void start()=0;
	};
}

#endif // DRAGDROPASYNCHANDLER_H
