#ifndef LIBRARYPLUGINCOMBOBOXDELEGATE_H
#define LIBRARYPLUGINCOMBOBOXDELEGATE_H

#include "Gui/Utils/Delegates/ComboBoxDelegate.h"

class LibraryPluginComboBoxDelegate :
	public Gui::ComboBoxDelegate
{
	Q_OBJECT

	private:
		QWidget* mParent=nullptr;

	public:
		explicit LibraryPluginComboBoxDelegate(QWidget* parent);
		~LibraryPluginComboBoxDelegate();

	public:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
		QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};


#endif // LIBRARYPLUGINCOMBOBOXDELEGATE_H
