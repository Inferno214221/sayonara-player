#ifndef HISTORYENTRYWIDGET_H
#define HISTORYENTRYWIDGET_H

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include "Gui/Utils/Widgets/Widget.h"

namespace Session
{
	class Manager;
}

class HistoryEntryWidget :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(HistoryEntryWidget)

	public:
		explicit HistoryEntryWidget(Session::Manager* sessionManager, Session::Timecode timecode, QWidget* parent=nullptr);
		~HistoryEntryWidget() override;
		[[nodiscard]] Session::Id id() const;

	protected:
		void languageChanged() override;

	private slots:
		void rowcountChanged();
};

#endif // HISTORYENTRYWIDGET_H
