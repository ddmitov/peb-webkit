﻿/*
 Perl Executing Browser

 This program is free software;
 you can redistribute it and/or modify it under the terms of the
 GNU General Public License, as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.
 Dimitar D. Mitov, 2013 - 2016
 Valcho Nedelchev, 2014 - 2016
 https://github.com/ddmitov/perl-executing-browser
*/

#ifndef PEB_H
#define PEB_H

#include <QApplication>
#include <QMainWindow>
#include <QtNetwork/QNetworkAccessManager>
#include <QNetworkReply>
#include <QtWebKit>
#include <QUrl>
#include <QWebPage>
#include <QWebView>
#include <QWebFrame>
#include <QWebInspector>
#include <QProcess>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QMenu>
#include <QDesktopWidget>
#include <qglobal.h>

#include <QMenu>

// ==============================
// PRINT SUPPORT:
// ==============================
#ifndef QT_NO_PRINTER
#include <QPrintPreviewDialog>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#endif

// ==============================
// PSEUDO-DOMAIN:
// ==============================
#ifndef PSEUDO_DOMAIN
#define PSEUDO_DOMAIN "perl-executing-browser-pseudodomain"
#endif

// ==============================
// MAIN WINDOW CLASS DEFINITION:
// ==============================
class QMainBrowserWindow : public QMainWindow
{
    Q_OBJECT

public slots:
    void setMainWindowTitleSlot(QString title)
    {
        setWindowTitle(title);
    }

    void closeEvent(QCloseEvent *event)
    {
        if (qApp->property("mainWindowCloseRequested").toBool() == false) {
            if (webViewWidget->
                    page()->mainFrame()->childFrames().length() > 0) {
                foreach (QWebFrame *frame,
                         webViewWidget->page()->mainFrame()->childFrames()) {
                    qCheckUserInputBeforeClose(frame, event);
                }
            } else {
                qCheckUserInputBeforeClose(
                            webViewWidget->page()->mainFrame(), event);
            }
        }

        // If closing the main window is requested using
        // the special window closing URL,
        // no check for user input is performed.
        if (qApp->property("mainWindowCloseRequested").toBool() == true) {
            event->accept();
        }
    }

    void qCheckUserInputBeforeClose(QWebFrame *frame, QCloseEvent *event)
    {
        frame->evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant checkUserInputJsResult =
                frame->evaluateJavaScript("pebCheckUserInputBeforeClose()");
        bool textIsEntered = checkUserInputJsResult.toBool();

        QVariant checkCloseWarningJsResult =
                frame->evaluateJavaScript("pebCheckCloseWarning()");
        QString closeWarning = checkCloseWarningJsResult.toString();

        if (textIsEntered == false) {
            event->accept();
        }

        if (textIsEntered == true) {
            if (closeWarning == "async") {
                event->ignore();
                frame->evaluateJavaScript("pebCloseConfirmationAsync()");
            }
            if (closeWarning == "sync") {
                QVariant jsSyncResult =
                        frame->evaluateJavaScript("pebCloseConfirmationSync()");

                bool jsCloseDecision;
                if (jsSyncResult.toString().length() > 0) {
                    jsCloseDecision = jsSyncResult.toBool();
                } else {
                    jsCloseDecision = true;
                }

                if (jsCloseDecision == true) {
                    event->accept();
                }
                if (jsCloseDecision == false) {
                    event->ignore();
                }
            }
        }
    }

    void qExitApplicationSlot()
    {
        qDebug() << qApp->applicationName().toLatin1().constData()
                 << qApp->applicationVersion().toLatin1().constData()
                 << "terminated normally.";

        QApplication::exit();
    }

public:
    explicit QMainBrowserWindow(QWidget *parent = 0);
    QWebView *webViewWidget;
};

// ==============================
// FILE DETECTOR CLASS DEFINITION:
// ==============================
class QFileDetector : public QObject
{
    Q_OBJECT

public slots:
    void qDefineInterpreter(QString filepath)
    {
        interpreter = "undefined";

        QFileInfo fileInfo(filepath);
        extension = fileInfo.suffix();

        if (extension.length() == 0) {
            QString firstLine;

            QFile file(filepath);
            if (file.open (QIODevice::ReadOnly | QIODevice::Text)) {
                firstLine = file.readLine();
            }

            if (firstLine.contains(perlShebang)) {
                interpreter = (qApp->property("perlInterpreter").toString());
            }
        } else {
            if (extension.contains(htmlExtensions) or
                    extension.contains(xmlExtension) or
                    extension.contains(jsonExtension) or
                    extension.contains(cssExtension) or
                    extension.contains(jsExtension)) {
                interpreter = "browser-text";
            }
            if (extension.contains(ttfExtension) or
                    extension.contains(eotExtension) or
                    extension.contains(woffExtensions) or
                    extension.contains(svgExtension) or
                    extension.contains(pngExtension) or
                    extension.contains(jpgExtensions) or
                    extension.contains(gifExtension)) {
                interpreter = "browser-images";
            }

            if (extension == "pl") {
                interpreter = (qApp->property("perlInterpreter").toString());
            }
        }
    }

    void qCheckFileExistence(QString fullFilePath)
    {
        QFile file(QDir::toNativeSeparators(fullFilePath));
        if (!file.exists()) {
            fileExists = false;
            qDebug() << QDir::toNativeSeparators(fullFilePath) <<
                        "is missing.";
        }
    }

public:
    QFileDetector();
    bool fileExists;
    QString interpreter;

private:
    QString extension;
    QRegExp perlShebang;

    // Regular expressions for file type detection by extension:
    QRegExp plExtension;
    QRegExp htmlExtensions;
    QRegExp xmlExtension;
    QRegExp jsonExtension;
    QRegExp cssExtension;
    QRegExp jsExtension;
    QRegExp svgExtension;
    QRegExp ttfExtension;
    QRegExp eotExtension;
    QRegExp woffExtensions;
    QRegExp pngExtension;
    QRegExp jpgExtensions;
    QRegExp gifExtension;
};

