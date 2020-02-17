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

public:
	explicit GUI_History(QWidget* parent=nullptr);
	~GUI_History();

	QFrame* header() const;


private:
	QWidget* addNewPage();
	bool changePage(int index);


private slots:
	void olderClicked();
	void newerClicked();
};

#endif // GUI_HISTORY_H
