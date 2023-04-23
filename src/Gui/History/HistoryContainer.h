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

		[[nodiscard]] QString name() const override;
		[[nodiscard]] QString displayName() const override;
		[[nodiscard]] QWidget* widget() const override;
		[[nodiscard]] QFrame* header() const override;
		[[nodiscard]] QIcon icon() const override;

	protected:
		void initUi() override;
};

#endif // HISTORYCONTAINER_H