// ==============================
// CUSTOM NETWORK REPLY CLASS DEFINITION:
// ==============================
class QCustomNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:

    QCustomNetworkReply(const QUrl &url, QString &data);
    ~QCustomNetworkReply();

    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;
    qint64 size() const;

protected:
    qint64 read(char *data, qint64 maxSize);
    qint64 readData(char *data, qint64 maxSize);

private:
    struct QCustomNetworkReplyPrivate *reply;
};

// ==============================
// NETWORK ACCESS MANAGER
// CLASS DEFINITION:
// ==============================
class QAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

signals:
    void startScriptSignal(QUrl url, QByteArray postDataArray);
    void closeWindowSignal();

protected:
    virtual QNetworkReply *createRequest(Operation operation,
                                         const QNetworkRequest &request,
                                         QIODevice *outgoingData = 0)
    {
        // Window closing URL:
        if (operation == GetOperation and
                request.url().authority() == PSEUDO_DOMAIN and
                request.url().fileName() == "close-window.function") {
                    emit closeWindowSignal();

            QNetworkRequest emptyNetworkRequest;
            return QNetworkAccessManager::createRequest
                    (QNetworkAccessManager::GetOperation,
                     QNetworkRequest(emptyNetworkRequest));
        }

        // Local AJAX GET and POST requests:
        if ((operation == GetOperation or
             operation == PostOperation) and
                request.url().authority() == PSEUDO_DOMAIN and
                request.url().path().contains("ajax")) {

            QString ajaxScriptFullFilePath = QDir::toNativeSeparators
                    ((qApp->property("application").toString())
                     + request.url().path());

            QFileDetector fileDetector;
            fileDetector.qCheckFileExistence(ajaxScriptFullFilePath);

            if (fileDetector.fileExists == true) {
                if (qApp->property("perlInterpreter").toString().length() > 0) {
                    QString queryString = request.url().query();

                    QByteArray postDataArray;
                    if (outgoingData) {
                        postDataArray = outgoingData->readAll();
                    }
                    QString postData(postDataArray);

                    qDebug() << "AJAX script started:"
                             << ajaxScriptFullFilePath;

                    QProcessEnvironment scriptEnvironment;

                    if (queryString.length() > 0) {
                        scriptEnvironment.insert("REQUEST_METHOD", "GET");
                        scriptEnvironment.insert("QUERY_STRING", queryString);
                        // qDebug() << "Query string:" << queryString;
                    }

                    if (postData.length() > 0) {
                        scriptEnvironment
                                .insert("REQUEST_METHOD", "POST");
                        QString postDataSize = QString::number(postData.size());
                        scriptEnvironment
                                .insert("CONTENT_LENGTH", postDataSize);
                        // qDebug() << "POST data:" << postData;
                    }

                    QProcess ajaxScriptHandler;
                    ajaxScriptHandler.setProcessEnvironment(scriptEnvironment);

                    // Non-blocking event loop waiting for AJAX script results:
                    QEventLoop ajaxScriptHandlerWaitingLoop;
                    QObject::connect(&ajaxScriptHandler,
                                     SIGNAL(finished(
                                                int, QProcess::ExitStatus)),
                                     &ajaxScriptHandlerWaitingLoop,
                                     SLOT(quit()));

                    // AJAX scripts timer- 2 seconds:
                    // QTimer::singleShot(2000, &scriptHandlerWaitingLoop,
                    //                    SLOT(quit()));

                    QString ajaxScriptResultString;
                    QString ajaxScriptErrorString;

                    // 'censor.pl' is compiled into the resources of
                    // the binary file and called from there.
                    QString censorScriptFileName(
                                ":/scripts/perl/censor.pl");
                    QFile censorScriptFile(censorScriptFileName);
                    censorScriptFile.open(QIODevice::ReadOnly
                                          | QIODevice::Text);
                    QTextStream stream(&censorScriptFile);
                    QString censorScriptContents = stream.readAll();
                    censorScriptFile.close();

                    ajaxScriptHandler.start((qApp->property("perlInterpreter")
                                             .toString()),
                                            QStringList()
                                            << "-e"
                                            << censorScriptContents
                                            << "--"
                                            << QDir::toNativeSeparators(
                                                ajaxScriptFullFilePath),
                                            QProcess::Unbuffered
                                            | QProcess::ReadWrite);

                    if (postData.length() > 0) {
                        ajaxScriptHandler.write(postDataArray);
                    }

                    ajaxScriptHandlerWaitingLoop.exec();

                    QByteArray scriptResultArray =
                            ajaxScriptHandler.readAllStandardOutput();
                    ajaxScriptResultString =
                            QString::fromLatin1(scriptResultArray);

                    QByteArray scriptErrorArray =
                            ajaxScriptHandler.readAllStandardError();
                    ajaxScriptErrorString =
                            QString::fromLatin1(scriptErrorArray);

                    if (ajaxScriptResultString.length() == 0 and
                            ajaxScriptErrorString == 0) {
                        qDebug() << "AJAX script timed out or gave no output:"
                                 << ajaxScriptFullFilePath;
                    } else {
                        qDebug() << "AJAX script finished:"
                                 << ajaxScriptFullFilePath;
                    }

                    if (ajaxScriptErrorString.length() > 0) {
                        qDebug() << "AJAX script errors:";
                        QStringList scriptErrors =
                                ajaxScriptErrorString.split("\n");
                        foreach (QString scriptError, scriptErrors) {
                            if (scriptError.length() > 0) {
                                qDebug() << scriptError;
                            }
                        }
                    }

                    QWebSettings::clearMemoryCaches();

                    scriptEnvironment.remove("REQUEST_METHOD");

                    if (queryString.length() > 0) {
                        scriptEnvironment.remove("QUERY_STRING");
                    }

                    if (postData.length() > 0) {
                        scriptEnvironment.remove("CONTENT_LENGTH");
                    }

                    QCustomNetworkReply *reply =
                            new QCustomNetworkReply (request.url(),
                                                     ajaxScriptResultString);
                    return reply;
                } else {
                    qDebug() << "AJAX script not started:"
                             << ajaxScriptFullFilePath;
                    qDebug() << "Perl interpreter was not set.";

                    QNetworkRequest emptyNetworkRequest;
                    return QNetworkAccessManager::createRequest
                            (QNetworkAccessManager::GetOperation,
                             QNetworkRequest(emptyNetworkRequest));
                }
            }

            if (fileDetector.fileExists == false) {
                QNetworkRequest emptyNetworkRequest;
                return QNetworkAccessManager::createRequest
                        (QNetworkAccessManager::GetOperation,
                         QNetworkRequest(emptyNetworkRequest));
            }
        }

        // GET requests to the browser pseudodomain -
        // local files and CGI-like scripts:
        if (operation == GetOperation and
                request.url().authority() == PSEUDO_DOMAIN and
                (!request.url().path().contains("ajax"))) {

            // Get the full file path and file extension:
            QString fullFilePath = QDir::toNativeSeparators
                    ((qApp->property("application").toString())
                     + request.url().path());

            QFileDetector fileDetector;
            fileDetector.qCheckFileExistence(fullFilePath);

            if (fileDetector.fileExists == true) {
                fileDetector.qDefineInterpreter(fullFilePath);
                QString interpreter = fileDetector.interpreter;

                // Handle local Perl scripts:
                if (interpreter ==
                        qApp->property("perlInterpreter").toString()) {

                    QByteArray emptyPostDataArray;
                    emit startScriptSignal(request.url(), emptyPostDataArray);

                    QNetworkRequest emptyNetworkRequest;
                    return QNetworkAccessManager::createRequest
                            (QNetworkAccessManager::GetOperation,
                             QNetworkRequest(emptyNetworkRequest));
                }

                // Handle other supported local files:
                if (interpreter.contains ("browser")) {
                    if (interpreter.contains ("text")) {
                        qDebug() << "Local link requested:"
                                 << request.url().toString();

                        QString localFileName(QDir::toNativeSeparators(
                                                  (qApp->property("application")
                                                   .toString())
                                                  + request.url().path()));
                        QFile localFile(localFileName);
                        localFile.open(QIODevice::ReadOnly);
                        QTextStream stream(&localFile);
                        QString localFileContents = stream.readAll();
                        localFile.close();

                        QCustomNetworkReply *reply =
                                new QCustomNetworkReply (request.url(),
                                                         localFileContents);
                        return reply;
                    }

                    if (interpreter.contains ("images")) {
                        qDebug() << "Local link requested:"
                                 << request.url().toString();

                        QString localFileName(QDir::toNativeSeparators(
                                                  (qApp->property("application")
                                                   .toString())
                                                  + request.url().path()));

                        QNetworkRequest networkRequest;
                        networkRequest
                                .setUrl(QUrl::fromLocalFile(localFileName));
                        return QNetworkAccessManager::createRequest
                                (QNetworkAccessManager::GetOperation,
                                 QNetworkRequest(networkRequest));
                    }
                }

                // Unsupported local files:
                if (interpreter.contains("undefined")) {
                    qDebug() << "File type not supported by browser:"
                             << fullFilePath;

                    QDesktopServices::openUrl(QUrl::fromLocalFile(
                                                  fullFilePath));

                    QNetworkRequest emptyNetworkRequest;
                    return QNetworkAccessManager::createRequest
                            (QNetworkAccessManager::GetOperation,
                             QNetworkRequest(emptyNetworkRequest));
                }
            }

            if (fileDetector.fileExists == false) {
                qDebug() << "File not found:"
                         << fullFilePath;

                QNetworkRequest emptyNetworkRequest;
                return QNetworkAccessManager::createRequest
                        (QNetworkAccessManager::GetOperation,
                         QNetworkRequest(emptyNetworkRequest));
            }
        }

        // POST requests to the browser pseudodomain - CGI-like scripts:
        if (operation == PostOperation and
                request.url().authority() == PSEUDO_DOMAIN and
                (!request.url().path().contains("ajax"))) {

            if (outgoingData) {
                QByteArray postDataArray = outgoingData->readAll();
                emit startScriptSignal(request.url(), postDataArray);
            }

            QNetworkRequest emptyNetworkRequest;
            return QNetworkAccessManager::createRequest
                    (QNetworkAccessManager::GetOperation,
                     QNetworkRequest(emptyNetworkRequest));
        }

        qDebug() << "Link requested:"
                 << request.url().toString();

        return QNetworkAccessManager::createRequest
                (QNetworkAccessManager::GetOperation,
                 QNetworkRequest(request));
    }
};

