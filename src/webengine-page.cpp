/*
 Perl Executing Browser

 This program is free software;
 you can redistribute it and/or modify it under the terms of the
 GNU Lesser General Public License,
 as published by the Free Software Foundation;
 either version 3 of the License, or (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY;
 without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE.
 Dimitar D. Mitov, 2013 - 2018
 Valcho Nedelchev, 2014 - 2016
 https://github.com/ddmitov/perl-executing-browser
*/

#include <QWebEngineSettings>
#include <QWebEngineProfile>

#include "webengine-page.h"

// ==============================
// LOCAL PAGE CLASS CONSTRUCTOR:
// (QTWEBENGINE VERSION)
// ==============================
QPage::QPage()
    : QWebEnginePage()
{
    // QWebPage settings:
    QWebEngineSettings::globalSettings()->
            setDefaultTextEncoding(QString("utf-8"));

    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);

    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::AutoLoadImages, true);
    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::FullScreenSupportEnabled, true);
    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::JavascriptEnabled, true);
    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls,
                         true);
    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::PluginsEnabled, true);
    QWebEngineSettings::globalSettings()->
            setAttribute(QWebEngineSettings::XSSAuditingEnabled, true);

    // Signal and slot for file downloads:
//    QObject::connect(QWebEngineProfile::defaultProfile(),
//                     SIGNAL(downloadRequested(QWebEngineDownloadItem*)),
//                     this,
//                     SLOT(qDownloadRequestedSlot(QWebEngineDownloadItem*)));

    // Signal and slot for actions taken after page is loaded:
    QObject::connect(this, SIGNAL(loadFinished(bool)),
                     this, SLOT(qPageLoadedSlot(bool)));

    // Signal and slot for closing window from JavaScript:
    QObject::connect(this, SIGNAL(windowCloseRequested()),
                     this, SLOT(qCloseAllScriptsSlot()));

    // Default dialog and context menu labels:
    okLabel = "Ok";
    cancelLabel = "Cancel";
    yesLabel = "Yes";
    noLabel = "No";

    // Close requested indicator:
    closeRequested = false;
}

// ==============================
// Special URLs handling:
// ==============================
bool QPage::acceptNavigationRequest(const QUrl &url,
                                    QWebEnginePage::NavigationType type,
                                    bool isMainFrame)
{
    if (url.scheme() == "file" and isMainFrame == true) {
        // Local forms submission:
        if (type == QWebEnginePage::NavigationTypeFormSubmitted) {
            if (url.fileName().contains(".settings")) {
                qHandleScripts(url.fileName().replace(".settings", ""));
                return false;
            } else {
                return false;
            }
        }

        if (type == QWebEnginePage::NavigationTypeLinkClicked) {
            // Handle filesystem dialogs:
            if (url.fileName().contains(".dialog")) {
                qHandleDialogs(url.fileName().replace(".dialog", ""));
                return false;
            }

            // Handle local Perl scripts:
            if (url.fileName().contains(".settings")) {
                qHandleScripts(url.fileName().replace(".settings", ""));
                return false;
            }

            // About browser dialog:
            if (url.fileName() == "about-browser.function") {
                QFileReader *resourceReader =
                        new QFileReader(QString(":/html/about.html"));
                QString aboutText = resourceReader->fileContents;

                aboutText.replace("APPLICATION_VERSION",
                                  QApplication::applicationVersion());
                aboutText.replace("QT_VERSION",
                                  QT_VERSION_STR);

                QPixmap icon(32, 32);
                icon.load(":/icon/camel.png");

                QMessageBox aboutBox;
                aboutBox.setWindowTitle ("About PEB");
                aboutBox.setIconPixmap(icon);
                aboutBox.setText(aboutText);
                aboutBox.setDefaultButton (QMessageBox::Ok);
                aboutBox.exec();

                return false;
            }

            // About Qt dialog:
            if (url.fileName() == "about-qt.function") {
                QApplication::aboutQt();
                return false;
            }
        }
    }

    return true;
}
