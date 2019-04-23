#include "GUI_StationSearcher.h"
#include "GUI/Plugins/ui_GUI_StationSearcher.h"
#include "Components/Streaming/StationSearcher/StationSearcher.h"
#include "Utils/Language.h"

struct GUI_StationSearcher::Private
{
	QList<RadioStation> stations;
};

GUI_StationSearcher::GUI_StationSearcher(QWidget* parent) :
	Gui::Dialog(parent)
{
	m = Pimpl::make<Private>();

	ui = new Ui::GUI_StationSearcher();
	ui->setupUi(this);

	ui->pb_progress->setVisible(false);
	ui->btn_listen->setEnabled(false);
	ui->btn_search->setEnabled(ui->le_search->text().size() > 0);
	ui->tw_stations->setEnabled(false);
	ui->tw_streams->setEnabled(false);

	ui->splitter->setStretchFactor(0, 3);
	ui->splitter->setStretchFactor(1, 1);

	connect(ui->le_search, &QLineEdit::textChanged, this, &GUI_StationSearcher::search_text_changed);
	connect(ui->btn_close, &QPushButton::clicked, this, &GUI_StationSearcher::close_clicked);
	connect(ui->btn_listen, &QPushButton::clicked, this, &GUI_StationSearcher::listen_clicked);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_StationSearcher::search_clicked);

	connect(ui->tw_stations, &QTableWidget::itemClicked, this, &GUI_StationSearcher::station_changed);
	connect(ui->tw_streams, &QTableWidget::itemClicked, this, &GUI_StationSearcher::stream_changed);
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

	StationSearcher* searcher = new StationSearcher(this);

	connect(searcher, &StationSearcher::sig_stations_found, this, &GUI_StationSearcher::data_available);

	searcher->search_station(text);

	ui->pb_progress->setVisible(true);

}


void GUI_StationSearcher::data_available()
{
	ui->pb_progress->setVisible(false);

	StationSearcher* searcher = static_cast<StationSearcher*>(sender());
	m->stations = searcher->found_stations();

	ui->tw_stations->setEnabled(m->stations.size() > 0);
	if(m->stations.isEmpty()){
		return;
	}

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
}

void GUI_StationSearcher::clear()
{
	ui->tw_stations->clear();
	ui->tw_streams->clear();
}

void GUI_StationSearcher::station_changed(QTableWidgetItem* item)
{
	ui->tw_streams->setEnabled(true);
	RadioStation station = m->stations[item->row()];

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

void GUI_StationSearcher::stream_changed(QTableWidgetItem* item)
{
	Q_UNUSED(item)

	this->ui->btn_listen->setEnabled(ui->tw_streams->currentRow() >= 0);
}
