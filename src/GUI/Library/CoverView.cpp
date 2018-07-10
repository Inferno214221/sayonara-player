#include "CoverView.h"
#include "CoverModel.h"
#include "CoverDelegate.h"

#include "Components/Covers/CoverChangeNotifier.h"
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

using Library::CoverModel;

struct CoverView::Private
{
	LocalLibrary*		library=nullptr;
	CoverModel*			model=nullptr;
	QMenu*				menu_sortings=nullptr;
	QAction*			action_sorting=nullptr;
	QMenu*				menu_zoom=nullptr;
	QAction*			action_zoom=nullptr;
	QAction*			action_show_utils=nullptr;

	QTimer*				buffer_timer=nullptr;

	std::atomic<bool>	blocked;

	Private(CoverView* cover_view) :
		blocked(false)
	{
		menu_sortings = new QMenu(cover_view);
		menu_zoom = new QMenu(cover_view);

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
};

CoverView::CoverView(QWidget* parent) :
	Library::ItemView(parent)
{
	m = Pimpl::make<Private>(this);


}

CoverView::~CoverView() {}

void CoverView::init(LocalLibrary* library)
{
	m->library = library;
	m->model = new Library::CoverModel(this, library);

	ItemView::set_selection_type( SelectionViewInterface::SelectionType::Items );
	ItemView::set_metadata_interpretation(MD::Interpretation::Albums);
	ItemView::set_item_model(m->model);
	ItemView::set_search_model(m->model);

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

	init_sorting_actions();

	connect(m->library, &AbstractLibrary::sig_all_albums_loaded, this, &CoverView::albums_ready);
	connect(m->buffer_timer, &QTimer::timeout, this, &CoverView::timed_out, Qt::QueuedConnection);

	Cover::ChangeNotfier* ccn = Cover::ChangeNotfier::instance();
	connect(ccn, &Cover::ChangeNotfier::sig_covers_changed, this, &CoverView::cover_changed);
}

void CoverView::albums_ready()
{
	if(this->isVisible())
	{
		m->model->refresh_data();
	}
}

QList<ActionPair> CoverView::sorting_options() const
{
	QList<ActionPair> ret;
	ActionPair ap;
	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Name),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumNameAsc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Name),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumNameDesc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Year),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumYearAsc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Year),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumYearDesc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::NumTracks),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumTracksAsc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::NumTracks),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumTracksDesc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Duration),
						 Lang::get(Lang::Ascending)),
					Library::SortOrder::AlbumDurationAsc
					);

	ret << ap;

	ap = ActionPair(QString("%1 (%2)")
					.arg(Lang::get(Lang::Duration),
						 Lang::get(Lang::Descending)),
					Library::SortOrder::AlbumDurationDesc
					);

	ret << ap;
	return ret;

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

	if(m->model->rowCount() == 0){
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

	refresh();
	emit sig_zoom_changed(zoom);
}


void CoverView::action_zoom_triggered()
{
	QAction* action = static_cast<QAction*>(sender());
	int zoom = action->text().toInt();

	change_zoom(action->text().toInt());

	emit sig_zoom_changed(zoom);
}



void CoverView::init_sorting_actions()
{
	init_context_menu();

	m->action_sorting->setText(Lang::get(Lang::SortBy));
	m->menu_sortings->clear();

	const QList<ActionPair> action_pairs = sorting_options();
	for(const ActionPair& ap : action_pairs)
	{
		QAction* a = m->menu_sortings->addAction(ap.name);
		a->setCheckable(true);
		a->setData((int) ap.so);
	}

	Library::Sortings sortings = _settings->get<Set::Lib_Sorting>();
	Library::SortOrder so = sortings.so_albums;

	const QList<QAction*> actions = m->menu_sortings->actions();
	for(QAction* action : actions)
	{
		action->setCheckable(true);

		if(action->data().toInt() == (int) so){
			action->setChecked(true);
		}

		connect(action, &QAction::triggered, this, &CoverView::action_sortorder_triggered);
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

	emit sig_sortorder_changed(so);
}


void CoverView::cover_changed()
{
	m->model->reload();
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


void CoverView::refresh()
{
	if(m->model->rowCount() == 0){
		return;
	}

	m->buffer_timer->start();
}


void CoverView::language_changed()
{
	init_sorting_actions();

	m->action_zoom->setText(Lang::get(Lang::Zoom));
	m->action_show_utils->setText(tr("Show toolbar"));
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
	ItemView::selection_changed(indexes);
	if(!m->library){
		return;
	}

	m->library->selected_albums_changed(indexes);
}

void CoverView::show_utils_triggered(bool b)
{
	_settings->set<Set::Lib_CoverShowUtils>(b);
}


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
