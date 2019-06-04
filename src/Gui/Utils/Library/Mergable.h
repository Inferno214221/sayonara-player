#ifndef MERGABLE_H
#define MERGABLE_H

#include "Utils/Pimpl.h"
#include "Gui/Utils/Widgets/WidgetTemplate.h"

#include <QMap>
#include <QMenu>

class QAction;
class QStringList;
class MergeData;


class MergeMenu :
	public Gui::WidgetTemplate<QMenu>
{
	Q_OBJECT
	PIMPL(MergeMenu)

	signals:
		void sig_merge_triggered();

	public:
		MergeMenu(QMenu* parent=nullptr);
		virtual ~MergeMenu();

		void set_data(const QMap<Id, QString>& data);

		QAction* action() const;
		bool is_data_valid() const;
		MergeData mergedata() const;

	protected:
		void language_changed() override;
};

#endif // MERGABLE_H
