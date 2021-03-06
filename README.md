# Automatic Intranet Login (AIL)
Version 0.1.0-6-g9c6a249


## WHAT IS THE AIL PROJECT ?

One of our past clients (*) wanted to allow users of Workstations to login automatically once they
logged into the Windows 7 workstation. The Windows 7 workstation passwords and the website passwords
are idential and managed via a central LDAP or Active Directory solution. This software caotures
given credentials via a custom windows credentials provider and uses this to automatically log into
the website via a browser of choice (determed upon install). The scripting for the browser access
etc. is flexible since its established via e.g. powershell or MS Windows Scripting host.

Once the user logs out the existing credentials are removed from disk. The script is automatically executed
on login and if needed by clicking on a shortcut icon.

In order to achieve this we utilize a freely available credentials provider from Tyler Wrightson [1][2]
with Visual Studio example [3][4].

(*) Project discontinued, client has no rights to source code


[1] http://twrightson.wordpress.com/2012/01/02/capturing-windows-7-credentials-at-logon-using-custom-credential-provider/
[2] archive/blog/blog.pdf
[3] http://www.leetsys.com/programs/credentialprovider/cp.zip
[4] http://www.leetsys.com/programs/credentialprovider/cp-devel.zip


## FEATURES

- Application
- Feature Providing Base Libraries
- Supported Ressources
  - MS Windows 7 Workstation (Target platform for package)
- Development
  - Visual Studio 2010
  - Install Jammer 1.2.15
- Development Base Libraries
- Code Quality
  - Code review


## ON WHAT HARDWARE DOES IT RUN?

This Software was originally developed and tested on 32-bit x86 / SMP based PCs running on MS
Windows, Ubuntu and Gentoo GNU/Linux 2.6.x. Other direct Linux and Unix derivates should be viable
too as long as all dynamical linking dependencys are met.


## DOCUMENTATION

A general developers API guide can be extracted from the Doxygen subdirectory which is able to
generate HTML as well as PDF docs. Please refer to the [Rake|Make]file for additional information
how to generate this documentation.


## INSTALLING

If you got this package as a packed tar.gz or tar.bz2 please unpack the contents in an appropriate
folder e.g. ~/myapp/ and follow the supplied INSTALL or README documentation. Please delete or
replace existing versions before unpacking/installing new ones.


## SOFTWARE REQUIREMENTS

This package was developed and compiled under MS Windows with Visual Studio 2010 and virtual
machines. Packaging is done via Install Jammer 1.2.15. Once packages no other software is needed
for execution except the running MS Windows 7 Workstation platform.


## BUILD PROCESS

Visual Studio building via sln file. Afterwards output is packaged via InstallJammer into executable.


## CONFIGURING

Direct changes in source code, no configuration files for now. Windows Scripting host file also
needs direct tweaking.


## RUNNING

Install bundle, afterwars after restart you should have the script being executed immediately after
login. Also, shortcuts should be available to force execution.


## IF SOMETHING GOES WRONG

In case you enconter bugs which seem to be related to the package please check in
the MAINTAINERS file for the associated person in charge and contact him or her directly. If
there is no valid address then try to mail Bjoern Rennhak <bjoern AT greylon DOT com> to get
some basic assistance in finding the right person in charge of this section of the project.


## CONTRIBUTING

- Fork it
- Create your feature branch (git checkout -b my-new-feature)
- Commit your changes (git commit -am 'Add some feature')
- Push to the branch (git push origin my-new-feature)
- Create new Pull Request

Be be prepared to release any rights of your changes to adhere to the project license.


## DEVELOPMENT

Version bumps follow semver [5].

[5] http://semver.org/

Readme generation is done by:

```
~# rvm --create use 2.1.0@ail_project
~# bundle
~# rake readme

```

## COPYRIGHT

Please refer to the COPYRIGHT file in the various folders for explicit copyright notice.  Unless
otherwise stated all remains protected and copyrighted by Bjoern Rennhak (bjoern AT greylon DOT com).

