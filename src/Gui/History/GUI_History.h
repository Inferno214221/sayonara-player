#ifndef GUI_HISTORY_H
#define GUI_HISTORY_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Pimpl.h"

class QFrame;

UI_FWD(GUI_History)

class GUI_History :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_History)
	UI_CLASS(GUI_History)

private:
	QWidget* add_new_page();
	bool change_page(int index);

public:
	explicit GUI_History(QWidget* parent=nullptr);
	~GUI_History();

	QFrame* header() const;

private slots:
	void older_clicked();
	void newer_clicked();
};

#endif // GUI_HISTORY_H
