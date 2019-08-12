#ifndef LIBRARYFILEEXTENSIONBAR_H
#define LIBRARYFILEEXTENSIONBAR_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

namespace Gui
{
	class ExtensionSet;
}

class AbstractLibrary;

namespace Library
{
	class FileExtensionBar : public Gui::Widget
	{
		Q_OBJECT
		PIMPL(FileExtensionBar)

	signals:
		void sig_close_clicked();

	public:
		explicit FileExtensionBar(QWidget* parent=nullptr);
		~FileExtensionBar() override;

		void init(AbstractLibrary* library);
		void refresh();
		void clear();

		bool has_extensions() const;

	protected:
		void language_changed() override;

	private slots:
		void button_toggled(bool b);
	};
}

#endif // LIBRARYFILEEXTENSIONBAR_H
