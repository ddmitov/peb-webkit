  
Perl Executing Browser  
----------------------------------------------------------------------------------------
  
Perl Executing Browser (PEB) is a minimalistic C++ Qt5 WebKit graphical framework for local CGI-like or AJAX Perl5 scripts executed without a web server as desktop applications. Local scripts can be fed from HTML forms using CGI protocol GET and POST methods or using jQuery AJAX requests. HTML-based interface for interaction with the built-in Perl debugger is also available.  
  
## Design Objectives
  
* **1. Fast and easy graphical framework for desktop scripts:**  
    use Perl5, JavaScript, HTML5 and CSS to craft and deploy rapidly beautiful desktop applications;  

* **2. Zero installation when needed:**  
    put together your Perl5 scripts, modules and your version of Perl5 with a copy of PEB and its Qt5 libraries and run your applications from any folder;  

* **3. Cross-platform availability:**  
    use it on every platform, where Perl5, Qt5 and QtWebKit are available;  

* **4. Secure user-space solution:**  
    no daemons or services are installed or started, no privileged ports are opened, no firewall notifications are triggered and no need for administrative privileges to run the program;  

* **5. Maximal (re)use of existing web technologies and standards:**  
    use as much as possible from existing web technologies, standards and their documentation to build desktop applications.  

## Features
  
**No feature or implementation should be considered final at this early stage of development!**
  
**Local Scripting:**  
* CGI-like scripts can be executed locally in a serverless mode, feeding them from standard HTML forms using CGI protocol GET and POST methods.  
* jQuery AJAX requests to local scripts can also be made and all returned data can be seamlessly inserted into the DOM tree using standard jQuery methods.  
* Perl modules can be loaded from a custom directory without system-wide installation using PERLLIB environment variable.  
* Any version of Perl5 can be selected from configuration file.  
* Scripts and their HTML-based interfaces can be extracted and run from ZIP packages.  
  
**Web Access:**  
* PEB can open web pages with cross-site scripting disabled.  
  
**Configurability:**  
* All settings are stored in simple INI files.  
* Browser root folder can be any folder.  
* All browser functions are accessible from special URLs.  
* Use your favorite logo as a custom icon to be displayed on windows and message boxes.  
* 100% of the browser screen area are dedicated to your HTML interface.  
* Single-page or multiple-pages application with an option to start in fullscreen mode.  
  
**Development goodies:**  
* PEB can interact with the built-in Perl5 debugger. Any Perl script can be selected and loaded for debugging in an HTML graphical interface. Output from debugger commands is displayed together with the syntax highlighted source code of the debugged script and it's included modules. Interaction with the built-in Perl debugger is an idea proposed by Valcho Nedelchev.  
* WebKit Web Inspector can be invoked using keyboard shortcut.  
* Extensive optional logging of all browser activities, including the execution of local scripts.  

## Compile-time Requirements
  
GCC compiler and Qt5.1 - Qt5.5 headers (including QtWebKit headers).  
  
Compiled and tested successfully using:  
* Qt Creator 2.8.1 and Qt 5.1.1 on 32-bit Debian Linux,  
* Qt Creator 3.0.0 and Qt 5.2.0 on 32-bit Debian Linux,  
* Qt Creator 3.0.0 and Qt 5.2.0 on 32-bit Windows XP,  
* Qt Creator 3.0.1 and Qt 5.2.1 on 64-bit OS X 10.9.1, i5,  
(main development and testing platform - Valcho Nedelchev).  
* Qt Creator 3.1.1 and Qt 5.3.0 on 64-bit Lubuntu 14.10 Linux,
* Qt Creator 3.1.1 and Qt 5.4.1 on 64-bit Lubuntu 15.04 Linux  
(main development and testing platform - Dimitar D. Mitov).  
  
## Runtime Requirements
  
* Qt5 libraries,  
* Perl5 distribution - any standard Linux, Mac or Windows Perl distribution.  
  
## Target Audience
  
* Perl and JavaScript enthusiasts willing to create rapidly data-driven desktop applications with HTML4/5 & CSS2/3 GUI.  
* Perl developers willing to use the built-in Perl debugger in graphical mode.  
  
## Applications using Perl Executing Browser
  
