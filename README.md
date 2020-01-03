# TheBViewerProject
The BViewer project is open source software provided by the National Institute for Occupational Safety and Health (NIOSH), a division of the Centers for Disease Control and Prevention (CDC).  To see how it fits into the NIOSH scheme of things, visit their website, https://www.cdc.gov/niosh/topics/chestradiography/digital-images.html

This project generates BViewer.exe and its BRetriever and ServiceController assistants for displaying chest X-Rays and supporting the diagnosing of lung disease by B reader radiologists.  For a description of BViewer, see the BViewerGP.chm help file in the \Project_Installation\Install\Files folder.  Also in this folder are the AboutBViewer.txt and the BViewerTechnicalRequirements.txt documents.  Each of the three executables is generated from a Microsoft Visual Studio "solution" file, for example, BViewer.sln.  These programs are coded in Visual C++ using the MFC class library for screen layout and OpenGL for the graphics rendering.  BRetriever runs in the background as a Windows service and does not have a GUI.  For a description of the project organization and building procedures, see the How to Build A BViewer Installation File.pdf file in the \Documentation folder.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See the deployment section below for notes on how to deploy the project on a live system.

Begin by cloning a local copy of TheBViewerProject from the GitHub repository.  Designate the base folder where you want the project files to be cloned.  The project files will be organized in the following file tree:

    TheBViewerProject
        Documentation
        Help_General_Purpose
        Help_NIOSH
        Project_Codebase
            BRetriever
            BViewer
            ServiceController
        Project_Installation
            Install
                Files
                Output

The two help folders supply the html and image files for generating the .chm help files for the BViewer general-purpose operation mode and for the NIOSH operation mode.  (The NIOSH operation mode is used for B readers participating in the NIOSH coal miners surveilance program.) The Project_Codebase folder contains the three Visual C++ projects, each of which generates a single .exe program file.  The Libraries folder contains projects for generating the image graphics libraries.  (This rarely needs to happen.)  The Project_Installation folder contains the Inno Setup installation scripts and the files to be included in the installation file compilation.  The Output folder will receive the completed BViewer installation file.

### Prerequisites

What things you need to build the project:

    * The current version of Microsoft's Visual Studio.  The free community edition is adequate for building BViewer.
    * Inno Setup, available for free downloading on the internet.  This program creates the .exe installation file from the files contained in the /Install/Files folder.
    * The antiquated Microsoft HTML Help Workshop software (hhw.exe) which is no longer supported, but is still available on the internet.  Otherwise, a modern HTML editor that can generate a .chm file.  This is only needed for changes to the help file.

## Building BViewer Executable Program Files

The procedures for building a BViewer installation are described in \Documentation\How to Build A BViewer Installation File.pdf.

## Deploying a BViewer Installation

To create a BViewer installation on a user workstation, just copy (or download) the installation file to the workstation and double-click on it.  The workstation requirements are described in \Project_Installation\Install\Files\BViewerTechnicalRequirements.txt.  BViewer can be operated using a single desktop display panel, but professional use requires two additional medical image displays capable of displaying 10-bit grayscale pixels.

Professional use also requires the installation of the set of standard reference images provided by the International Labor Organization (ILO).

Initial configuration of BViewer is described in the help file.  Before you begin diagnostic interpretation of chest images, you should go to the setup tab on the control panel and configure your displays, internet port, reader information, etc.

## Testing the Installation

To verify that a new BViewer installation is working, open the program.  Three test images that are included in the installation file are initially imported.  You can select any of these on the study selection tab.  You should see the image displayed on the display panel designated for study images.

Clicking an image on the blue standard reference image selector should result in a reference image displaying on the reference display panel.  Initially these reference images are dummy place holders.  For professional interpretations you need a licensed set of reference images from the International Labor Organization (ILO).

Full information on how to use BViewer for interpreting chest X-rays is contained in the help file.
