#include "ContextMenu.h"
#include "Utils/Language/Language.h"
#include "Gui/Utils/Icons.h"
#include <QAction>

using SC::ContextMenu;

struct ContextMenu::Private
{
	QAction* actionAddArtist;

	Private()
	{
		actionAddArtist = new QAction();
	}
};

ContextMenu::ContextMenu(QWidget* parent) : Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	this->insertAction(this->beforePreferenceAction(), m->actionAddArtist);
	connect(m->actionAddArtist, &QAction::triggered, this, &SC::ContextMenu::sigAddArtistTriggered);
}

ContextMenu::~ContextMenu() = default;

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	m->actionAddArtist->setText(Lang::get(Lang::AddArtist));
}

ContextMenu::Entries ContextMenu::entries() const
{
	ContextMenu::Entries entries = Library::ContextMenu::entries();

	if(m->actionAddArtist->isVisible()) {
		entries |= ContextMenu::SCEntryAddArtist;
	}

	return entries;
}

void ContextMenu::showActions(ContextMenu::Entries entries)
{
	m->actionAddArtist->setVisible(entries & SCEntryAddArtist);

	Library::ContextMenu::showActions(entries);
}

void ContextMenu::showAction(ContextMenu::Entry entry, bool visible)
{
	ContextMenu::Entries entries = this->entries();
	if(visible) {
		entries |= entry;
	}

	else {
		entries &= ~(entry);
	}

	SC::ContextMenu::showActions(entries);
}
