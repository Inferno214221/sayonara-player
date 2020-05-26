#ifndef HISTORYENTRYWIDGET_H
#define HISTORYENTRYWIDGET_H

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include "Gui/Utils/Widgets/Widget.h"

class HistoryEntryWidget :
	public Gui::Widget
{
	Q_OBJECT
	PIMPL(HistoryEntryWidget)

	public:
		explicit HistoryEntryWidget(Session::Timecode timecode, QWidget* parent=nullptr);
		~HistoryEntryWidget() override;

		Session::Id id() const;

	private slots:
		void rowcount_changed();

	protected:
		void languageChanged() override;
};

#endif // HISTORYENTRYWIDGET_H
