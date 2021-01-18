#include "HistoryTableView.h"
#include "HistoryEntryModel.h"

#include "Utils/Language/Language.h"

#include <QHeaderView>
#include <QStringListModel>
#include <QScrollBar>
#include <QDrag>

using Parent = Gui::WidgetTemplate<QTableView>;

struct HistoryTableView::Private
{
	HistoryEntryModel* model = nullptr;
};

HistoryTableView::HistoryTableView(Session::Timecode timecode, QWidget* parent) :
	Gui::WidgetTemplate<QTableView>(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>();

	m->model = new HistoryEntryModel(timecode, nullptr);
	this->setModel(m->model);

	this->setAlternatingRowColors(true);
	this->setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	this->verticalHeader()->setVisible(false);

	QHeaderView* horizontalHeader = this->horizontalHeader();
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

void HistoryTableView::languageChanged() {}

void HistoryTableView::skinChanged()
{
	const int rows = m->model->rowCount(QModelIndex());
	const int allHeight = (rows * (this->fontMetrics().height() + 2)) +
	                      horizontalHeader()->height() * 2 +
	                      horizontalScrollBar()->height();

	this->setMinimumHeight(std::min(allHeight, 400));

	this->verticalHeader()->resetDefaultSectionSize();
}

void HistoryTableView::resizeEvent(QResizeEvent* e)
{
	QTableView::resizeEvent(e);
	this->resizeColumnToContents(0);
}
