#ifndef LIBRARYPLUGINCOMBOBOXDELEGATE_H
#define LIBRARYPLUGINCOMBOBOXDELEGATE_H

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"

class LibraryPluginComboBoxDelegate :
	public Gui::ComboBoxDelegate
{
	Q_OBJECT

	public:
		explicit LibraryPluginComboBoxDelegate(QObject* parent=nullptr);
		~LibraryPluginComboBoxDelegate();

	public:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};


#endif // LIBRARYPLUGINCOMBOBOXDELEGATE_H
