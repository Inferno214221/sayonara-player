#ifndef HISTORYTABLEVIEW_H
#define HISTORYTABLEVIEW_H

#include "Gui/Utils/Widgets/Dragable.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include <QTableView>

namespace Session
{
	class Manager;
}

class LibraryPlaylistInteractor;

class HistoryTableView :
	public Gui::WidgetTemplate<QTableView>,
	public Gui::Dragable
{
	Q_OBJECT
	PIMPL(HistoryTableView)

	signals:
		void sigRowcountChanged();

	public:
		HistoryTableView(LibraryPlaylistInteractor* libraryPlaylistInteractor,
		                 Session::Manager* sessionManager,
		                 Session::Timecode timecode,
		                 QWidget* parent = nullptr);
		~HistoryTableView() override;

		[[nodiscard]] int rows() const;

	protected:
		void skinChanged() override;
		void resizeEvent(QResizeEvent* e) override;
		void showEvent(QShowEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;

	private slots:
		void rowcountChanged();
		void appendTriggered();
		void playNewTabTriggered();
		void playNextTriggered();
		void playTriggered();

	private: // NOLINT(readability-redundant-access-specifiers)
		void initContextMenu();
};

#endif // HISTORYTABLEVIEW_H
