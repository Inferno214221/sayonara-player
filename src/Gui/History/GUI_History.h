#ifndef GUI_HISTORY_H
#define GUI_HISTORY_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Session/SessionUtils.h"
#include "Utils/Pimpl.h"

class QFrame;
class QDate;

UI_FWD(GUI_History)

class GUI_History :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_History)
	UI_CLASS(GUI_History)

public:
	explicit GUI_History(QWidget* parent=nullptr);
	~GUI_History() override;

	[[nodiscard]] QFrame* header() const;

private:
	void initShortcuts();
	void requestData(int index);
	void loadSelectedDateRange();

private slots:
	void scrollToTop();
	void scrollToBottom();
	void loadMore();
	void dateRangeClicked();
	void clearRangeClicked();
	void calendarFinished();

protected:
	void languageChanged() override;
};

#endif // GUI_HISTORY_H
