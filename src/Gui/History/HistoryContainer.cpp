#include "HistoryContainer.h"
#include "Gui/History/GUI_History.h"
#include "Gui/Utils/Icons.h"

struct HistoryContainer::Private
{
	GUI_History* widget=nullptr;
};

HistoryContainer::HistoryContainer(QObject* parent) :
	Library::ContainerImpl(parent)
{
	m = Pimpl::make<Private>();
}

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
	return 	m->widget->header();
}

QPixmap HistoryContainer::icon() const
{
	return Gui::Icons::pixmap(Gui::Icons::Edit);
}

void HistoryContainer::initUi()
{
	m->widget = new GUI_History();
}
