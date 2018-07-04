/* AlbumCoverView.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "CoverView.h"

#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Library/AbstractLibrary.h"

#include "GUI/Library/CoverModel.h"
#include "GUI/Library/CoverDelegate.h"
#include "GUI/Utils/Widgets/ComboBox.h"
#include "GUI/Utils/Icons.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"

#include <QHeaderView>
#include <QWheelEvent>
#include <QTimer>
#include <QMenu>
#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QFrame>

#include <atomic>

struct ActionPair
{
	QString name;
	Library::SortOrder so;

	ActionPair() {}
	ActionPair(const QString& name, Library::SortOrder so) :
		name(name),
		so(so)
	{}
};

using namespace Library;

struct CoverView::Private
{

	QStringList			zoom_actions;
	QList<ActionPair>	sorting_actions;

	AbstractLibrary*	library=nullptr;
	CoverModel*			model=nullptr;
	QWidget*			topbar=nullptr;
	Gui::ComboBox*		combo_sorting=nullptr;
	QLabel*				label_sorting=nullptr;
	Gui::ComboBox*		combo_zoom=nullptr;
	QLabel*				label_zoom=nullptr;

	QTimer*				buffer_timer=nullptr;
	QMenu*				menu_sortings=nullptr;
	QAction*			action_sorting=nullptr;
	QMenu*				menu_zoom=nullptr;
	QAction*			action_zoom=nullptr;
	QAction*			action_show_utils=nullptr;

	std::atomic<bool>	blocked;

	Private(CoverView* cover_view, AbstractLibrary* library, QWidget* topbar) :
		zoom_actions {"50", "75", "100", "125", "150", "175", "200"},
		library(library),
		topbar(topbar),
		blocked(false)
	{
		model = new Library::CoverModel(cover_view, library);

		combo_zoom = new Gui::ComboBox(topbar);
		label_zoom = new QLabel(topbar);
		label_zoom->setText(Lang::get(Lang::Zoom).append(":"));

		combo_sorting = new Gui::ComboBox(topbar);
		combo_sorting->setEditable(false);
		label_sorting = new QLabel(topbar);
		label_sorting->setText(Lang::get(Lang::SortBy).append(":"));

		buffer_timer = new QTimer();
		buffer_timer->setInterval(10);
		buffer_timer->setSingleShot(true);
	}

	~Private()
	{
		while(buffer_timer->isActive()){
			buffer_timer->stop();
			::Util::sleep_ms(10);
		}

		delete buffer_timer; buffer_timer = nullptr;
	}

	void add_sorting_items()
	{
		for(const ActionPair& ap : ::Util::AsConst(sorting_actions))
		{
			if(combo_sorting)
			{
				combo_sorting->addItem(ap.name, (int) ap.so);
			}

			if(menu_sortings)
			{
				QAction* a = menu_sortings->addAction(ap.name);
				a->setCheckable(true);
				a->setData((int) ap.so);
			}
		}
	}
};

CoverView::CoverView(AbstractLibrary* library, QWidget* topbar, QWidget* parent) :
	ItemView(parent)
{
	m = Pimpl::make<Private>(this, library, topbar);

	ItemView::set_item_model(m->model);
	ItemView::set_search_model(m->model);

	init_sorting_actions();

	m->topbar->layout()->setContentsMargins(0, 0, 0, 0);
	m->topbar->layout()->addWidget(m->label_sorting);
	m->topbar->layout()->addWidget(m->combo_sorting);
	m->topbar->layout()->addItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
	m->topbar->layout()->addWidget(m->label_zoom);
	m->topbar->layout()->addWidget(m->combo_zoom);

	m->topbar->setVisible(_settings->get<Set::Lib_CoverShowUtils>());

	connect(m->combo_sorting, SIGNAL(activated(int)), this, SLOT(combo_sorting_changed(int)));
	connect(m->combo_zoom, SIGNAL(activated(int)), this, SLOT(combo_zoom_changed(int)));
	connect(this, &ItemView::doubleClicked, this, &CoverView::double_clicked);
	connect(m->library, &AbstractLibrary::sig_all_albums_loaded, this, &CoverView::albums_ready);

	set_selection_type( SelectionViewInterface::SelectionType::Items );
	set_metadata_interpretation(MD::Interpretation::Albums);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setItemDelegate(new CoverDelegate(this));
	this->setShowGrid(false);

	if(horizontalHeader()){
		horizontalHeader()->hide();
	}

	if(verticalHeader()){
		verticalHeader()->hide();
	}

	connect(m->buffer_timer, &QTimer::timeout, this, &CoverView::timed_out, Qt::QueuedConnection);
	Cover::ChangeNotfier* ccn = Cover::ChangeNotfier::instance();

	connect(ccn, &Cover::ChangeNotfier::sig_covers_changed, this, &CoverView::cover_changed);
}

CoverView::~CoverView() {}

int CoverView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row() * model()->columnCount() + idx.column();
}

QModelIndex CoverView::model_index_by_index(int idx) const
{
	int row = idx / model()->columnCount();
	int col = idx % model()->columnCount();

	return model()->index(row, col);
}


void CoverView::change_zoom(int zoom)
{
	bool force_reload = (zoom < 0);

	if(m->model->rowCount() == 0){
		return;
	}

	if(force_reload){
		zoom = m->model->zoom();
	}

	zoom = std::min(zoom, 200);
	zoom = std::max(zoom, 50);

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

	for(int i=0; i<m->combo_zoom->count(); i++)
	{
		if(m->combo_zoom->itemText(i).toInt() >= zoom)
		{
			m->combo_zoom->setCurrentIndex(i);
			break;
		}
	}

	if(!force_reload)
	{
		if( zoom == m->model->zoom() )
		{
			return;
		}
	}

	m->model->set_zoom(zoom, this->size());
	_settings->set<Set::Lib_CoverZoom>(zoom);

	refresh();
}

void CoverView::refresh()
{
	if(m->model->rowCount() == 0){
		return;
	}

	m->buffer_timer->start();
}

void CoverView::timed_out()
{
	if(m->blocked){
		return;
	}

	m->blocked = true;

	this->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	this->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

	m->buffer_timer->stop();
	m->blocked = false;
}

QStyleOptionViewItem CoverView::viewOptions() const
{
	QStyleOptionViewItem option = ItemView::viewOptions();
	option.decorationAlignment = Qt::AlignHCenter;
	option.displayAlignment = Qt::AlignHCenter;
	option.decorationPosition = QStyleOptionViewItem::Top;

	return option;
}

void CoverView::init_context_menu()
{
	ItemView::init_context_menu();
	if(m->menu_sortings){
		return;
	}

	LibraryContextMenu* menu = context_menu();

	menu->add_preference_action(new CoverPreferenceAction(menu));

	menu->addSeparator();

	m->action_show_utils = menu->addAction("Show utils");
	m->action_show_utils->setCheckable(true);
	m->action_show_utils->setChecked(_settings->get<Set::Lib_CoverShowUtils>());
	connect(m->action_show_utils, &QAction::triggered, this, &CoverView::show_utils_triggered);

	m->menu_sortings = new QMenu(menu);
	m->action_sorting = menu->addMenu(m->menu_sortings);
	init_sorting_actions();

	m->menu_zoom  = new QMenu(menu);
	m->action_zoom = menu->addMenu(m->menu_zoom);
	init_zoom_actions();

	language_changed();
}

void CoverView::init_sorting_actions()
{
	Library::Sortings sortings = _settings->get<Set::Lib_Sorting>();
	Library::SortOrder so = sortings.so_albums;

	m->sorting_actions.clear();

	m->label_sorting->setText(Lang::get(Lang::SortBy));

	if(m->action_sorting){
		m->action_sorting->setText(Lang::get(Lang::SortBy));
	}

	if(m->menu_sortings){
		m->menu_sortings->clear();
	}

	if(m->combo_sorting){
		m->combo_sorting->clear();
	}

	ActionPair ap;
	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Name),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumNameAsc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Name),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumNameDesc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Year),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumYearAsc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Year),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumYearDesc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::NumTracks),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumTracksAsc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::NumTracks),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumTracksDesc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Duration),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumDurationAsc
					);

	m->sorting_actions << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Duration),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumDurationDesc
					);

	m->sorting_actions << ap;

	m->add_sorting_items();

	if(m->menu_sortings)
	{
		const QList<QAction*> actions = m->menu_sortings->actions();
		for(QAction* action : actions)
		{
			action->setCheckable(true);

			if(action->data().toInt() == (int) so){
				action->setChecked(true);
			}

			connect(action, &QAction::triggered, this, &CoverView::menu_sorting_triggered);
		}
	}
}


void CoverView::change_sortorder(SortOrder so)
{
	const QList<QAction*> actions = m->menu_sortings->actions();
	for(QAction* a : actions)
	{
		a->setChecked(a->data().toInt() == (int) so);
	}

	for(int i=0; i<m->combo_sorting->count(); i++)
	{
		if(m->combo_sorting->itemData(i).toInt() == (int) so)
		{
			m->combo_sorting->setCurrentIndex(i);
		}
	}

	m->library->change_album_sortorder(so);
}

void CoverView::menu_sorting_triggered()
{
	QAction* a = static_cast<QAction*>(sender());
	int data = a->data().toInt();
	change_sortorder((Library::SortOrder) data);
}

void CoverView::combo_sorting_changed(int idx)
{
	Q_UNUSED(idx)

	int data = m->combo_sorting->currentData().toInt();
	change_sortorder((Library::SortOrder) data);
}


void CoverView::init_zoom_actions()
{
	m->combo_zoom->addItems(m->zoom_actions);

	for(const QString& z : ::Util::AsConst(m->zoom_actions))
	{
		QAction* action = m->menu_zoom->addAction(z);
		action->setCheckable(true);

		connect(action, &QAction::triggered, this, [=](){
			this->change_zoom(action->text().toInt());
		});
	}
}

void CoverView::combo_zoom_changed(int idx)
{
	Q_UNUSED(idx)

	change_zoom(m->combo_zoom->currentText().toInt());
}

void CoverView::show_utils_triggered()
{
	bool b = m->action_show_utils->isChecked();
	m->topbar->setVisible(b);
	_settings->set<Set::Lib_CoverShowUtils>(b);
}

void CoverView::wheelEvent(QWheelEvent* e)
{
	if( (e->modifiers() & Qt::ControlModifier) &&
			(e->delta() != 0) )
	{
		int zoom;
		if(e->delta() > 0){
			zoom = m->model->zoom() + 10;
		}

		else {
			zoom = m->model->zoom() - 10;
		}

		change_zoom(zoom);
	}

	else {
		ItemView::wheelEvent(e);
	}
}

void CoverView::resizeEvent(QResizeEvent* e)
{
	ItemView::resizeEvent(e);
	change_zoom();
}

void Library::CoverView::cover_changed()
{
	m->model->reload();
}


void CoverView::middle_clicked()
{
	ItemView::middle_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void CoverView::play_next_clicked()
{
	ItemView::play_next_clicked();
	m->library->play_next_fetched_tracks();
}

void CoverView::append_clicked()
{
	ItemView::append_clicked();
	m->library->append_fetched_tracks();
}

void CoverView::double_clicked(const QModelIndex& index)
{
	Q_UNUSED(index)
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void CoverView::selection_changed(const IndexSet& indexes)
{
	if(!m->library){
		return;
	}

	m->library->selected_albums_changed(indexes);
}

void CoverView::albums_ready()
{
	if(this->isVisible()){
		m->model->refresh_data();
	}
}

void CoverView::language_changed()
{
	ItemView::language_changed();
	init_sorting_actions();

	m->combo_zoom->setToolTip(tr("Use Ctrl + mouse wheel to zoom"));
	m->label_sorting->setText(Lang::get(Lang::SortBy));
	m->label_zoom->setText(Lang::get(Lang::Zoom));

	if(context_menu())
	{
		m->action_zoom->setText(Lang::get(Lang::Zoom));
		m->action_show_utils->setText(tr("Show toolbar"));
	}
}
