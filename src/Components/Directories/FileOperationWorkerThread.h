#ifndef FLIEOPERATIONWORKERTHREAD_H
#define FLIEOPERATIONWORKERTHREAD_H

#include <QThread>
#include "Utils/Pimpl.h"

namespace Library
{
	class InfoAccessor;
}

class FileOperationThread :
	public QThread
{
	Q_OBJECT
	PIMPL(FileOperationThread)

	signals:
		void sigStarted();
		void sigFinished();

	public:
		virtual ~FileOperationThread();

		QList<LibraryId> sourceIds() const;
		QList<LibraryId> targetIds() const;

	protected:
		FileOperationThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		                    const QStringList& targetFiles, QObject* parent);
		Library::InfoAccessor* libraryInfoAccessor();
};

class FileMoveThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileMoveThread)

	public:
		FileMoveThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		               const QString& targetDir, QObject* parent);
		~FileMoveThread() override;

	protected:
		void run() override;
};

class FileCopyThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileCopyThread)

	public:
		FileCopyThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourceFiles,
		               const QString& targetDir, QObject* parent);
		~FileCopyThread() override;

	protected:
		void run() override;
};

class FileRenameThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileRenameThread)

	public:
		FileRenameThread(Library::InfoAccessor* libraryInfoAccessor, const QString& sourceFile,
		                 const QString& targetFile,
		                 QObject* parent);
		~FileRenameThread() override;

	protected:
		void run() override;
};

class FileDeleteThread :
	public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileDeleteThread)

	public:
		FileDeleteThread(Library::InfoAccessor* libraryInfoAccessor, const QStringList& sourcePaths, QObject* parent);
		~FileDeleteThread() override;

	protected:
		void run() override;
};

#endif // FLIEOPERATIONWORKERTHREAD_H
