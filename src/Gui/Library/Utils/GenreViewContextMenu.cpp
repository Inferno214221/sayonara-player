#include "GenreViewContextMenu.h"
#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"

using Library::GenreViewContextMenu;

struct GenreViewContextMenu::Private
{
	QAction* toggle_tree_action=nullptr;
};

GenreViewContextMenu::GenreViewContextMenu(QWidget* parent) :
	ContextMenu(parent)
{
	m = Pimpl::make<Private>();

	bool show_tree = GetSetting(Set::Lib_GenreTree);
	m->toggle_tree_action = new QAction(this);
	m->toggle_tree_action->setCheckable(true);
	m->toggle_tree_action->setChecked(show_tree);
	m->toggle_tree_action->setText(Lang::get(Lang::Tree));

	this->register_action(m->toggle_tree_action);

	connect( m->toggle_tree_action, &QAction::triggered, this, &GenreViewContextMenu::toggle_tree_triggered);
}

GenreViewContextMenu::~GenreViewContextMenu() = default;

void GenreViewContextMenu::toggle_tree_triggered()
{
	SetSetting(Set::Lib_GenreTree, m->toggle_tree_action->isChecked());
}

void GenreViewContextMenu::language_changed()
{
	m->toggle_tree_action->setText(Lang::get(Lang::Tree));
}