// ==============================
// WEB PAGE CLASS CONSTRUCTOR:
// ==============================
class QPage : public QWebPage
{
    Q_OBJECT

signals:
    void changeTitleSignal();
    void displayErrorsSignal(QString errors);
    void printPreviewSignal();
    void printSignal();
    void selectInodeSignal(QNetworkRequest request);
    void closeWindowSignal();

public slots:
    void qPageLoadedSlot(bool ok)
    {
        if (ok) {
            emit changeTitleSignal();
        }
    }

    void qStartScriptSlot(QUrl url, QByteArray postDataArray)
    {
        scriptFullFilePath = QDir::toNativeSeparators
                ((qApp->property("application").toString()) + url.path());

        if (qApp->property("perlInterpreter").toString().length() > 0) {
            if (scriptHandler.isOpen()) {
                scriptHandler.close();
                qDebug() << "Script is going to be restarted:"
                         << scriptFullFilePath;
            }

            QString queryString = url.query();
            QString postData(postDataArray);

            qDebug() << "Script started:" << scriptFullFilePath;

            QProcessEnvironment scriptEnvironment;

            if (queryString.length() > 0) {
                scriptEnvironment.insert("REQUEST_METHOD", "GET");
                scriptEnvironment.insert("QUERY_STRING", queryString);
                // qDebug() << "Query string:" << queryString;
            }

            if (postData.length() > 0) {
                scriptEnvironment.insert("REQUEST_METHOD", "POST");
                QString postDataSize = QString::number(postData.size());
                scriptEnvironment.insert("CONTENT_LENGTH", postDataSize);
                // qDebug() << "POST data:" << postData;
            }

            scriptHandler.setProcessEnvironment(scriptEnvironment);

            // 'censor.pl' is compiled into the resources of
            // the binary file and called from there.
            QString censorScriptFileName(":/scripts/perl/censor.pl");
            QFile censorScriptFile(censorScriptFileName);
            censorScriptFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QTextStream censorStream(&censorScriptFile);
            QString censorScriptContents = censorStream.readAll();
            censorScriptFile.close();

            scriptHandler.start((qApp->property("perlInterpreter").toString()),
                                QStringList()
                                << "-e"
                                << censorScriptContents
                                << "--"
                                << QDir::toNativeSeparators(scriptFullFilePath),
                                QProcess::Unbuffered | QProcess::ReadWrite);

            if (postData.length() > 0) {
                scriptHandler.write(postDataArray);
            }

            QWebSettings::clearMemoryCaches();

            scriptEnvironment.remove("REQUEST_METHOD");

            if (queryString.length() > 0) {
                scriptEnvironment.remove("QUERY_STRING");
            }

            if (postData.length() > 0) {
                scriptEnvironment.remove("CONTENT_LENGTH");
            }
        } else {
            qDebug() << "Script not started:" << scriptFullFilePath;
            qDebug() << "Perl interpreter was not set.";
        }
    }

