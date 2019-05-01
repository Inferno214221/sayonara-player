#include "GUI_StationSearcher.h"
#include "GUI/Plugins/ui_GUI_StationSearcher.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"
#include "Utils/Utils.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"
#include "GUI/Utils/Style.h"
#include "GUI/Utils/EventFilter.h"

#include <QMenu>

struct GUI_StationSearcher::Private
{
	QList<RadioStation> stations;
	StationSearcher*	searcher;
	StationSearcher::Mode mode;
	QMenu*			context_menu=nullptr;

	Private(GUI_StationSearcher* parent) :
		mode(StationSearcher::NewSearch)
	{
		searcher = new StationSearcher(parent);
	}


	void set_from_to_label(QLabel* label)
	{
		label->setVisible(stations.size() > 5);

		if(stations.size() < 5){
			return;
		}

		label->setText
		(
			tr("Show stations from %1 to %2")
				.arg("<b>" + stations.first().name + "</b>")
				.arg("<b>" + stations.last().name + "</b>" )
		);
	}

	void set_placeholder_text(QLineEdit* le)
	{
		if(mode == StationSearcher::Style)
		{
			le->setPlaceholderText(Lang::get(Lang::SearchVerb) + ": " + Lang::get(Lang::Genre));
		}

		else {
			le->setPlaceholderText(Lang::get(Lang::SearchVerb) + ": " + Lang::get(Lang::RadioStation));
		}
	}

};

GUI_StationSearcher::GUI_StationSearcher(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>(this);

	ui = new Ui::GUI_StationSearcher();
	ui->setupUi(this);

	ui->pb_progress->setVisible(false);
	ui->btn_listen->setEnabled(false);
	ui->btn_search->setEnabled(ui->le_search->text().size() > 0);
	ui->btn_search_next->setVisible(m->searcher->can_search_next());
	ui->btn_search_prev->setVisible(m->searcher->can_search_previous());
	ui->tw_stations->setEnabled(false);
	ui->tw_streams->setEnabled(false);

	ui->splitter->setStretchFactor(0, 3);
	ui->splitter->setStretchFactor(1, 1);

	connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_StationSearcher::search_text_changed);
	connect(ui->le_search, &QLineEdit::returnPressed, this, &GUI_StationSearcher::search_clicked);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_StationSearcher::close);
	connect(ui->btn_listen, &QPushButton::clicked, this, &GUI_StationSearcher::listen_clicked);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_StationSearcher::search_clicked);
	connect(ui->btn_search_next, &QPushButton::clicked, this, &GUI_StationSearcher::search_next_clicked);
	connect(ui->btn_search_prev, &QPushButton::clicked, this, &GUI_StationSearcher::search_prev_clicked);

	connect(ui->tw_stations, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::station_changed);
	connect(ui->tw_streams, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::stream_changed);

	connect(m->searcher, &StationSearcher::sig_stations_found, this, &GUI_StationSearcher::stations_fetched);
}

GUI_StationSearcher::~GUI_StationSearcher() {}

void GUI_StationSearcher::init_line_edit()
{
	ContextMenuFilter* cmf = new ContextMenuFilter(ui->le_search);
	QMenu* menu = new QMenu(ui->le_search);
	m->context_menu = menu;
	connect(cmf, &ContextMenuFilter::sig_context_menu, this, [menu](const QPoint& p, QAction* action)
	{
		Q_UNUSED(action)
		menu->exec(p);
	});

	QAction* action_1 = m->context_menu->addAction(Lang::get(Lang::RadioStation));
	QAction* action_2 = m->context_menu->addAction(Lang::get(Lang::Genre));

	connect(action_1, &QAction::triggered, this, [this](){
		this->change_mode(StationSearcher::Mode::NewSearch);
	});

	connect(action_2, &QAction::triggered, this, [this](){
		this->change_mode(StationSearcher::Mode::Style);
	});

	ui->le_search->installEventFilter(cmf);
}

