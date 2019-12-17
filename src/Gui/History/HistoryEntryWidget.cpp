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
	QLabel*				track_label=nullptr;
	QLabel*				date_label=nullptr;

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
		m->date_label = new QLabel(this);
		{
			QFont font = m->date_label->font();
			font.setBold(true);
			m->date_label->setFont(font);
			m->date_label->setText(Util::int_to_date(timecode).date().toString());
		}

		m->track_label = new QLabel(this);
		{
			QFont font = m->track_label->font();
			font.setBold(true);
			m->track_label->setFont(font);
			m->track_label->setText(Lang::get_with_number(Lang::NrTracks, m->tableview->rows()));
		}

		label_layout->addWidget(m->date_label);
		label_layout->addItem(new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
		label_layout->addWidget(m->track_label);
	}

	layout->setSpacing(10);
	layout->addLayout(label_layout);
	layout->addWidget(m->tableview);

	connect(m->tableview, &HistoryTableView::sig_rowcount_changed, this, &HistoryEntryWidget::rowcount_changed);
}

Session::Id HistoryEntryWidget::id() const
{
	return m->timecode;
}

HistoryEntryWidget::~HistoryEntryWidget() = default;

void HistoryEntryWidget::language_changed()
{
	m->date_label->setText(Util::int_to_date(m->timecode).date().toString());
	m->track_label->setText(tr("%n track(s)", "", m->tableview->rows()));
}

void HistoryEntryWidget::rowcount_changed()
{
	m->track_label->setText(tr("%n track(s)", "", m->tableview->rows()));
}