    void qScriptOutputSlot()
    {
        QString output = scriptHandler.readAllStandardOutput();
        scriptAccumulatedOutput.append(output);

        QPage::currentFrame()->setHtml(scriptAccumulatedOutput,
                                       QUrl(PSEUDO_DOMAIN));

        qDebug() << QDateTime::currentMSecsSinceEpoch()
                 << "msecs from epoch: output from" << scriptFullFilePath;
    }

    void qScriptErrorsSlot()
    {
        QString error = scriptHandler.readAllStandardError();
        scriptAccumulatedErrors.append(error);
        scriptAccumulatedErrors.append("\n");
    }

    void qScriptFinishedSlot()
    {
        QPage::currentFrame()->setHtml(scriptAccumulatedOutput,
                                       QUrl(PSEUDO_DOMAIN));

        if (scriptAccumulatedErrors.length() > 0) {
            if (scriptAccumulatedOutput.length() == 0) {
                QPage::currentFrame()->setHtml(scriptAccumulatedErrors,
                                               QUrl(PSEUDO_DOMAIN));
            } else {
                emit displayErrorsSignal(scriptAccumulatedErrors);
            }
        }

        scriptHandler.close();
        scriptAccumulatedOutput = "";
        scriptAccumulatedErrors = "";

        qDebug() << "Script finished:" << scriptFullFilePath;
    }

    void qSslErrorsSlot(QNetworkReply *reply, const QList<QSslError> &errors)
    {
        reply->ignoreSslErrors();

        foreach (QSslError error, errors) {
            qDebug() << "SSL error:" << error;
        }
    }

    void qCloseWindowFromURLTransmitterSlot()
    {
        emit closeWindowSignal();
    }

    // ==============================
    // PERL DEBUGGER INTERACTION.
    // Implementation of an idea proposed by Valcho Nedelchev.
    // ==============================
#ifndef Q_OS_WIN
    void qStartPerlDebuggerSlot()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            // Read and store in memory
            // the Perl debugger output formatter script:
            QFile debuggerOutputFormatterFile(":/scripts/perl/dbgformatter.pl");
            debuggerOutputFormatterFile.open(QFile::ReadOnly | QFile::Text);
            debuggerOutputFormatterScript =
                    QString(debuggerOutputFormatterFile.readAll());
            debuggerOutputFormatterFile.close();

            qDebug() << "File passed to Perl debugger:"
                     << QDir::toNativeSeparators(debuggerScriptToDebug);

            // Clean accumulated debugger output from previous debugger session:
            debuggerAccumulatedOutput = "";

            QString commandLineArguments;

