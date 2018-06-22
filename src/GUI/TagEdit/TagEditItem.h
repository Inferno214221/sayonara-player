#ifndef TAGEDITITEM_H
#define TAGEDITITEM_H

#include "GUI/Utils/Widgets/Widget.h"
#include "Utils/Language.h"
#include "Utils/Pimpl.h"

class QGridLayout;

class TagEditItemString :
		public SayonaraClass
{
	Q_OBJECT
	PIMPL(TagEditItemString)

	public:
		TagEditItemString(Lang::Term language_term, bool has_all, QGridLayout* layout, int row, int column, QWidget* parent=nullptr);
		virtual ~TagEditItemString();

		QString value() const;
		void set_value(const QString& value);

		// WidgetTemplate interface
	public:
		void language_changed();
};


class TagEditItemInt :
		public SayonaraClass
{
	Q_OBJECT
	PIMPL(TagEditItemInt)

	public:
		TagEditItemInt(Lang::Term language_term, bool has_all, QGridLayout* layout, int row, int column, QWidget* parent=nullptr);
		virtual ~TagEditItemInt();

		int value() const;
		void set_value(const int& value);

		void set_lower(const int& lower);
		void set_upper(const int& upper);

		// WidgetTemplate interface
	public:
		void language_changed();
};

#endif // TAGEDITITEM_H
