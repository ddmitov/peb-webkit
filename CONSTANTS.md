Perl Executing Browser - Constants
--------------------------------------------------------------------------------

## Hard Coded Files and Folders
* **Perl interpreter:**  
  PEB expects to find Perl interpreter in ``{PEB_binary_directory}/perl/bin`` folder. The interpreter must be named ``perl`` on Linux and Mac machines and ``perl.exe`` on Windows machines. If Perl interpreter is not found in the above location, PEB will try to find the first Perl interpreter on PATH. If no Perl interpreter is found, an error page is displayed instead of the start page. No Perl interpreter is a showstopper for PEB.

* **Application directory:**  
  Application directory is ``{PEB_binary_directory}/resources/app``.  
  All files used by PEB, with the exception of data files, should be located within this folder.  

  Application directory is hard coded in C++ code for compatibility with the [Electron](http://electron.atom.io/) framework.  
  [Epigraphista](https://github.com/ddmitov/epigraphista) is an example of a PEB-based application, that is also compatible with [Electron](http://electron.atom.io/) and [NW.js](http://nwjs.io/).  

  By default the working directory of all Perl scripts run by PEB is the application directory.

* **Data Directory:**
  Data directory should contain any files used or produced by a PEB-based application.  
  The data directory path is ``{PEB_binary_directory}/resources/data``.  
  Perl scripts can access this folder using the environment variable ``PEB_DATA_DIR``:

  ```perl
  my $data_directory = $ENV{'PEB_DATA_DIR'};
  ```

* **Start page:**  
  PEB starts always with ``{PEB_binary_directory}/resources/app/index.html``. If this file is missing, an error message is displayed. No start page is a showstopper for PEB.  
  Note that start page pathname is case sensitive.

  <a name="icon"></a>
* **Icon:**
  A PEB-based application can have its own icon and it must be located at ``{PEB_binary_directory}/resources/app/app.png``. If this file is found during application startup, it is used as the icon of the application and all dialog boxes. If this file is not found, the default icon embedded into the resources of the browser binary is used.

## Functional Pseudo Filenames
* **About PEB dialog:** ``about-browser.function``

* **About Qt dialog:** ``about-qt.function``

## Specific Keyboard Shortcuts
All specific keyboard shortcuts are available only in the QtWebKit builds of PEB.
* <kbd>Ctrl</kbd> + <kbd>I</kbd> - start QWebInspector
* <kbd>Ctrl</kbd> + <kbd>P</kbd> - get printer selection dialog. If no printer is configured, no dialog is displayed.
* <kbd>Ctrl</kbd> + <kbd>R</kbd> - get print preview