            if (debuggerHandler.isOpen()) {
                QByteArray debuggerCommand;
                debuggerCommand.append(debuggerLastCommand.toLatin1());
                debuggerCommand.append(QString("\n").toLatin1());
                debuggerHandler.write(debuggerCommand);
            } else {
                debuggerJustStarted = true;

                if (debuggerScriptToDebug
                        .contains(qApp->property("application").toString())) {
                    debuggerHandler.setProcessEnvironment(scriptEnvironment);
                } else {
                    bool ok;
                    QString input =
                            QInputDialog::getText(
                                qApp->activeWindow(),
                                "Command Line",
                                "Enter all command line arguments, if any:",
                                QLineEdit::Normal,
                                "",
                                &ok);
                    if (ok && !input.isEmpty()) {
                        commandLineArguments = input;
                    }

                    debuggerHandler.setProcessEnvironment(
                                QProcessEnvironment::systemEnvironment());
                }

                QFileInfo scriptAbsoluteFilePath(debuggerScriptToDebug);
                QString scriptDirectory = scriptAbsoluteFilePath.absolutePath();
                debuggerHandler.setWorkingDirectory(scriptDirectory);

                debuggerHandler.setProcessChannelMode(QProcess::MergedChannels);
                debuggerHandler.start(qApp->property("perlInterpreter")
                                      .toString(),
                                      QStringList()
                                      << "-d"
                                      << QDir::toNativeSeparators(
                                          debuggerScriptToDebug)
                                      << commandLineArguments,
                                      QProcess::Unbuffered
                                      | QProcess::ReadWrite);

                QByteArray debuggerCommand;
                debuggerCommand.append(debuggerLastCommand.toLatin1());
                debuggerCommand.append(QString("\n").toLatin1());
                debuggerHandler.write(debuggerCommand);

                qDebug() << QDateTime::currentMSecsSinceEpoch()
                         << "msecs from epoch: command sent to Perl debugger:"
                         << debuggerLastCommand;
            }
        }
    }

    void qDebuggerOutputSlot()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            // Read debugger output:
            QString debuggerOutput = debuggerHandler.readAllStandardOutput();

            // Append last output of the debugger to
            // the accumulated debugger output:
            debuggerAccumulatedOutput.append(debuggerOutput);

            qDebug() << QDateTime::currentMSecsSinceEpoch()
                     << "msecs from epoch: output from Perl debugger received.";
            // qDebug() << "Debugger raw output:" << endl
            //          << debuggerOutput;

            // Formatting of Perl debugger output is started only after
            // the final command prompt comes out of the debugger:
            if (debuggerJustStarted == true) {
                if ((debuggerLastCommand.length() > 0) and
                        (debuggerAccumulatedOutput.contains(
                             QRegExp ("DB\\<\\d{1,5}\\>.*DB\\<\\d{1,5}\\>")))) {
                    debuggerJustStarted = false;
                    if (debuggerOutputHandler.isOpen()) {
                        debuggerOutputHandler.close();
                    }
                    qDebuggerStartHtmlFormatter();
                }

                if ((debuggerLastCommand.length() == 0) and
                        (debuggerAccumulatedOutput
                         .contains(QRegExp ("DB\\<\\d{1,5}\\>")))) {
                    debuggerJustStarted = false;
                    if (debuggerOutputHandler.isOpen()) {
                        debuggerOutputHandler.close();
                    }
                    qDebuggerStartHtmlFormatter();
                }
            }

            if ((debuggerJustStarted == false) and
                    (debuggerAccumulatedOutput
                     .contains(QRegExp ("DB\\<\\d{1,5}\\>")))) {
                if (debuggerOutputHandler.isOpen()) {
                    debuggerOutputHandler.close();
                }
                qDebuggerStartHtmlFormatter();
            }
        }
    }

    void qDebuggerStartHtmlFormatter()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            scriptEnvironment.insert("REQUEST_METHOD", "POST");
            QString debuggerDataSize =
                    QString::number(debuggerAccumulatedOutput.size());
            scriptEnvironment.insert("CONTENT_LENGTH", debuggerDataSize);

            debuggerOutputHandler.setProcessEnvironment(scriptEnvironment);

            debuggerOutputFormatterScript
                    .replace("PEBLIB_PATH",
                             QApplication::applicationDirPath()
                             + QDir::separator()
                             + "peblib");

            debuggerOutputFormatterScript
                    .replace("SCRIPT", debuggerScriptToDebug);
            debuggerOutputFormatterScript
                    .replace("DEBUGGER_COMMAND", debuggerLastCommand);

            debuggerOutputHandler
                    .start((qApp->property("perlInterpreter")
                            .toString()),
                           QStringList()
                           << "-e"
                           << debuggerOutputFormatterScript,
                           QProcess::Unbuffered
                           | QProcess::ReadWrite);

            QByteArray debuggerAccumulatedOutputArray;
            debuggerAccumulatedOutputArray
                    .append(debuggerAccumulatedOutput);
            debuggerOutputHandler.write(debuggerAccumulatedOutputArray);

            scriptEnvironment.remove("REQUEST_METHOD");
            scriptEnvironment.remove("CONTENT_LENGTH");
            debuggerAccumulatedOutput = "";

            qDebug() << QDateTime::currentMSecsSinceEpoch()
                     << "msecs from epoch:"
                     << "Perl debugger output formatter script started.";
        }
    }

    void qDebuggerHtmlFormatterOutputSlot()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            QString debuggerHtmlOutput =
                    debuggerOutputHandler.readAllStandardOutput();

            // Append last output of the debugger formatter to
            // the accumulated debugger formatter output:
            debuggerAccumulatedHtmlOutput.append(debuggerHtmlOutput);

            qDebug() << QDateTime::currentMSecsSinceEpoch()
                     << "msecs from epoch:"
                     << "output from Perl debugger formatter received.";
        }
    }

    void qDebuggerHtmlFormatterErrorsSlot()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            QString debuggerOutputFormatterErrors =
                    debuggerOutputHandler.readAllStandardError();

            qDebug() << "Perl debugger formatter error:"
                     << debuggerOutputFormatterErrors;
        }
    }

    void qDebuggerHtmlFormatterFinishedSlot()
    {
        if (PERL_DEBUGGER_INTERACTION == 1) {
            targetFrame->setHtml(debuggerAccumulatedHtmlOutput,
                                 QUrl(PSEUDO_DOMAIN));

            qDebug() << QDateTime::currentMSecsSinceEpoch()
                     << "msecs from epoch:"
                     << "output from Perl debugger formatter displayed.";

            // qDebug() << "Perl debugger formatter output:" << endl
            //          << debuggerAccumulatedHtmlOutput;

            debuggerOutputHandler.close();
            debuggerAccumulatedHtmlOutput = "";
        }
    }
#endif

public:
    QPage();

