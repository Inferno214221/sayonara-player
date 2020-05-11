#include "GUI_History.h"
#include "HistoryEntryWidget.h"
#include "DoubleCalendarDialog.h"

#include "Gui/History/ui_GUI_History.h"
#include "Gui/Utils/Widgets/CalendarWidget.h"
#include "Gui/Utils/Style.h"

#include "Components/Session/Session.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"

#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"
#include "Utils/Language/LanguageUtils.h"

#include <QDate>
#include <QScrollArea>
#include <QPushButton>
#include <QApplication>
#include <QShortcut>

using Session::Timecode;

struct GUI_History::Private
{
	QWidget* dateRangeWidget=nullptr;
	QPushButton* btnLoadMore=nullptr;
	QDate startDate, endDate;

	QAction* actionGoToBottom=nullptr;
	QAction* actionGoToTop=nullptr;
	QAction* actionSelecteDataRange=nullptr;

	Session::Manager* session=nullptr;
	int lastPage;

	Private() :
		lastPage(-1)
	{
		session = Session::Manager::instance();

		actionGoToTop = new QAction();
		actionGoToBottom = new QAction();
		actionSelecteDataRange = new QAction();

		btnLoadMore = new QPushButton();
		btnLoadMore->setText(tr("Load more entries"));
	}
};

GUI_History::GUI_History(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_History();
	ui->setupUi(this);
	ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout());

	ui->toolButton->registerAction(m->actionGoToBottom);
	ui->toolButton->registerAction(m->actionGoToTop);
	ui->toolButton->registerAction(m->actionSelecteDataRange);

	ui->stackedWidget->setCurrentIndex(0);

	requestData(0);

	connect(m->btnLoadMore, &QPushButton::clicked, this, &GUI_History::loadMore);
	connect(m->actionGoToTop, &QAction::triggered, this, &GUI_History::scrollToTop);
	connect(m->actionGoToBottom, &QAction::triggered, this, &GUI_History::scrollToBottom);
	connect(m->actionSelecteDataRange, &QAction::triggered, this, &GUI_History::dateRangeClicked);
	connect(ui->btnClear, &QPushButton::clicked, this, &GUI_History::clearRangeClicked);

	initShortcuts();
	languageChanged();
}

GUI_History::~GUI_History()
{
	delete ui;
}

QFrame* GUI_History::header() const
{
	return ui->header;
}

void GUI_History::initShortcuts()
{
	m->actionGoToTop->setShortcut(QKeySequence(Qt::Key_Home));
	m->actionGoToBottom->setShortcut(QKeySequence(Qt::Key_End));
	m->actionSelecteDataRange->setShortcut(QKeySequence("Ctrl+r"));

#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
	m->actionGoToTop->setShortcutVisibleInContextMenu(true);
	m->actionGoToBottom->setShortcutVisibleInContextMenu(true);
	m->actionSelecteDataRange->setShortcutVisibleInContextMenu(true);
#endif

	{
		auto* sc = new QShortcut(this);
		sc->setKey(QKeySequence(Qt::Key_Home));
		sc->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
		connect(sc, &QShortcut::activated, this, &GUI_History::scrollToTop);
	}

	{
		auto* sc = new QShortcut(this);
		sc->setKey(QKeySequence(Qt::Key_End));
		sc->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
		connect(sc, &QShortcut::activated, this, &GUI_History::scrollToBottom);
	}

	{
		auto* sc = new QShortcut(this);
		sc->setKey(QKeySequence("Ctrl+r"));
		sc->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
		connect(sc, &QShortcut::activated, this, &GUI_History::dateRangeClicked);
	}

	{
		auto* sc = new QShortcut(this);
		sc->setKey(QKeySequence("Ctrl+r"));
		sc->setContext(Qt::ShortcutContext::WidgetWithChildrenShortcut);
		connect(sc, &QShortcut::activated, this, &GUI_History::dateRangeClicked);
	}
}

void GUI_History::scrollToTop()
{
	const auto widgets = {ui->scrollArea, ui->scrollAreaRange};
	for(auto widget : widgets)
	{
		if(widget->isVisible())
		{
			widget->ensureVisible(0, 0, 0, 0);
		}
	}
}

