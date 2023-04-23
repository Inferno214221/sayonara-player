#include "HistoryTableView.h"
#include "HistoryEntryModel.h"

#include "Gui/Utils/Widgets/HeaderView.h"

#include <QStringListModel>
#include <QScrollBar>
#include <QDrag>

using Parent = Gui::WidgetTemplate<QTableView>;

struct HistoryTableView::Private
{
	HistoryEntryModel* model = nullptr;

	Private(Session::Manager* sessionManager, Session::Timecode timecode) :
		model(new HistoryEntryModel(sessionManager, timecode))
	{}
};

HistoryTableView::HistoryTableView(Session::Manager* sessionManager, Session::Timecode timecode, QWidget* parent) :
	Gui::WidgetTemplate<QTableView>(parent),
	Gui::Dragable(this)
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