protected:
    bool acceptNavigationRequest(QWebFrame *frame,
                                 const QNetworkRequest &request,
                                 QWebPage::NavigationType type);

    virtual void javaScriptAlert(QWebFrame *frame, const QString &msg)
    {
        frame->evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant messageBoxElementsJsResult =
                frame->evaluateJavaScript("pebFindMessageBoxElements()");

        QJsonDocument messageBoxElementsJsonDocument =
                QJsonDocument::fromJson(
                    messageBoxElementsJsResult.toString().toUtf8());

        QJsonObject messageBoxElementsJsonObject =
                messageBoxElementsJsonDocument.object();

        if (messageBoxElementsJsonObject.length() > 0) {
            if (messageBoxElementsJsonObject["alertTitle"]
                    .toString().length() > 0) {
                alertTitle =
                        messageBoxElementsJsonObject["alertTitle"]
                        .toString();
            }

            if (messageBoxElementsJsonObject["okLabel"]
                    .toString().length() > 0) {
                okLabel =
                        messageBoxElementsJsonObject["okLabel"].toString();
            }
        }

        QMessageBox javaScriptAlertMessageBox (qApp->activeWindow());
        javaScriptAlertMessageBox.setWindowModality(Qt::WindowModal);
        javaScriptAlertMessageBox.setWindowTitle(alertTitle);
        javaScriptAlertMessageBox.setText(msg);
        javaScriptAlertMessageBox.setButtonText(QMessageBox::Ok, okLabel);
        javaScriptAlertMessageBox.setDefaultButton(QMessageBox::Ok);
        javaScriptAlertMessageBox.exec();
    }

    virtual bool javaScriptConfirm(QWebFrame *frame, const QString &msg)
    {
        frame->evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant messageBoxElementsJsResult =
                frame->evaluateJavaScript("pebFindMessageBoxElements()");

        QJsonDocument messageBoxElementsJsonDocument =
                QJsonDocument::fromJson(
                    messageBoxElementsJsResult.toString().toUtf8());

        QJsonObject messageBoxElementsJsonObject =
                messageBoxElementsJsonDocument.object();

        if (messageBoxElementsJsonObject.length() > 0) {
            if (messageBoxElementsJsonObject["confirmTitle"]
                    .toString().length() > 0) {
                confirmTitle =
                        messageBoxElementsJsonObject["confirmTitle"]
                        .toString();
            }

            if (messageBoxElementsJsonObject["yesLabel"]
                    .toString().length() > 0) {
                yesLabel =
                        messageBoxElementsJsonObject["yesLabel"].toString();
            }

            if (messageBoxElementsJsonObject["noLabel"]
                    .toString().length() > 0) {
                noLabel =
                        messageBoxElementsJsonObject["noLabel"].toString();
            }
        }

        QMessageBox javaScriptConfirmMessageBox (qApp->activeWindow());
        javaScriptConfirmMessageBox.setWindowModality(Qt::WindowModal);
        javaScriptConfirmMessageBox.setWindowTitle(confirmTitle);
        javaScriptConfirmMessageBox.setText(msg);
        javaScriptConfirmMessageBox
                .setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        javaScriptConfirmMessageBox.setButtonText(QMessageBox::Yes, yesLabel);
        javaScriptConfirmMessageBox.setButtonText(QMessageBox::No, noLabel);
        javaScriptConfirmMessageBox.setDefaultButton(QMessageBox::No);
        return QMessageBox::Yes == javaScriptConfirmMessageBox.exec();
    }

    virtual bool javaScriptPrompt(QWebFrame *frame,
                                  const QString &msg,
                                  const QString &defaultValue,
                                  QString *result)
    {
        frame->evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant messageBoxElementsJsResult =
                frame->evaluateJavaScript("pebFindMessageBoxElements()");

        QJsonDocument messageBoxElementsJsonDocument =
                QJsonDocument::fromJson(
                    messageBoxElementsJsResult.toString().toUtf8());

        QJsonObject messageBoxElementsJsonObject =
                messageBoxElementsJsonDocument.object();

        if (messageBoxElementsJsonObject.length() > 0) {
            if (messageBoxElementsJsonObject["promptTitle"]
                    .toString().length() > 0) {
                promptTitle =
                        messageBoxElementsJsonObject["promptTitle"]
                        .toString();
            }

            if (messageBoxElementsJsonObject["okLabel"]
                    .toString().length() > 0) {
                okLabel =
                        messageBoxElementsJsonObject["okLabel"].toString();
            }

            if (messageBoxElementsJsonObject["cancelLabel"]
                    .toString().length() > 0) {
                cancelLabel =
                        messageBoxElementsJsonObject["cancelLabel"].toString();
            }
        }

        bool ok = false;

        QInputDialog dialog;
        dialog.setModal(true);
        dialog.setWindowTitle(promptTitle);
        dialog.setLabelText(msg);
        dialog.setInputMode(QInputDialog::TextInput);
        dialog.setTextValue(defaultValue);
        dialog.setOkButtonText(okLabel);
        dialog.setCancelButtonText(cancelLabel);

        if (dialog.exec() == QDialog::Accepted) {
            *result = dialog.textValue();
            ok = true;
            return ok;
        }

        return ok;
    }

private:
    QWebView *webViewWidget;

    QString alertTitle;
    QString confirmTitle;
    QString promptTitle;

    QString okLabel;
    QString cancelLabel;
    QString yesLabel;
    QString noLabel;

    QString scriptFullFilePath;
    QProcess scriptHandler;
    QProcessEnvironment scriptEnvironment;
    QString scriptAccumulatedOutput;
    QString scriptAccumulatedErrors;

    QString userSelectedFileName;

#ifndef Q_OS_WIN
    QWebFrame *targetFrame;
    bool debuggerJustStarted;
    QString debuggerScriptToDebug;
    QString debuggerLastCommand;
    QProcess debuggerHandler;
    QString debuggerOutputFormatterScript;
    QProcess debuggerOutputHandler;
    QString debuggerAccumulatedOutput;
    QString debuggerAccumulatedHtmlOutput;
