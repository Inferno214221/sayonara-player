#ifndef FLIEOPERATIONWORKERTHREAD_H
#define FLIEOPERATIONWORKERTHREAD_H

#include <QThread>
#include "Utils/Pimpl.h"

class FileOperationThread : public QThread
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
		explicit FileOperationThread(QObject* parent);
		FileOperationThread(const QStringList& sourceFiles, const QStringList& targetFiles, QObject* parent);
};

class FileMoveThread : public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileMoveThread)

	public:
		FileMoveThread(const QStringList& sourceFiles, const QString& targetDir, QObject* parent);
		~FileMoveThread();

	protected:
		void run() override;
};

class FileCopyThread : public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileCopyThread)

	public:
		FileCopyThread(const QStringList& sourceFiles, const QString& targetDir, QObject* parent);
		~FileCopyThread();

	protected:
		void run() override;
};


class FileRenameThread : public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileRenameThread)

	public:
		FileRenameThread(const QString& sourceFile, const QString& targetFile, QObject* parent);
		~FileRenameThread();

	protected:
		void run() override;
};

class FileDeleteThread : public FileOperationThread
{
	Q_OBJECT
	PIMPL(FileDeleteThread)

	public:
		FileDeleteThread(const QStringList& sourcePaths, QObject* parent);
		~FileDeleteThread() override;

	protected:
		void run() override;
};

#endif // FLIEOPERATIONWORKERTHREAD_H
