#ifndef BOTTOMBARBUTTON_H
#define BOTTOMBARBUTTON_H

#include <QPushButton>

class QPixmap;

class BottomBarButton : public QPushButton
{
	public:
		BottomBarButton(const QPixmap& pm, QWidget* parent);
		~BottomBarButton();

	private:
		QPixmap m_pixmap;

		using QPushButton::setIcon;

	protected:
		void paintEvent(QPaintEvent* e) override;

	public:
		void set_pixmap(const QPixmap& pm);



};

#endif // BOTTOMBARBUTTON_H
