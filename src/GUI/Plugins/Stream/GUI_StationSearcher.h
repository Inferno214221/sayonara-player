#ifndef GUISTATIONSEARCHER_H
#define GUISTATIONSEARCHER_H

#include "GUI/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

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

private:
	void check_listen_button();
	void clear_stations();
	void clear_streams();


private slots:
	void search_clicked();
	void search_prev_clicked();
	void search_next_clicked();
	void listen_clicked();

	void search_text_changed(const QString& text);
	void stations_fetched();

	void station_changed();
	void stream_changed();

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

	void language_changed() override;
	void skin_changed() override;
};


#endif // STATIONSEARCHER_H
