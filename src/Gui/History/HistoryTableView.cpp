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
	m = Pimpl::make<Private>(sessionManager, timecode);

	this->setModel(m->model);
	this->setAlternatingRowColors(true);
	this->setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	this->verticalHeader()->setVisible(false);
	this->setShowGrid(false);

	auto horizontalHeader = new Gui::HeaderView(Qt::Orientation::Horizontal, this);
	this->setHorizontalHeader(horizontalHeader);
	horizontalHeader->setStretchLastSection(true);
	horizontalHeader->setSectionResizeMode(0, QHeaderView::Interactive);
	horizontalHeader->setSectionResizeMode(1, QHeaderView::Stretch);
	horizontalHeader->setSectionResizeMode(2, QHeaderView::Stretch);
	horizontalHeader->setSectionResizeMode(3, QHeaderView::Stretch);

	connect(m->model, &HistoryEntryModel::sigRowsAdded, this, &HistoryTableView::rowcountChanged);
}

HistoryTableView::~HistoryTableView() = default;

int HistoryTableView::rows() const
{
	return model()->rowCount();
}

void HistoryTableView::rowcountChanged()
{
	skinChanged();
	emit sigRowcountChanged();
}

void HistoryTableView::skinChanged()
{
	if(isVisible())
	{
		const int rows = m->model->rowCount(QModelIndex());
		const int allHeight = (rows * (this->fontMetrics().height() + 2)) +
		                      horizontalHeader()->height() * 2 +
		                      horizontalScrollBar()->height();

		this->setMinimumHeight(std::min(allHeight, 400));

		this->verticalHeader()->resetDefaultSectionSize();
	}
}

void HistoryTableView::resizeEvent(QResizeEvent* e)
{
	QTableView::resizeEvent(e);
	this->resizeColumnToContents(0);
}

void HistoryTableView::showEvent(QShowEvent* e)
{
	Gui::WidgetTemplate<QTableView>::showEvent(e);
	skinChanged();
}
