#include "HistoryEntryWidget.h"
#include "HistoryTableView.h"
#include "Components/Session/Session.h"
#include "Utils/Utils.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"

#include <QVBoxLayout>
#include <QLabel>

struct HistoryEntryWidget::Private
{
	Session::Timecode timecode;

	HistoryTableView*	tableView=nullptr;
	QLabel*				trackLabel=nullptr;
	QLabel*				dateLabel=nullptr;

	Private(Session::Timecode timecode) :
		timecode(timecode)
	{}
};

static QString dateToString(const QDateTime& date)
{
	QLocale locale = Util::Language::getCurrentLocale();
	QString str = locale.toString(date.date());
	return str;
}

HistoryEntryWidget::HistoryEntryWidget(Session::Manager* sessionManager, Session::Timecode timecode, QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>(timecode);

	auto* layout = new QVBoxLayout();
	this->setLayout(layout);

	m->tableView = new HistoryTableView(sessionManager, timecode, this);

	auto* labelLayout = new QHBoxLayout();
	{
		m->dateLabel = new QLabel(this);
		{
			QFont font = m->dateLabel->font();
			font.setBold(true);
			m->dateLabel->setFont(font);
			m->dateLabel->setText( dateToString(Util::intToDate(timecode)) );
		}

		m->trackLabel = new QLabel(this);
		{
			QFont font = m->trackLabel->font();
			font.setBold(true);
			m->trackLabel->setFont(font);
			m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableView->rows()));
		}

		labelLayout->addWidget(m->dateLabel);
		labelLayout->addItem(new QSpacerItem(100, 1, QSizePolicy::MinimumExpanding, QSizePolicy::Maximum));
		labelLayout->addWidget(m->trackLabel);
	}

	layout->setSpacing(10);
	layout->addLayout(labelLayout);
	layout->addWidget(m->tableView);

	connect(m->tableView, &HistoryTableView::sigRowcountChanged, this, &HistoryEntryWidget::rowcount_changed);
}

Session::Id HistoryEntryWidget::id() const
{
	return m->timecode;
}

HistoryEntryWidget::~HistoryEntryWidget() = default;

void HistoryEntryWidget::languageChanged()
{
	m->dateLabel->setText( dateToString(Util::intToDate(m->timecode)) );
	m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableView->rows()));
}

void HistoryEntryWidget::rowcount_changed()
{
	m->trackLabel->setText(Lang::getWithNumber(Lang::NrTracks, m->tableView->rows()));
}
