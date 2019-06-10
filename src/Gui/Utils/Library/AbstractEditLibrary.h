#ifndef ABSTRACTEDITLIBRARY_H
#define ABSTRACTEDITLIBRARY_H

class QLineEdit;
class QLabel;
class QPushButton;
class QWidget

class AbstractEditLibrary
{
	public:
		AbstractEditLibrary();
		virtual ~AbstractEditLibrary();

		const QLineEdit*	le_path() const=0;
		const QLineEdit*	le_name() const=0;
		const QLabel*		lab_path() const=0;
		const QLabel*		lab_name() const=0;

		const QPushButton*	btn_search_dir() const=0;
};

#endif // ABSTRACTEDITLIBRARY_H
