#include "CoverViewContextMenu.h"
#include "CoverView.h"
#include "GUI/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"

#include "Utils/ActionPair.h"

#include <QStringList>

using ActionPairList=QList<ActionPair>;

struct CoverViewContextMenu::Private
{
	QMenu*		menu_sorting=nullptr;
	QAction*	action_sorting=nullptr;

	QMenu*		menu_zoom=nullptr;
	QAction*	action_zoom=nullptr;

	QAction*	action_show_utils=nullptr;

	QStringList		zoom_actions;
	ActionPairList	sorting_actions;

	Private() :
		zoom_actions(Library::CoverView::zoom_actions()),
		sorting_actions(Library::CoverView::sorting_actions())
	{}
};

CoverViewContextMenu::CoverViewContextMenu(QWidget* parent) :
	LibraryContextMenu(parent)
{
	m = Pimpl::make<Private>();

	init();
}

CoverViewContextMenu::~CoverViewContextMenu() {}


void CoverViewContextMenu::init()
{
	this->add_preference_action(new CoverPreferenceAction(this));
	this->addSeparator();

	// insert everything before the preferences
	QAction* sep_before_prefs = this->before_preference_action();
	this->insertSeparator(sep_before_prefs);

	m->action_show_utils = new QAction(this);
	m->action_show_utils->setCheckable(true);
	m->action_show_utils->setChecked(_settings->get<Set::Lib_CoverShowUtils>());

	connect(m->action_show_utils, &QAction::triggered, this, &CoverViewContextMenu::show_utils_triggered);
	this->insertAction(sep_before_prefs, m->action_show_utils);

	m->menu_sorting = new QMenu(this);
	m->action_sorting = this->insertMenu(sep_before_prefs, m->menu_sorting);
	init_sorting_actions();

	m->menu_zoom  = new QMenu(this);
	m->action_zoom = this->insertMenu(sep_before_prefs, m->menu_zoom);
	init_zoom_actions();
}

void CoverViewContextMenu::init_sorting_actions()
{
	m->menu_sorting->clear();
	m->action_sorting->setText(Lang::get(Lang::SortBy));

	Library::Sortings sortings = _settings->get<Set::Lib_Sorting>();
	Library::SortOrder so = sortings.so_albums;

	for(const ActionPair& ap : m->sorting_actions)
	{
		QAction* a = m->menu_sorting->addAction(ap.name);

		a->setCheckable(true);
		a->setChecked(ap.so == so);
		a->setData((int) ap.so);

		connect(a, &QAction::triggered, this, &CoverViewContextMenu::action_sorting_triggered);
	}
}

void CoverViewContextMenu::init_zoom_actions()
{
	m->menu_zoom->clear();
	int zoom = _settings->get<Set::Lib_CoverZoom>();

	for(const QString& z : m->zoom_actions)
	{
		QAction* action = m->menu_zoom->addAction(z);
		action->setData(z.toInt());
		action->setCheckable(true);
		action->setChecked(zoom == z.toInt());

		connect(action, &QAction::triggered, this, &CoverViewContextMenu::action_zoom_triggered);
	}
}

void CoverViewContextMenu::show_utils_triggered(bool b)
{
	_settings->set<Set::Lib_CoverShowUtils>(b);
}

void CoverViewContextMenu::action_zoom_triggered(bool b)
{
	Q_UNUSED(b)
	QAction* action = static_cast<QAction*>(sender());

	int zoom = action->data().toInt();
	emit sig_zoom_changed(zoom);
}

void CoverViewContextMenu::action_sorting_triggered(bool b)
{
	Q_UNUSED(b)
	QAction* action = static_cast<QAction*>(sender());

	Library::SortOrder so = static_cast<Library::SortOrder>(action->data().toInt());
	emit sig_sorting_changed(so);
}


CoverViewContextMenu::Entries CoverViewContextMenu::get_entries() const
{
	CoverViewContextMenu::Entries entries = LibraryContextMenu::get_entries();
	entries |= CoverViewContextMenu::EntryShowUtils;
	entries |= CoverViewContextMenu::EntrySorting;
	entries |= CoverViewContextMenu::EntryZoom;

	return entries;
}

void CoverViewContextMenu::show_actions(CoverViewContextMenu::Entries entries)
{
	LibraryContextMenu::show_actions(entries);

	m->action_show_utils->setVisible(entries & CoverViewContextMenu::EntryShowUtils);
	m->action_sorting->setVisible(entries & CoverViewContextMenu::EntrySorting);
	m->action_zoom->setVisible(entries & CoverViewContextMenu::EntryZoom);
}

void CoverViewContextMenu::set_zoom(int zoom)
{
	bool found=false;

	const QList<QAction*> actions = m->menu_zoom->actions();
	for(QAction* a : actions)
	{
		a->setChecked( (a->text().toInt() >= zoom) && !found );
		if(a->text().toInt() >= zoom)
		{
			found = true;
		}
	}
}

void CoverViewContextMenu::set_sorting(Library::SortOrder so)
{
	const QList<QAction*> actions = m->menu_sorting->actions();
	for(QAction* a : actions)
	{
		a->setChecked(a->data().toInt() == (int) so);
	}
}


void CoverViewContextMenu::language_changed()
{
	LibraryContextMenu::language_changed();

	init_sorting_actions();
	m->action_zoom->setText(Lang::get(Lang::Zoom));
	m->action_show_utils->setText(tr("Show toolbar"));
	m->action_show_utils->setText(tr("Show utils"));
}

void CoverViewContextMenu::skin_changed()
{
	LibraryContextMenu::skin_changed();
}
