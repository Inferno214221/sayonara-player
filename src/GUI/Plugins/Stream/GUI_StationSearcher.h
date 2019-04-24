#ifndef GUISTATIONSEARCHER_H
#define GUISTATIONSEARCHER_H

#include "GUI/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class QWidget;

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
	void search_prev_clicked();
	void search_next_clicked();
	void listen_clicked();
	void close_clicked();

	void search_text_changed(const QString& text);
	void data_available();
	void clear();

	void station_changed();
	void stream_changed();

protected:
	void showEvent(QShowEvent* e) override;
	void closeEvent(QCloseEvent* e) override;

	void language_changed() override;
	void skin_changed() override;
};


#endif // STATIONSEARCHER_H