void GUI_History::scrollToBottom()
{
	const auto widgets = {ui->scrollArea, ui->scrollAreaRange};
	for(auto widget : widgets)
	{
		if(widget->isVisible())
		{
			widget->ensureVisible(0, widget->widget()->height(), 0, 0);
		}
	}

	m->btnLoadMore->setFocus();
}

void GUI_History::loadMore()
{
	requestData(m->lastPage + 1);
}

void GUI_History::dateRangeClicked()
{
	m->startDate = QDate();
	m->endDate = QDate();

	auto* calendarWidget = new Gui::DoubleCalendarDialog(this);
	connect(calendarWidget, &Gui::DoubleCalendarDialog::sigAccepted, this, &GUI_History::calendarFinished);
	connect(calendarWidget, &Gui::DoubleCalendarDialog::sigRejected, calendarWidget, &QObject::deleteLater);

	calendarWidget->resize(800, 480);
	calendarWidget->show();
}

void GUI_History::calendarFinished()
{
	auto* calendarWidget = static_cast<Gui::DoubleCalendarDialog*>(sender());

	m->startDate = calendarWidget->startDate();
	m->endDate = calendarWidget->endDate();

	calendarWidget->deleteLater();

	loadSelectedDateRange();
}

void GUI_History::languageChanged()
{
	m->actionGoToTop->setText(tr("Scroll to top"));
	m->actionGoToBottom->setText(tr("Scroll to bottom"));
	m->actionSelecteDataRange->setText(tr("Select date range") + "...");
}

void GUI_History::clearRangeClicked()
{
	ui->stackedWidget->setCurrentIndex(0);
}

void GUI_History::requestData(int index)
{
	const Session::EntryListMap history = m->session->historyEntries(index, 10);
	QWidget* page = createEntryListWidget(history);

	int oldHeight = ui->scrollArea->widget()->height();

	ui->scrollAreaWidgetContents->layout()->removeWidget(m->btnLoadMore);
	ui->scrollAreaWidgetContents->layout()->addWidget(page);
	ui->scrollAreaWidgetContents->layout()->addWidget(m->btnLoadMore);

	m->lastPage = index;

	if(index > 0)
	{
		ui->scrollArea->widget()->resize(ui->scrollArea->widget()->sizeHint());
		QApplication::processEvents();

		int viewportHeight = ui->scrollArea->viewport()->height();
		ui->scrollArea->ensureVisible(0, oldHeight + viewportHeight - m->btnLoadMore->height(), 0, 0);
	}
}

void GUI_History::loadSelectedDateRange()
{
	if(!ui->scrollAreaRangeContents->layout())
	{
		auto* layout = new QVBoxLayout();
		layout->setContentsMargins(0,0,0,0);
		ui->scrollAreaRangeContents->setLayout(layout);
	}

	else {
		auto* layout = ui->scrollAreaRangeContents->layout();
		layout->removeWidget(m->dateRangeWidget);
		m->dateRangeWidget->deleteLater();
	}

	const QLocale locale = Util::Language::getCurrentLocale();
	ui->labFrom->setText(locale.toString(m->startDate));
	ui->labUntil->setText(locale.toString(m->endDate));

	const QDateTime start(m->startDate, QTime(0,0));
	const QDateTime end(m->endDate, QTime(23, 59));

	const Session::EntryListMap history = m->session->history(start, end);

	m->dateRangeWidget = createEntryListWidget(history);
	ui->scrollAreaRangeContents->layout()->addWidget(m->dateRangeWidget);

	ui->stackedWidget->setCurrentIndex(1);
}

QWidget* GUI_History::createEntryListWidget(const Session::EntryListMap& history)
{
	auto* page = new QWidget();
	auto* pageLayout = new QVBoxLayout(page);
	pageLayout->setContentsMargins(0, 0, 0, 0);
	page->setLayout(pageLayout);

	QList<Timecode> sessionIds = history.keys();
	Util::Algorithm::sort(sessionIds, [](auto id1, auto id2){
		return (id1 > id2);
	});

	Util::Set<Timecode> timecodes;
	for(Timecode timecode : sessionIds)
	{
		const Timecode dayBegin = Session::dayBegin(timecode);
		if(!timecodes.contains(dayBegin))
		{
			timecodes << dayBegin;
			auto* historyWidget = new HistoryEntryWidget(dayBegin, page);
			page->layout()->addWidget(historyWidget);
		}
	}

	return page;
}