void GUI_StationSearcher::check_listen_button()
{
	ui->btn_listen->setEnabled(false);

	int cur_station = ui->tw_stations->currentRow();
	if(cur_station < 0 || cur_station >= m->stations.size()){
		return;
	}

	RadioStation station = m->stations[cur_station];
	int cur_stream = ui->tw_streams->currentRow();
	if(cur_stream < 0 || cur_stream >= station.streams.size()){
		return;
	}

	ui->btn_listen->setEnabled(true);
}

void GUI_StationSearcher::clear_stations()
{
	ui->tw_stations->clear();
	while(ui->tw_stations->rowCount() > 0){
		ui->tw_stations->removeRow(0);
	}

	ui->tw_stations->setEnabled(false);
}

void GUI_StationSearcher::clear_streams()
{
	ui->tw_streams->clear();
	while(ui->tw_streams->rowCount() > 0){
		ui->tw_streams->removeRow(0);
	}

	ui->tw_streams->setEnabled(false);
}

void GUI_StationSearcher::change_mode(StationSearcher::Mode mode)
{
	m->mode = mode;
	m->set_placeholder_text(ui->le_search);
}

void GUI_StationSearcher::search_clicked()
{
	QString text = ui->le_search->text();
	if(text.isEmpty()){
		return;
	}

	m->stations.clear();

	clear_stations();
	clear_streams();

	if(m->mode == StationSearcher::Style)
	{
		m->searcher->search_style(text);
	}

	else
	{
		m->searcher->search_station(text);
	}


	ui->pb_progress->setVisible(true);
}

void GUI_StationSearcher::search_prev_clicked()
{
	m->searcher->search_previous();
	ui->pb_progress->setVisible(true);

	ui->tw_stations->setEnabled(false);
	ui->tw_streams->setEnabled(false);
}


void GUI_StationSearcher::search_next_clicked()
{
	m->searcher->search_next();
	ui->pb_progress->setVisible(true);

	ui->tw_stations->setEnabled(false);
	ui->tw_streams->setEnabled(false);
}


void GUI_StationSearcher::stations_fetched()
{
	QList<RadioStation> stations = m->searcher->found_stations();

	ui->pb_progress->setVisible(false);
	ui->tw_stations->setEnabled(true);

	ui->btn_search_next->setVisible(m->searcher->can_search_next());
	ui->btn_search_prev->setVisible(m->searcher->can_search_previous());

	if(stations.isEmpty())
	{
		if( m->searcher->mode() == StationSearcher::NewSearch ||
			m->searcher->mode() == StationSearcher::Style)
		{			
			ui->lab_from_to->setVisible(false);

			clear_stations();
			clear_streams();

			m->stations.clear();
		}

		return;
	}

	clear_stations();

	m->stations = stations;
	m->set_from_to_label(ui->lab_from_to);

	ui->tw_stations->setRowCount(m->stations.size());
	ui->tw_stations->setColumnCount(3);
	ui->tw_stations->setHorizontalHeaderItem(0, new QTableWidgetItem(Lang::get(Lang::Name)));
	ui->tw_stations->setHorizontalHeaderItem(1, new QTableWidgetItem(tr("Location")));
	ui->tw_stations->setHorizontalHeaderItem(2, new QTableWidgetItem(Lang::get(Lang::Info)));

	int row=0;
	for(const RadioStation& station : m->stations)
	{
		QTableWidgetItem* item_name = new QTableWidgetItem(station.name);
		QTableWidgetItem* item_location = new QTableWidgetItem(station.location);
		QTableWidgetItem* item_desc = new QTableWidgetItem(station.description);

		ui->tw_stations->setItem(row, 0, item_name);
		ui->tw_stations->setItem(row, 1, item_location);
		ui->tw_stations->setItem(row, 2, item_desc);

		item_name->setToolTip(station.description);

		row++;
	}

	ui->tw_stations->setEnabled(true);
	ui->tw_stations->resizeColumnToContents(0);
	ui->tw_stations->setColumnWidth(0,
		std::max(ui->tw_stations->columnWidth(0), ui->tw_stations->width() / 3)
	);
}

