#include "CoverView.h"
#include "CoverModel.h"
#include "CoverDelegate.h"
#include "CoverViewContextMenu.h"

#include "Components/Library/LocalLibrary.h"

#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"

#include "Utils/Library/Sorting.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/Utils.h"

#include <QHeaderView>
#include <QTimer>
#include <QWheelEvent>
#include <atomic>

#include <QShortcut>
#include <QKeySequence>
using Library::CoverView;
using Library::CoverModel;
using AtomicBool=std::atomic<bool>;

struct CoverView::Private
{
	LocalLibrary*	library=nullptr;
	CoverModel*		model=nullptr;
	QTimer*			buffer_timer=nullptr;

	AtomicBool		blocked;

	Private() :
		blocked(false)
	{}
};

CoverView::CoverView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>();

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
	this->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
	this->setSelectionBehavior(QAbstractItemView::SelectItems);
	this->setItemDelegate(new Library::CoverDelegate(this));
	this->setShowGrid(false);

	if(this->horizontalHeader()){
		this->horizontalHeader()->hide();
	}

	if(this->verticalHeader()){
		this->verticalHeader()->hide();
	}

	new QShortcut(QKeySequence("Ctrl+R"), this, SLOT(reload()), nullptr, Qt::WidgetShortcut);
}

AbstractLibrary* CoverView::library() const
{
	return m->library;
}

QStringList CoverView::zoom_actions()
{
	return QStringList{"50", "75", "100", "125", "150", "175", "200"};
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

	CoverViewContextMenu* menu = static_cast<CoverViewContextMenu*>(context_menu());
	if(menu){
		menu->set_zoom(zoom);
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


QList<ActionPair> CoverView::sorting_actions()
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


void CoverView::change_sortorder(Library::SortOrder so)
{
	CoverViewContextMenu* menu = static_cast<CoverViewContextMenu*>(context_menu());
	if(menu){
		menu->set_sorting(so);
	}

	m->library->change_album_sortorder(so);
}


void CoverView::init_context_menu()
{
	if(context_menu()){
		return;
	}

	CoverViewContextMenu* cm = new CoverViewContextMenu(this);
	ItemView::init_context_menu_custom_type(cm);

	connect(cm, &CoverViewContextMenu::sig_zoom_changed, this, &CoverView::change_zoom);
	connect(cm, &CoverViewContextMenu::sig_sorting_changed, this, &CoverView::change_sortorder);

	QAction* a = cm->addAction("Refresh");


	connect(a, &QAction::triggered, m->model, [=](){
		m->model->reload();
	});
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

void CoverView::reload()
{
	m->model->reload();
}


void CoverView::language_changed() {}

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

