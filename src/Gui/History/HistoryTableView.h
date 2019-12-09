#ifndef HISTORYTABLEVIEW_H
#define HISTORYTABLEVIEW_H

#include <QTableView>

#include "Utils/Pimpl.h"
#include "Utils/Session/SessionUtils.h"

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/Widgets/Dragable.h"

class HistoryTableView :
	public Gui::WidgetTemplate<QTableView>,
	public Gui::Dragable
{
	Q_OBJECT
	PIMPL(HistoryTableView)

	signals:
		void sig_rowcount_changed();

	public:
		HistoryTableView(Session::Timecode timecode, QWidget* parent=nullptr);
		~HistoryTableView() override;

		int rows() const;

	protected:
		void language_changed() override;
		void skin_changed() override;

		void resizeEvent(QResizeEvent* e) override;

	private slots:
		void rowcount_changed();

		// Dragable interface
protected:
	QMimeData* dragable_mimedata() const override;
};

#endif // HISTORYTABLEVIEW_H