void GUI_StationSearcher::listen_clicked()
{
	int cur_station_index = ui->tw_stations->currentRow();
	RadioStation station = m->stations.at(cur_station_index);

	int cur_stream_index = ui->tw_streams->currentRow();
	Stream stream = station.streams.at(cur_stream_index);

	emit sig_stream_selected(station.name, stream.url);

	this->close();
}

void GUI_StationSearcher::search_text_changed(const QString& text)
{
	ui->btn_search->setEnabled(text.size() > 0);
	ui->btn_search_next->setVisible(false);
	ui->btn_search_prev->setVisible(false);

	if(text.startsWith("s:") || text.startsWith("n:"))
	{
		change_mode(StationSearcher::NewSearch);
		ui->le_search->clear();
	}

	else if(text.startsWith("g:"))
	{
		change_mode(StationSearcher::Style);
		ui->le_search->clear();
	}
}


void GUI_StationSearcher::station_changed()
{
	ui->btn_listen->setEnabled(false);

	int cur_row = ui->tw_stations->currentRow();
	if(cur_row < 0 || cur_row >= m->stations.count()){
		return;
	}

	RadioStation station = m->stations[cur_row];

	clear_streams();
	check_listen_button();

	ui->tw_streams->setEnabled(true);
	ui->tw_streams->setRowCount(station.streams.size());
	ui->tw_streams->setColumnCount(3);
	ui->tw_streams->setHorizontalHeaderItem(0, new QTableWidgetItem(tr("Type")));
	ui->tw_streams->setHorizontalHeaderItem(1, new QTableWidgetItem(Lang::get(Lang::Bitrate)));
	ui->tw_streams->setHorizontalHeaderItem(2, new QTableWidgetItem(tr("Url")));

	int row = 0;
	for(const Stream& stream : station.streams)
	{
		QTableWidgetItem* item_type = new QTableWidgetItem(stream.type);
		QTableWidgetItem* item_bitrate = new QTableWidgetItem(stream.bitrate);
		QTableWidgetItem* item_url = new QTableWidgetItem(stream.url);

		ui->tw_streams->setItem(row, 0, item_type);
		ui->tw_streams->setItem(row, 1, item_bitrate);
		ui->tw_streams->setItem(row, 2, item_url);

		row++;
	}

	ui->tw_streams->setCurrentItem(ui->tw_streams->item(0, 0));
	stream_changed();
}

void GUI_StationSearcher::stream_changed()
{
	check_listen_button();
}


void GUI_StationSearcher::showEvent(QShowEvent* e)
{
	ui->le_search->setFocus();
	Gui::Dialog::showEvent(e);

	QSize sz = GetSetting(Set::Stream_SearchWindowSize);
	if(!sz.isEmpty())
	{
		this->resize(sz);
	}

	ui->le_search->setFocus();
	ui->le_search->setFocus();
}

void GUI_StationSearcher::closeEvent(QCloseEvent* e)
{
	SetSetting(Set::Stream_SearchWindowSize, this->size());

	Gui::Dialog::closeEvent(e);
}

void GUI_StationSearcher::language_changed()
{
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_search_next->setText(Lang::get(Lang::NextPage));
	ui->btn_search_prev->setText(Lang::get(Lang::PreviousPage));
	ui->btn_listen->setText(Lang::get(Lang::Add));
	ui->btn_close->setText(Lang::get(Lang::Close));

	QString tooltip = QString("<b>%1</b><br />s:, n: %2<br />g: %3")
		.arg(Lang::get(Lang::SearchNoun))
		.arg(Lang::get(Lang::RadioStation))
		.arg(Lang::get(Lang::Genre));

	ui->le_search->setToolTip(tooltip);

	m->set_placeholder_text(ui->le_search);
	m->set_from_to_label(ui->lab_from_to);
	ui->label->setText(Lang::get(Lang::SearchNoun) + ": " + Lang::get(Lang::RadioStation));
}


void GUI_StationSearcher::skin_changed()
{
	QFontMetrics fm = this->fontMetrics();

	ui->tw_stations->horizontalHeader()->setMinimumHeight(std::max(fm.height() + 10, 20));
	ui->lab_link->setText(Util::create_link("fmstream.org", Style::is_dark(), "http://fmstream.org"));
}
