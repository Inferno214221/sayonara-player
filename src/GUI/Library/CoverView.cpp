#include "CoverView.h"
#include "CoverModel.h"
#include "CoverDelegate.h"

#include "Components/Library/LocalLibrary.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/PreferenceAction.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"

#include <QHeaderView>
#include <QTimer>
#include <QWheelEvent>
#include <atomic>

using Library::CoverView;
using Library::CoverModel;

struct CoverView::Private
{
	LocalLibrary*	library=nullptr;
	CoverModel*		model=nullptr;
	QMenu*			menu_sortings=nullptr;
	QMenu*			menu_zoom=nullptr;
	QAction*		action_sorting=nullptr;
	QAction*		action_zoom=nullptr;
	QAction*		action_show_utils=nullptr;

	QTimer*			buffer_timer=nullptr;

	std::atomic<bool>	blocked;

	Private(CoverView* cover_view) :
		blocked(false)
	{
		menu_sortings = new QMenu(cover_view);
		menu_zoom = new QMenu(cover_view);
	}
};

CoverView::CoverView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>(this);

	connect(this, &ItemView::doubleClicked, this, &CoverView::play_clicked);
}

CoverView::~CoverView()
{
	if(m->buffer_timer){

		while(m->buffer_timer->isActive())
		{
			m->buffer_timer->stop();
			::Util::sleep_ms(10);
		}

		delete m->buffer_timer; m->buffer_timer = nullptr;
	}
}

void CoverView::init(LocalLibrary* library)
{
	m->library = library;
	m->model = new Library::CoverModel(this, library);

	ItemView::set_selection_type( SelectionViewInterface::SelectionType::Items );
	ItemView::set_metadata_interpretation(MD::Interpretation::Albums);
	ItemView::set_item_model(m->model);

	this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setItemDelegate(new Library::CoverDelegate(this));
	this->setShowGrid(false);

	if(this->horizontalHeader()){
		this->horizontalHeader()->hide();
	}

	if(this->verticalHeader()){
		this->verticalHeader()->hide();
	}
}

AbstractLibrary* CoverView::library() const
{
	return m->library;
}

QStringList CoverView::zoom_actions() const
{
	return QStringList{"50", "75", "100", "125", "150", "175", "200"};
}

void CoverView::init_zoom_actions()
{
	m->menu_zoom->clear();

	const QStringList actions = zoom_actions();
	for(const QString& z : actions)
	{
		QAction* action = m->menu_zoom->addAction(z);
		action->setCheckable(true);

		connect(action, &QAction::triggered, this, &CoverView::action_zoom_triggered);
	}
}

