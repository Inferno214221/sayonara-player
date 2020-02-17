#include "HistoryEntryWidget.h"
#include "HistoryTableView.h"
#include "Components/Session/Session.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"

#include <QVBoxLayout>
#include <QLabel>

struct HistoryEntryWidget::Private
{
	Session::Timecode timecode;

	HistoryTableView*	tableview=nullptr;
	QLabel*				trackLabel=nullptr;
	QLabel*				dateLabel=nullptr;

	Private(Session::Timecode timecode) :
		timecode(timecode)
	{}
};

HistoryEntryWidget::HistoryEntryWidget(Session::Timecode timecode, QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>(timecode);

	auto* layout = new QVBoxLayout();
	this->setLayout(layout);

	m->tableview = new HistoryTableView(timecode, this);

	auto* label_layout = new QHBoxLayout();
	{
		m->dateLabel = new QLabel(this);
		{
			QFont font = m->dateLabel->font();
			font.setBold(true);
			m->dateLabel->setFont(font);
			m->dateLabel->setText(Util::intToDate(timecode).date().toString());
		}

		m->trackLabel = new QLabel(this);
		{
			QFont font = m->trackLabel->font();
			font.setBold(true);
			m->trackLabel->setFont(font);
			m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableview->rows()));
		}

		label_layout->addWidget(m->dateLabel);
		label_layout->addItem(new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
		label_layout->addWidget(m->trackLabel);
	}

	layout->setSpacing(10);
	layout->addLayout(label_layout);
	layout->addWidget(m->tableview);

	connect(m->tableview, &HistoryTableView::sigRowcountChanged, this, &HistoryEntryWidget::rowcount_changed);
}

Session::Id HistoryEntryWidget::id() const
{
	return m->timecode;
}

HistoryEntryWidget::~HistoryEntryWidget() = default;

void HistoryEntryWidget::languageChanged()
{
	m->dateLabel->setText(Util::intToDate(m->timecode).date().toString());
	m->trackLabel->setText(tr("%n track(s)", "", m->tableview->rows()));
}

void HistoryEntryWidget::rowcount_changed()
{
	m->trackLabel->setText(tr("%n track(s)", "", m->tableview->rows()));
}
