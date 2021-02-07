#ifndef HISTORYCONTAINER_H
#define HISTORYCONTAINER_H

#include "Gui/Library/LibraryContainer.h"
#include "Utils/Pimpl.h"

namespace Session
{
	class Manager;
}

class HistoryContainer :
	public Library::Container
{
	Q_OBJECT
	PIMPL(HistoryContainer)

	public:
		HistoryContainer(Session::Manager* sessionManager, QObject* parent = nullptr);
		~HistoryContainer() override;

		// Container interface
	public:
		QString name() const override;
		QString displayName() const override;
		QWidget* widget() const override;
		QFrame* header() const override;
		QPixmap icon() const override;

		// ContainerImpl interface
	protected:
		void initUi() override;
};

#endif // HISTORYCONTAINER_H
