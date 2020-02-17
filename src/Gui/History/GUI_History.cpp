#include "GUI_History.h"
#include "HistoryEntryWidget.h"

#include "Gui/History/ui_GUI_History.h"

#include "Components/Session/Session.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/Language/Language.h"

#include <QScrollArea>

using Session::Timecode;

struct GUI_History::Private
{
	Session::Manager* session=nullptr;
	int lastPage;

	Private() :
		lastPage(10000)
	{
		session = Session::Manager::instance();
	}
};

GUI_History::GUI_History(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_History();
	ui->setupUi(this);

	changePage(0);

	connect(ui->btn_older, &QPushButton::clicked, this, &GUI_History::olderClicked);
	connect(ui->btn_newer, &QPushButton::clicked, this, &GUI_History::newerClicked);
}

GUI_History::~GUI_History()
{
	delete ui;
}

QFrame* GUI_History::header() const
{
	return ui->header;
}

void GUI_History::olderClicked()
{
	bool b = changePage(ui->stackedWidget->currentIndex() + 1);
	if(!b){
		ui->btn_older->setEnabled(false);
	}
}

void GUI_History::newerClicked()
{
	changePage(ui->stackedWidget->currentIndex() - 1);
}

QWidget* GUI_History::addNewPage()
{
	auto* page = new QWidget();
	auto* page_layout = new QVBoxLayout(page);
	page->setLayout(page_layout);

	auto* scrollarea = new QScrollArea(page);
	auto* scrollarea_content = new QWidget();
	auto* scrollarea_content_layout = new QVBoxLayout(scrollarea_content);
	scrollarea_content->setLayout(scrollarea_content_layout);
	scrollarea->setWidget(scrollarea_content);
	scrollarea->setWidgetResizable(true);

	page_layout->addWidget(scrollarea);

	ui->stackedWidget->addWidget(page);

	return scrollarea_content;
}

bool GUI_History::changePage(int index)
{
	if(index >= ui->stackedWidget->count())
	{
		const Session::EntryListMap history = m->session->historyEntries(index * 5, 5);
		if(history.isEmpty()){
			return false;
		}

		QWidget* page = addNewPage();

		QList<Timecode> session_ids = history.keys();

		 // insert today if neccessary
		const Timecode now = Session::now();
		const Timecode today_begin = Session::dayBegin(now);
		const Timecode today_end = Session::dayEnd(now);
		bool contains_today = Util::Algorithm::contains(session_ids, [today_begin, today_end](auto tc){
			return (tc >= today_begin && tc <= today_end);
		});

		if(!contains_today && (index == 0)) {
			session_ids << now;
		}

		Util::Algorithm::sort(session_ids, [](auto id1, auto id2){
			return (id1 > id2);
		});

		Util::Set<Timecode> timecodes;

		for(Timecode timecode : session_ids)
		{
			const Timecode day_begin = Session::dayBegin(timecode);
			if(timecodes.contains(day_begin)){
				continue;
			}

			timecodes << day_begin;

			auto* history_widget = new HistoryEntryWidget(day_begin, page);

			page->layout()->addWidget(history_widget);
		}

		if(timecodes.count() < 5){
			m->lastPage = index;
		}
	}

	else if(index < 0)
	{
		index = 0;
	}

	ui->btn_newer->setEnabled(index > 0);
	ui->btn_older->setEnabled(index < m->lastPage);

	ui->stackedWidget->setCurrentIndex(index);

	return true;
}

