#ifndef LABEL_H
#define LABEL_H

#include "Utils/Pimpl.h"
#include <QLabel>
#include "Gui/Utils/Widgets/WidgetTemplate.h"

class QTimer;
class QString;
namespace Gui
{
	class FloatingLabel :
		public Gui::WidgetTemplate<QLabel>
	{
		Q_OBJECT
		PIMPL(FloatingLabel)

		public:
			explicit FloatingLabel(QWidget* parent=nullptr);
			~FloatingLabel();

			void setFloatingText(const QString& text);
			void setCharsPerSecond(int charsPerSecond);

		public slots:
			void updateOffset();

		protected:
			void paintEvent(QPaintEvent* event);
			void resizeEvent(QResizeEvent* event);
	};
}

#endif // LABEL_H