void CoverView::change_zoom(int zoom)
{
	bool force_reload = (zoom < 0);

	if(row_count() == 0){
		return;
	}

	if(force_reload){
		zoom = m->model->zoom();
	}

	else
	{
		if(zoom == m->model->zoom()){
			return;
		}
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

	if(!force_reload)
	{
		if( zoom == m->model->zoom() )
		{
			return;
		}
	}

	m->model->set_zoom(zoom, this->size());
	_settings->set<Set::Lib_CoverZoom>(zoom);

	timer_start();
}


void CoverView::action_zoom_triggered()
{
	QAction* action = static_cast<QAction*>(sender());
	int zoom = action->text().toInt();

	change_zoom(zoom);
}


QList<ActionPair> CoverView::sorting_actions() const
{
	using namespace Library;

	QList<ActionPair> ret
	{
		ActionPair(Lang::Name, Lang::Ascending, SortOrder::AlbumNameAsc),
		ActionPair(Lang::Name, Lang::Descending, SortOrder::AlbumNameDesc),
		ActionPair(Lang::Year, Lang::Ascending, SortOrder::AlbumYearAsc),
		ActionPair(Lang::Year, Lang::Descending, SortOrder::AlbumYearDesc),
		ActionPair(Lang::NumTracks, Lang::Ascending, SortOrder::AlbumTracksAsc),
		ActionPair(Lang::NumTracks, Lang::Descending, SortOrder::AlbumTracksDesc),
		ActionPair(Lang::Duration, Lang::Ascending, SortOrder::AlbumDurationAsc),
		ActionPair(Lang::Duration, Lang::Descending, SortOrder::AlbumDurationDesc)
	};

	return ret;
}


void CoverView::init_sorting_actions()
{
	if(!context_menu()){
		return;
	}

	m->menu_sortings->clear();
	m->action_sorting->setText(Lang::get(Lang::SortBy));

	Library::Sortings sortings = _settings->get<Set::Lib_Sorting>();
	Library::SortOrder so = sortings.so_albums;

	const QList<ActionPair> action_pairs = sorting_actions();
	for(const ActionPair& ap : action_pairs)
	{
		QAction* a = m->menu_sortings->addAction(ap.name);

		a->setCheckable(true);
		a->setChecked(ap.so == so);
		a->setData((int) ap.so);

		connect(a, &QAction::triggered, this, &CoverView::action_sortorder_triggered);
	}
}


void CoverView::change_sortorder(Library::SortOrder so)
{
	const QList<QAction*> actions = m->menu_sortings->actions();
	for(QAction* a : actions)
	{
		a->setChecked(a->data().toInt() == (int) so);
	}

	m->library->change_album_sortorder(so);
}


void CoverView::action_sortorder_triggered()
{
	QAction* a = static_cast<QAction*>(sender());
	Library::SortOrder so = static_cast<Library::SortOrder>(a->data().toInt());

	change_sortorder(so);
}


void CoverView::init_context_menu()
{
	if(context_menu()){
		return;
	}

	ItemView::init_context_menu();

	LibraryContextMenu* menu = context_menu();
	menu->add_preference_action(new CoverPreferenceAction(menu));
	menu->addSeparator();

	// insert everything before the preferences
	QAction* sep_before_prefs = menu->before_preference_action();
	menu->insertSeparator(sep_before_prefs);

	m->action_show_utils = new QAction(menu);
	m->action_show_utils->setCheckable(true);
	m->action_show_utils->setChecked(_settings->get<Set::Lib_CoverShowUtils>());

	connect(m->action_show_utils, &QAction::triggered, this, &CoverView::show_utils_triggered);
	menu->insertAction(sep_before_prefs, m->action_show_utils);

	m->menu_sortings = new QMenu(menu);
	m->action_sorting = menu->insertMenu(sep_before_prefs, m->menu_sortings);
	init_sorting_actions();

	m->menu_zoom  = new QMenu(menu);
	m->action_zoom = menu->insertMenu(sep_before_prefs, m->menu_zoom);
	init_zoom_actions();

	language_changed();
}


void CoverView::timer_start()
{
	if(this->is_empty()){
		return;
	}

	if(!m->buffer_timer)
	{
		m->buffer_timer = new QTimer();
		m->buffer_timer->setInterval(10);
		m->buffer_timer->setSingleShot(true);
		connect(m->buffer_timer, &QTimer::timeout, this, &CoverView::timer_timed_out, Qt::QueuedConnection);
	}

	m->buffer_timer->start();
}


void CoverView::timer_timed_out()
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


void CoverView::language_changed()
{
	if(context_menu())
	{
		init_sorting_actions();
		m->action_zoom->setText(Lang::get(Lang::Zoom));
		m->action_show_utils->setText(tr("Show toolbar"));
		m->action_show_utils->setText(tr("Show utils"));
	}
}


QStyleOptionViewItem CoverView::viewOptions() const
{
	QStyleOptionViewItem option = ItemView::viewOptions();
	option.decorationAlignment = Qt::AlignHCenter;
	option.displayAlignment = Qt::AlignHCenter;
	option.decorationPosition = QStyleOptionViewItem::Top;

	return option;
}


void CoverView::wheelEvent(QWheelEvent* e)
{
	if( (e->modifiers() & Qt::ControlModifier) && (e->delta() != 0) )
	{
		int d = (e->delta() > 0) ? 10 : -10;

		change_zoom(m->model->zoom() + d);
	}

	else
	{
		ItemView::wheelEvent(e);
	}
}

void CoverView::resizeEvent(QResizeEvent* e)
{
	ItemView::resizeEvent(e);
	change_zoom();
}

void CoverView::hideEvent(QHideEvent* e)
{
	if(m->model){
		m->model->clear();
	}

	ItemView::hideEvent(e);
}


void CoverView::show_utils_triggered(bool b)
{
	_settings->set<Set::Lib_CoverShowUtils>(b);
}


int CoverView::index_by_model_index(const QModelIndex& idx) const
{
	return idx.row() * model()->columnCount() + idx.column();
}

ModelIndexRange CoverView::model_indexrange_by_index(int idx) const
{
	int row = idx / model()->columnCount();
	int col = idx % model()->columnCount();

	return ModelIndexRange(model()->index(row, col), model()->index(row, col));
}

void CoverView::play_clicked()
{
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void CoverView::play_new_tab_clicked()
{
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void CoverView::play_next_clicked()
{
	m->library->play_next_fetched_tracks();
}

void CoverView::append_clicked()
{
	m->library->append_fetched_tracks();
}

void CoverView::selection_changed(const IndexSet& indexes)
{
	m->library->selected_albums_changed(indexes);
}

void CoverView::refresh_clicked()
{
	m->library->refresh_albums();
}

void CoverView::run_merge_operation(const Library::ItemView::MergeData& mergedata)
{
	m->library->merge_albums(mergedata.source_ids, mergedata.target_id);
}

