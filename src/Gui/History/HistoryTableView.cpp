#include "HistoryTableView.h"
#include "HistoryEntryModel.h"

#include "Utils/Language/Language.h"

#include <QHeaderView>
#include <QStringListModel>
#include <QScrollBar>
#include <QDrag>

using Parent=Gui::WidgetTemplate<QTableView>;

struct HistoryTableView::Private
{
	HistoryEntryModel* model=nullptr;
};

HistoryTableView::HistoryTableView(Session::Timecode timecode, QWidget* parent) :
	Gui::WidgetTemplate<QTableView>(parent),
	Gui::Dragable(this)
{
	m = Pimpl::make<Private>();

	m->model = new HistoryEntryModel(timecode, nullptr);
	this->setModel(m->model);

	this->setAlternatingRowColors(true);
	this->verticalHeader()->setVisible(false);
	this->horizontalHeader()->setStretchLastSection(true);
	this->setHorizontalScrollMode(QTableView::ScrollMode::ScrollPerPixel);
	this->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	this->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

	skin_changed();

	connect(m->model, &HistoryEntryModel::sig_rows_added, this, &HistoryTableView::rowcount_changed);
}

int HistoryTableView::rows() const
{
	return model()->rowCount();
}

HistoryTableView::~HistoryTableView() = default;

void HistoryTableView::resizeEvent(QResizeEvent* e)
{
	QTableView::resizeEvent(e);

	this->resizeColumnToContents(0);
	int w = this->columnWidth(0);

	this->setColumnWidth(1, (this->width() - w) / 3);
	this->setColumnWidth(2, (this->width() - w) / 3);
	this->setColumnWidth(3, (this->width() - w) / 3);
}

void HistoryTableView::rowcount_changed()
{
	skin_changed();

	emit sig_rowcount_changed();
}

void HistoryTableView::language_changed() {}

void HistoryTableView::skin_changed()
{
	int all_height = (m->model->rowCount() * (this->fontMetrics().height() + 2)) +
		horizontalHeader()->height() * 2 +
		horizontalScrollBar()->height();

	this->setMinimumHeight(std::min(all_height, 400));
}

QMimeData* HistoryTableView::dragable_mimedata() const
{
	return m->model->mimeData(this->selectionModel()->selectedIndexes());
}
