#include "HistoryTableView.h"
#include "HistoryEntryModel.h"

#include "Components/Playlist/LibraryPlaylistInteractor.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/EventFilter.h"
#include "Gui/Utils/Widgets/HeaderView.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QDrag>
#include <QScrollBar>
#include <QStringListModel>

struct HistoryTableView::Private
{
	LibraryPlaylistInteractor* libraryPlaylistInteractor;
	HistoryEntryModel* model;
	Library::ContextMenu* contextMenu {nullptr};

	Private(LibraryPlaylistInteractor* libraryPlaylistInteractor,
	        Session::Manager* sessionManager,
	        const Session::Timecode timecode) :
		libraryPlaylistInteractor {libraryPlaylistInteractor},
		model {new HistoryEntryModel(sessionManager, timecode)} {}
};

HistoryTableView::HistoryTableView(LibraryPlaylistInteractor* libraryPlaylistInteractor,
                                   Session::Manager* sessionManager,
                                   const Session::Timecode timecode,
                                   QWidget* parent) :
	Gui::WidgetTemplate<QTableView>(parent),
	Gui::Dragable(this),
	m {Pimpl::make<Private>(libraryPlaylistInteractor, sessionManager, timecode)}
{
	setModel(m->model);
	setAlternatingRowColors(true);
	setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
	setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	verticalHeader()->setVisible(false);
	setShowGrid(false);

	auto* horizontalHeader = new Gui::HeaderView(Qt::Orientation::Horizontal, this);
	setHorizontalHeader(horizontalHeader);
	horizontalHeader->setStretchLastSection(true);
	horizontalHeader->setSectionResizeMode(0, QHeaderView::Interactive);
	horizontalHeader->setSectionResizeMode(1, QHeaderView::Stretch);
	horizontalHeader->setSectionResizeMode(2, QHeaderView::Stretch);
	horizontalHeader->setSectionResizeMode(3, QHeaderView::Stretch);

	connect(m->model, &HistoryEntryModel::sigRowsAdded, this, &HistoryTableView::rowcountChanged);
	connect(this, &QTableView::doubleClicked, this, [this](const auto& /*index*/) {
		playTriggered();
	});

	auto* eventFilter = new Gui::MousePressedFilter(this);
	connect(eventFilter, &Gui::MousePressedFilter::sigMousePressed, this, [this](auto* mouseEvent) {
		if(mouseEvent->button() == Qt::MiddleButton)
		{
			playNewTabTriggered();
		}
	});
	installEventFilter(eventFilter);
}

HistoryTableView::~HistoryTableView() = default;

int HistoryTableView::rows() const { return model()->rowCount(); }

void HistoryTableView::rowcountChanged()
{
	skinChanged();
	emit sigRowcountChanged();
}

void HistoryTableView::skinChanged()
{
	if(isVisible())
	{
		const int rows = m->model->rowCount({});
		const int allHeight = (rows * (fontMetrics().height() + 2)) +
		                      horizontalHeader()->height() * 2 +
		                      horizontalScrollBar()->height();

		setMinimumHeight(std::min(allHeight, 400)); // NOLINT(readability-magic-numbers)
		verticalHeader()->resetDefaultSectionSize();
	}
}

void HistoryTableView::resizeEvent(QResizeEvent* e)
{
	QTableView::resizeEvent(e);
	resizeColumnToContents(0);
}

void HistoryTableView::showEvent(QShowEvent* e)
{
	Gui::WidgetTemplate<QTableView>::showEvent(e);
	skinChanged();
}

void HistoryTableView::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->contextMenu)
	{
		initContextMenu();
	}

	m->contextMenu->exec(e->globalPos());
}

void HistoryTableView::initContextMenu()
{
	m->contextMenu = new Library::ContextMenu(this);
	m->contextMenu->showActions(Library::ContextMenu::EntryAppend |
	                            Library::ContextMenu::EntryPlayNext |
	                            Library::ContextMenu::EntryPlay |
	                            Library::ContextMenu::EntryPlayNewTab);

	connect(m->contextMenu->action(Library::ContextMenu::EntryAppend), &QAction::triggered,
	        this, &HistoryTableView::appendTriggered);
	connect(m->contextMenu->action(Library::ContextMenu::EntryPlay), &QAction::triggered,
	        this, &HistoryTableView::playTriggered);
	connect(m->contextMenu->action(Library::ContextMenu::EntryPlayNext), &QAction::triggered,
	        this, &HistoryTableView::playNextTriggered);
	connect(m->contextMenu->action(Library::ContextMenu::EntryPlayNewTab), &QAction::triggered,
	        this, &HistoryTableView::playNewTabTriggered);
}

void HistoryTableView::appendTriggered()
{
	const auto tracks = m->model->tracksByIndexes(selectedIndexes());
	m->libraryPlaylistInteractor->append(tracks);
}

void HistoryTableView::playNewTabTriggered()
{
	const auto tracks = m->model->tracksByIndexes(selectedIndexes());
	m->libraryPlaylistInteractor->createPlaylist(tracks, true);
}

void HistoryTableView::playNextTriggered()
{
	const auto tracks = m->model->tracksByIndexes(selectedIndexes());
	m->libraryPlaylistInteractor->insertAfterCurrentTrack(tracks);
}

void HistoryTableView::playTriggered()
{
	const auto tracks = m->model->tracksByIndexes(selectedIndexes());
	m->libraryPlaylistInteractor->createPlaylist(tracks, false);
}
