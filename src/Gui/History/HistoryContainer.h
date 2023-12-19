#ifndef HISTORYCONTAINER_H
#define HISTORYCONTAINER_H

#include "Gui/Library/LibraryContainer.h"
#include "Utils/Pimpl.h"

namespace Session
{
	class Manager;
}

namespace Library
{
	class PluginHandler;
}

class LibraryPlaylistInteractor;
class HistoryContainer :
	public Gui::Library::Container
{
	PIMPL(HistoryContainer)

	public:
		HistoryContainer(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager,
		                 Library::PluginHandler* pluginHandler);
		~HistoryContainer() override;

		[[nodiscard]] QFrame* header() const override;
		[[nodiscard]] QIcon icon() const override;
		[[nodiscard]] QMenu* menu() override;
		[[nodiscard]] QString displayName() const override;
		[[nodiscard]] QString name() const override;
		[[nodiscard]] QWidget* widget() const override;
		[[nodiscard]] bool isLocal() const override;
		void rename(const QString& newName) override;

	protected:
		void initUi() override;
};

#endif // HISTORYCONTAINER_H