#endif
};

// ==============================
// WEB VIEW CLASS DEFINITION:
// ==============================
class QWebViewWidget : public QWebView
{
    Q_OBJECT

public slots:
    void qChangeTitleSlot()
    {
        setWindowTitle(QWebViewWidget::title());
    }

    void qSelectInodesSlot(QNetworkRequest request)
    {
        if (mainPage->currentFrame()->baseUrl()
                .toString().contains(PSEUDO_DOMAIN)) {
            QString target = request.url().query().replace("target=", "");

            QFileDialog inodesDialog (this);
            inodesDialog.setWindowModality(Qt::WindowModal);
            inodesDialog.setViewMode(QFileDialog::Detail);
            inodesDialog.setWindowTitle(QWebViewWidget::title());

            if (request.url().fileName() == "open-file.function") {
                inodesDialog.setFileMode(QFileDialog::AnyFile);
            }

            if (request.url().fileName() == "open-files.function") {
                inodesDialog.setFileMode(QFileDialog::ExistingFiles);
            }

            if (request.url().fileName() == "new-file-name.function") {
                inodesDialog.setAcceptMode(QFileDialog::AcceptSave);
            }

            if (request.url().fileName() == "open-directory.function") {
                inodesDialog.setFileMode(QFileDialog::Directory);
            }

            QStringList userSelectedInodes;
            if (inodesDialog.exec()) {
                userSelectedInodes = inodesDialog.selectedFiles();
            }

            inodesDialog.close();
            inodesDialog.deleteLater();

            if (!userSelectedInodes.isEmpty()) {
                QString userSelectedInodesFormatted;
                foreach (QString userSelectedInode, userSelectedInodes) {
                    userSelectedInodesFormatted.append(userSelectedInode);
                    userSelectedInodesFormatted.append(";");
                }
                userSelectedInodesFormatted
                        .replace(QRegularExpression(";$"), "");

                // JavaScript bridge back to
                // the local HTML page where request originated:
                mainPage->currentFrame()->
                        evaluateJavaScript(qApp->property("pebJS").toString());

                QString inodeSelectedEventJavaScript =
                        "pebInodeSelection(\"" +
                        target +
                        "\" , \"" +
                        userSelectedInodesFormatted +
                        "\"); null";

                mainPage->currentFrame()->
                        evaluateJavaScript(inodeSelectedEventJavaScript);

                qDebug() << "User selected inode:"
                         << userSelectedInodesFormatted;
            }
        } else {
            qDebug() << "Webpage attempted local full path selection:"
                     << mainPage->currentFrame()->baseUrl().toString();
        }
    }

    void contextMenuEvent(QContextMenuEvent *event)
    {
        QWebHitTestResult qWebHitTestResult =
                mainPage->mainFrame()->hitTestContent(event->pos());
        QMenu menu;

        QString printPreviewLabel;
        QString printLabel;

        QString cutLabel;
        QString copyLabel;
        QString pasteLabel;
        QString selectAllLabel;

        mainPage->currentFrame()->
                evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant contextMenuJsResult =
                mainPage->currentFrame()->
                evaluateJavaScript("pebFindContextMenu()");

        QJsonDocument contextMenuJsonDocument =
                QJsonDocument::fromJson(
                    contextMenuJsResult.toString().toUtf8());

        QJsonObject contextMenuJsonObject =
                contextMenuJsonDocument.object();

        if (contextMenuJsonObject.length() > 0) {
            if (contextMenuJsonObject["printPreview"].toString()
                    .length() > 0) {
                printPreviewLabel =
                        contextMenuJsonObject["printPreview"].toString();
            }

            if (contextMenuJsonObject["print"].toString().length() > 0) {
                printLabel = contextMenuJsonObject["print"].toString();
            }

            if (contextMenuJsonObject["cut"].toString().length() > 0) {
                cutLabel = contextMenuJsonObject["cut"].toString();
            }

            if (contextMenuJsonObject["copy"].toString().length() > 0) {
                copyLabel = contextMenuJsonObject["copy"].toString();
            }

            if (contextMenuJsonObject["paste"].toString().length() > 0) {
                pasteLabel = contextMenuJsonObject["paste"].toString();
            }

            if (contextMenuJsonObject["selectAll"].toString().length() > 0) {
                selectAllLabel = contextMenuJsonObject["selectAll"]
                        .toString();
            }
        } else {
            printPreviewLabel = "Print Preview";
            printLabel = "Print";

            cutLabel = "Cut";
            copyLabel = "Copy";
            pasteLabel = "Paste";
            selectAllLabel = "Select All";
        }

        if ((qWebHitTestResult.isContentEditable() and
                qWebHitTestResult.linkUrl().isEmpty() and
                qWebHitTestResult.imageUrl().isEmpty()) or
                qWebHitTestResult.isContentSelected()) {

            if (cutLabel.length() > 0) {
                QAction *cutAct = menu.addAction(cutLabel);
                QObject::connect(cutAct, SIGNAL(triggered()),
                                 this, SLOT(qCutAction()));
            }

            if (copyLabel.length() > 0) {
                QAction *copyAct = menu.addAction(copyLabel);
                QObject::connect(copyAct, SIGNAL(triggered()),
                                 this, SLOT(qCopyAction()));
            }

            if (pasteLabel.length() > 0) {
                QAction *pasteAct = menu.addAction(pasteLabel);
                QObject::connect(pasteAct, SIGNAL(triggered()),
                                 this, SLOT(qPasteAction()));
            }

            if (selectAllLabel.length() > 0) {
                QAction *selectAllAct = menu.addAction(selectAllLabel);
                QObject::connect(selectAllAct, SIGNAL(triggered()),
                                 this, SLOT(qSelectAllAction()));
            }
        }

        if (!qWebHitTestResult.isContentEditable() and
                qWebHitTestResult.linkUrl().isEmpty() and
                qWebHitTestResult.imageUrl().isEmpty() and
                (!qWebHitTestResult.isContentSelected())) {

            if (printPreviewLabel.length() > 0) {
                QAction *printPreviewAct = menu.addAction(printPreviewLabel);
                QObject::connect(printPreviewAct, SIGNAL(triggered()),
                                 this, SLOT(qStartPrintPreviewSlot()));
            }

            if (printLabel.length() > 0) {
                QAction *printAct = menu.addAction(printLabel);
                QObject::connect(printAct, SIGNAL(triggered()),
                                 this, SLOT(qPrintSlot()));
            }
        }

        menu.exec(mapToGlobal(event->pos()));
        this->focusWidget();
    }