* [Epigraphista](https://github.com/ddmitov/epigraphista) - Epigraphista is an EpiDoc XML file creator using Perl Executing Browser as a desktop GUI framework, HTML5 and Bootstrap for a themable user interface, JavaScript for on-screen text conversion and Perl5 for file-writing backend.  
  
## What Perl Executing Browser Is Not

* PEB is not a general purpose web browser and does not have all traditional features of general purpose web browsers.  
* PEB does not embed any Perl interpreter in itself and rellies on an external Perl distribution, which could be easily changed or upgraded independently if needed.  
* PEB has no sandbox for local Perl scripts. A work-in-progress security system is implemented in the ```censor.pl``` script (see below), which is created to protect local files from malicious or poorly written Perl scripts, but currently no claims are made for it's effectiveness and stability. It is still recommended to inspect your scripts before use for possible security vulnerabilities and best programming practices!  
* PEB is not an implementation of the CGI protocol. It uses only four environment variables (see below) together with the GET and POST methods from the CGI protocol in a purely local context without any attempt to communicate with the outside world.  
* Unlike JavaScript in general purpose web browsers, local Perl scripts executed by PEB have no access to the HTML DOM.  
  
## Security Features & Considerations
  
* Local scripts are executed with the minimum of necessary environment variables. These are:  
1) ```PERL5LIB``` - long-established Perl environment variable used to add Perl modules in non-standard locations;  
2) environment variables borrowed from the CGI protocol and used for finding local files and communication between HTML forms and local Perl scripts:  
```DOCUMENT_ROOT```, ```REQUEST_METHOD```, ```QUERY_STRING``` and ```CONTENT_LENGTH```;  
* Local scripts are executed in an ```eval``` function and only after banning of potentially unsafe core functions. This feature is implemented in a special script named ```censor.pl```. By default ```censor.pl``` is compiled in the resources of the browser binary and is executed from memory whenever a local Perl script is started. ```censor.pl``` can be turned off by a compile-time variable. Just change ```SCRIPT_CENSORING = 1``` to ```SCRIPT_CENSORING = 0``` in the project file of the browser (peb.pro) before compiling the binary.  
* Starting the browser as root on Linux is not allowed - it exits with a warning message. 
* PEB does not download locally executed scripts from any remote locations and it does not use any Perl interpreter as helper application for online content. This is not going to be implemented due to the huge security risks involved!  
* Users have no dialog to select arbitrary local scripts for execution by PEB - only scripts within the root folder of the browser can be executed if they are invoked from a special URL (currently ```http://perl-executing-browser-pseudodomain/```).  
* If user is not administrator of his/her machine and configuration file and root folder are owned by root/administrator and read-only for all others, user will be effectively prevented from executing untrusted code. Executing as root on a Linux machine:  
```chown --recursive root peb-root-folder```  
```chgrp --recursive root peb-root-folder```  
```chmod --recursive 755 peb-root-folder```  
is enough to do the job. The same commands could be applied to the folder of the binary file to prevent it's unauthorized replacing or modification. Locally executed scripts don't have to be made executable because they are always given as an argument to the interpreter, but mode 755 is necessary to avoid ```cannot read directory``` error.  
Essentially the same protection on a Windows(TM) machine could be achieved by installing PEB from the administrator's account in a location that is read-only for all other users.  
Note however, that a copy of PEB running from a flash drive or external harddisk and owned by an ordinary user will not have this extra protection.  
* Perl scripts, which are selected for debugging, are also executed and, in contrast with all other local scripts, there are no restrictions on which scripts could be debugged. This means that a potential security risk from a debugged Perl script does exist and if Perl debugger interaction is not needed, it can be turned off by a compile-time variable. Just change ```PERL_DEBUGGER_INTERACTION = 1``` to ```PERL_DEBUGGER_INTERACTION = 0``` in the project file of the browser (peb.pro) and compile the binary.  
* It is not a good idea to make any folders containing locally executed scripts available to web servers or file sharing applications due to the risk of executing locally malicious or unsecure code uploaded from outside. Securing configuration file and root folder as mentioned above should prevent file upload and modification, but will expose local files in read-only mode, which also has to be avoided.  
  
## Keyboard Shortcuts
* Ctrl+A - select all  
* Ctrl+C - copy  
* Ctrl+V - paste  
* F11 - toggle fullscreen  
* Alt+F4 - close current window  
* Ctrl+P - print current page  
* Ctrl+I - debug current page using QWebInspector  
  
## Limitation
  
* No reloading from JavaScript of a page that was produced by local script, but local static pages, as well as web pages, can be reloaded from JavaScript using ```location.reload()```.  
  
## History
  
PEB was started as a simple GUI for personal databases. Exhaustive documentation is still missing.  
  
## License
  
This program is free software;  
you can redistribute it and/or modify it under the terms of the GNU General Public License,  
as published by the Free Software Foundation; either version 3 of the License,  
or (at your option) any later version.  
This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;  
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
  
## Authors
  
Dimitar D. Mitov, 2013 - 2016,  
Valcho Nedelchev, 2014 - 2016.  
  
