#ifndef LIBRARYSEARCHBAR_H
#define LIBRARYSEARCHBAR_H

#include <QLineEdit>
#include "Utils/Pimpl.h"
#include "Utils/Library/Filter.h"
#include "GUI/Utils/Widgets/WidgetTemplate.h"

namespace Library
{
	class  SearchBar : public Gui::WidgetTemplate<QLineEdit>
	{
		Q_OBJECT
		PIMPL(SearchBar)

		using Parent=Gui::WidgetTemplate<QLineEdit>;

		signals:
			void sig_current_mode_changed();
			void sig_text_changed(const QString& text);

		public:
			SearchBar(QWidget* parent=nullptr);
			~SearchBar();

			void set_modes(const QList<Filter::Mode>& modes);
			QList<Filter::Mode> modes() const;

			void set_current_mode(Filter::Mode mode);
			void set_next_mode();
			Filter::Mode current_mode() const;

		protected:
			void init_context_menu();
			void keyPressEvent(QKeyEvent* e) override;
			void language_changed() override;
			void skin_changed() override;

		private slots:
			void text_changed(const QString& text);
			void search_shortcut_pressed();

			void livesearch_changed();
			void livesearch_triggered(bool b);
	};
}

#endif // LIBRARYSEARCHBAR_H
