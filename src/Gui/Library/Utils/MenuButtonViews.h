#ifndef MENUBUTTONVIEWS_H
#define MENUBUTTONVIEWS_H

#include "Gui/Utils/MenuTool/MenuToolButton.h"


namespace Library
{
	class MenuButtonViews : public Gui::MenuToolButton
	{
		Q_OBJECT
		PIMPL(MenuButtonViews)

		public:
			MenuButtonViews(QWidget* parent=nullptr);
			~MenuButtonViews() override;

		private slots:
			void actionTriggered(bool b);
			void viewTypeChanged();

		protected:
			void languageChanged() override;
			void skinChanged() override;
	};
}

#endif // MENUBUTTONVIEWS_H
