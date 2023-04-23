#include "HistoryContainer.h"

#include "Components/Session/Session.h"
#include "Gui/History/GUI_History.h"
#include "Gui/Utils/Icons.h"

#include <QIcon>

struct HistoryContainer::Private
{
	GUI_History* widget = nullptr;
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	Session::Manager* sessionManager;

	Private(LibraryPlaylistInteractor* libraryPlaylistInteractor, Session::Manager* sessionManager) :
		libraryPlaylistInteractor {libraryPlaylistInteractor},
		sessionManager(sessionManager) {}
};

HistoryContainer::HistoryContainer(LibraryPlaylistInteractor* libraryPlaylistInteractor,
                                   Session::Manager* sessionManager, QObject* parent) :
	Library::Container(parent),
	m {Pimpl::make<Private>(libraryPlaylistInteractor, sessionManager)} {}

HistoryContainer::~HistoryContainer() = default;

QString HistoryContainer::name() const
{
	return "history";
}

QString HistoryContainer::displayName() const
{
	return tr("History");
}

QWidget* HistoryContainer::widget() const
{
	return m->widget;
}

QFrame* HistoryContainer::header() const
{
	return m->widget->header();
}

QIcon HistoryContainer::icon() const
{
	return Gui::Icons::icon(Gui::Icons::Edit);
}

void HistoryContainer::initUi()
{
	m->widget = new GUI_History(m->libraryPlaylistInteractor, m->sessionManager);
}
