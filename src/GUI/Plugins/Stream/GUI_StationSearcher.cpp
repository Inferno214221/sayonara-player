#include "GUI_StationSearcher.h"
#include "GUI/Plugins/ui_GUI_StationSearcher.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"

struct GUI_StationSearcher::Private
{
	QList<RadioStation> stations;
	StationSearcher*	searcher;
	StationSearcher::Mode mode;

	Private(GUI_StationSearcher* parent) :
		mode(StationSearcher::NewSearch)
	{
		searcher = new StationSearcher(parent);
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
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_StationSearcher::close_clicked);
	connect(ui->btn_listen, &QPushButton::clicked, this, &GUI_StationSearcher::listen_clicked);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_StationSearcher::search_clicked);
	connect(ui->btn_search_next, &QPushButton::clicked, this, &GUI_StationSearcher::search_next_clicked);
	connect(ui->btn_search_prev, &QPushButton::clicked, this, &GUI_StationSearcher::search_prev_clicked);

	connect(ui->tw_stations, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::station_changed);
	connect(ui->tw_streams, &QTableWidget::itemSelectionChanged, this, &GUI_StationSearcher::stream_changed);

	connect(m->searcher, &StationSearcher::sig_stations_found, this, &GUI_StationSearcher::data_available);
}

GUI_StationSearcher::~GUI_StationSearcher() {}

void GUI_StationSearcher::search_clicked()
{
	QString text = ui->le_search->text();
	if(text.isEmpty()){
		return;
	}

	m->stations.clear();
	ui->tw_stations->clear();
	ui->tw_stations->setEnabled(false);

	ui->tw_streams->clear();
	ui->tw_streams->setEnabled(false);

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


void GUI_StationSearcher::data_available()
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
			m->stations = stations;

			ui->lab_from_to->setVisible(false);
			ui->tw_stations->setEnabled(false);
			ui->tw_stations->clear();
			ui->tw_streams->clear();
		}

		return;
	}

	m->stations = stations;

	ui->lab_from_to->setVisible(stations.size() > 5);
	ui->lab_from_to->setText(
		QString("Show stations from <b>%1</b> to <b>%2</b>").arg(stations.first().name).arg(stations.last().name)
	);

	ui->tw_stations->clear();
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

	ui->tw_stations->resizeColumnToContents(0);
	ui->tw_stations->setColumnWidth(0,
		std::max(ui->tw_stations->columnWidth(0), ui->tw_stations->width() / 3)
	);
}

void GUI_StationSearcher::listen_clicked()
{
	RadioStation station = m->stations.at(ui->tw_stations->currentRow());
	Stream stream = station.streams.at(ui->tw_streams->currentRow());

	emit sig_stream_selected(station.name, stream.url);

	close_clicked();
}

void GUI_StationSearcher::close_clicked()
{
	this->close();
}

void GUI_StationSearcher::search_text_changed(const QString& text)
{
	ui->btn_search->setEnabled(text.size() > 0);
	ui->btn_search_next->setVisible(false);
	ui->btn_search_prev->setVisible(false);


	if(text.startsWith("s:") || text.startsWith("n:"))
	{
		m->mode = StationSearcher::Style;
		ui->le_search->clear();
		ui->le_search->setPlaceholderText(Lang::get(Lang::SearchVerb));
	}

	else if(text.startsWith("g:"))
	{
		m->mode = StationSearcher::NewSearch;
		ui->le_search->clear();
		ui->le_search->setPlaceholderText(Lang::get(Lang::SearchVerb) + ": " + Lang::get(Lang::Genre));
	}
}

void GUI_StationSearcher::clear()
{
	ui->tw_stations->clear();
	ui->tw_streams->clear();
}

void GUI_StationSearcher::station_changed()
{
	int cur_row = ui->tw_stations->currentRow();
	if(cur_row < 0 || cur_row >= m->stations.count()){
		return;
	}

	ui->tw_streams->setEnabled(true);

	RadioStation station = m->stations[cur_row];

	ui->tw_streams->clear();
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
}

void GUI_StationSearcher::stream_changed()
{
	this->ui->btn_listen->setEnabled(ui->tw_streams->currentRow() >= 0);
}


void GUI_StationSearcher::showEvent(QShowEvent* e)
{
	ui->le_search->setFocus();
	Gui::Dialog::showEvent(e);

	QSize sz = _settings->get<Set::Stream_SearchWindowSize>();
	if(!sz.isEmpty())
	{
		this->resize(sz);
	}

	ui->le_search->setFocus();
	ui->le_search->setFocus();
}

void GUI_StationSearcher::closeEvent(QCloseEvent* e)
{
	_settings->set<Set::Stream_SearchWindowSize>(this->size());

	Gui::Dialog::closeEvent(e);
}

void GUI_StationSearcher::language_changed()
{
	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_search_next->setText(Lang::get(Lang::NextPage));
	ui->btn_search_prev->setText(Lang::get(Lang::PreviousPage));
	ui->btn_listen->setText(Lang::get(Lang::Listen));
	ui->btn_close->setText(Lang::get(Lang::Close));

	if(m->stations.size() > 0)
	{
		ui->lab_from_to->setText(
			QString("Show stations from <b>%1</b> to <b>%2</b>").arg(m->stations.first().name).arg(m->stations.last().name)
		);
	}
}

void GUI_StationSearcher::skin_changed()
{
	QFontMetrics fm(this->font());

	int height = std::max(fm.height() + 10, 20);
	ui->tw_stations->horizontalHeader()->setMinimumHeight(height);
}