    void qCutAction()
    {
        QWebViewWidget::triggerPageAction(QWebPage::Cut);
    }

    void qCopyAction()
    {
        QWebViewWidget::triggerPageAction(QWebPage::Copy);
    }

    void qPasteAction()
    {
        QWebViewWidget::triggerPageAction(QWebPage::Paste);
    }

    void qSelectAllAction()
    {
        QWebViewWidget::triggerPageAction(QWebPage::SelectAll);
    }

    void qStartPrintPreviewSlot()
    {
#ifndef QT_NO_PRINTER
        QPrinter printer(QPrinter::HighResolution);
        QPrintPreviewDialog preview(&printer, this);
        preview.setWindowModality(Qt::WindowModal);
        preview.setMinimumSize(QDesktopWidget()
                               .screen()->rect().width() * 0.8,
                               QDesktopWidget()
                               .screen()->rect().height() * 0.8);
        connect(&preview, SIGNAL(paintRequested(QPrinter*)),
                SLOT(qPrintPreviewSlot(QPrinter*)));
        preview.exec();
#endif
    }

    void qPrintPreviewSlot(QPrinter *printer)
    {
#ifdef QT_NO_PRINTER
        Q_UNUSED(printer);
#else
        QWebViewWidget::print(printer);
#endif
    }

    void qPrintSlot()
    {
#ifndef QT_NO_PRINTER
        qDebug() << "Printing requested.";

        QPrinter printer;
        QPrintDialog *printDialog = new QPrintDialog(&printer);
        printDialog->setWindowModality(Qt::WindowModal);
        QSize dialogSize = printDialog->sizeHint();
        QRect screenRect = QDesktopWidget().screen()->rect();
        printDialog->move(QPoint((screenRect.width() / 2)
                                 - (dialogSize.width() / 2),
                                 (screenRect.height() / 2)
                                 - (dialogSize.height() / 2)));
        if (printDialog->exec() == QDialog::Accepted) {
            QWebViewWidget::print(&printer);
        }
        printDialog->close();
        printDialog->deleteLater();
#endif
    }

    void qDisplayErrorsSlot(QString errors)
    {
        errorsWindow = new QWebViewWidget();
        errorsWindow->setHtml(errors, QUrl(PSEUDO_DOMAIN));
        errorsWindow->adjustSize();
        errorsWindow->setFocus();
        errorsWindow->show();
    }

    void qStartQWebInspector()
    {
        qDebug() << "QWebInspector started.";

        QWebInspector *inspector = new QWebInspector;
        inspector->setPage(QWebViewWidget::page());
        inspector->show();
    }

    void closeEvent(QCloseEvent *event)
    {
        if (!this->parentWidget() and windowCloseRequested == false) {
            if (mainPage->mainFrame()->childFrames().length() > 0) {
                foreach (QWebFrame *frame,
                         mainPage->mainFrame()->childFrames()) {
                    qCheckUserInputBeforeClose(frame, event);
                }
            } else {
                qCheckUserInputBeforeClose(mainPage->mainFrame(), event);
            }
        } else {
            event->accept();
        }
    }

    void qCheckUserInputBeforeClose(QWebFrame *frame, QCloseEvent *event)
    {
        frame->evaluateJavaScript(qApp->property("pebJS").toString());

        QVariant checkUserInputJsResult =
                frame->evaluateJavaScript("pebCheckUserInputBeforeClose()");
        bool textIsEntered = checkUserInputJsResult.toBool();

        QVariant checkCloseWarningJsResult =
                frame->evaluateJavaScript("pebCheckCloseWarning()");
        QString closeWarning = checkCloseWarningJsResult.toString();

        if (textIsEntered == false) {
            event->accept();
        }

        if (textIsEntered == true) {
            if (closeWarning == "async") {
                event->ignore();
                windowCloseRequested = true;
                frame->evaluateJavaScript("pebCloseConfirmationAsync()");
            }
            if (closeWarning == "sync") {
                QVariant jsSyncResult =
                        frame->evaluateJavaScript("pebCloseConfirmationSync()");

                bool jsCloseDecision;
                if (jsSyncResult.toString().length() > 0) {
                    jsCloseDecision = jsSyncResult.toBool();
                } else {
                    jsCloseDecision = true;
                }

                if (jsCloseDecision == true) {
                    event->accept();
                }
                if (jsCloseDecision == false) {
                    event->ignore();
                }
            }
        }
    }

    void qCloseWindowFromURLSlot()
    {
        if (!this->parentWidget()) {
            this->close();
        }

        if (this->parentWidget()) {
            qApp->setProperty("mainWindowCloseRequested", true);
            this->parentWidget()->close();
        }
    }

public:
    QWebViewWidget();

    QWebView *createWindow(QWebPage::WebWindowType type)
    {
        Q_UNUSED(type);

        QWebView *window = new QWebViewWidget();
        window->adjustSize();
        window->setFocus();

        qDebug() << "New window requested.";

        return window;
    }

private:
    QPage *mainPage;
    QWebView *newWindow;
    QWebView *errorsWindow;

    bool windowCloseRequested;
};

#endif // PEB_H
