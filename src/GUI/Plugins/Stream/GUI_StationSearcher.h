#ifndef GUISTATIONSEARCHER_H
#define GUISTATIONSEARCHER_H

#include "GUI/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class QWidget;
class QTableWidgetItem;

UI_FWD(GUI_StationSearcher)

class GUI_StationSearcher :
		public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_StationSearcher)
	UI_CLASS(GUI_StationSearcher)

signals:
	void sig_stream_selected(const QString& name, const QString& url);

public:
	GUI_StationSearcher(QWidget* parent=nullptr);
	~GUI_StationSearcher();

private slots:
	void search_clicked();
	void listen_clicked();
	void close_clicked();

	void search_text_changed(const QString& text);
	void data_available();
	void clear();

	void station_changed(QTableWidgetItem* item);
	void stream_changed(QTableWidgetItem* item);
};

#endif // STATIONSEARCHER_H
