#ifndef GUI_HISTORY_H
#define GUI_HISTORY_H

#include "Gui/Utils/Widgets/Dialog.h"
#include "Utils/Session/SessionUtils.h"
#include "Utils/Pimpl.h"

class QFrame;
class QDate;

UI_FWD(GUI_History)

namespace Session
{
	class Manager;
}

class GUI_History :
	public Gui::Dialog
{
	Q_OBJECT
	PIMPL(GUI_History)
	UI_CLASS_SHARED_PTR(GUI_History)

	public:
		explicit GUI_History(Session::Manager* sessionManager, QWidget* parent = nullptr);
		~GUI_History() override;

		[[nodiscard]] QFrame* header() const;

	private:
		void initShortcuts();
		void requestData(int index);
		void assureOneTable();
		void loadSelectedDateRange();

	private slots: // NOLINT(readability-redundant-access-specifiers)
		void scrollToTop();
		void scrollToBottom();
		void loadMore();
		void dateRangeClicked();
		void clearRangeClicked();
		void calendarFinished();
		void clearAllHistoryClicked();
		void clearOldHistoryClicked();

	protected:
		void languageChanged() override;
};

#endif // GUI_HISTORY_H
