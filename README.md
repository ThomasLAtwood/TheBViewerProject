# TheBViewerProject
This project generates BViewer.exe and its BRetriever and ServiceController assistants for displaying chest X-Rays and supporting the diagnosing of lung disease by B reader radiologists.  For a description of BViewer, see the BViewerGP.chm help file in the \Project_Installation\Install\Files folder.  Also in this folder are the AboutBViewer.txt and the BViewerTechnicalRequirements.txt documents.  Each of the three executables is generated from a Microsoft Visual Studio "solution" file, for example, BViewer.sln.  These programs are coded in Visual C++ using the MFC class library for screen layout and OpenGL for the graphics rendering.  For a description of the project organization and building procedures, see the How to Build A BViewer Installation File.pdf file in the \Documentation folder.

## Getting Started
These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

Begin by cloning a local copy of TheBViewerProject from the GitHub repository.  Designate the base folder where you want the project files to be cloned.  The project files will be organized in the following file tree:

    TheBViewerProject
        Documentation
        Help_General_Purpose
        Help_NIOSH
        Project_Codebase
            BRetriever
            BViewer
            ServiceController
            Libraries
                Jpeg8
                Jpeg12
                Jpeg16
                libpng
        Project_Installation
            Install
                Files
                Output

The two help folders supply the html and image files for generating the .chm help files forthe BViewer general-purpose operation mode and for the NIOSH operation mode.  The Project_Codebase folder contains the three Visual C++ projects, each of which generates a single .exe program file.  The Libraries folder contains projects for generating the image graphics libraries.  (This rarely needs to happen.)  The Project_Installation folder contains the Inno Setup installation scripts and the files to be included in the installation file compilation.  The Output folder will receive the completed BViewer installation file.

### Prerequisites

What things you need to build the project:

    * The current version of Microsoft's Visual Studio.  The free community edition is adequate for building BViewer.
    * Inno Setup, available for free downloading on the internet.
    * The antiquated Microsoft HTML Help Workshop software (hhw.exe) which is no longer supported, but is still available on the internet.  Otherwise, a modern HTML editor that can generate a .chm file.  This is only needed for changes to the help file.

## Building BViewer Executable Program Files

The procedures for building a BViewer installation are described in \Documentation\How to Build A BViewer Installation File.pdf.

## Deploying a BViewer Installation

To create a BViewer installation on a user workstation, just copy (or download) the installation file to the workstation and double-click on it.  The workstation requirements are described in \Project_Installation\Install\Files\BViewerTechnicalRequirements.txt.  BViewer can be operated using a single desktop display panel, but professional use requires two additional medical image displays capable of displaying 10-bit grayscale pixels.

Initial configuration of BViewer is described in the help file.  Before you begin diagnostic interpretation of chest images, you should go to the setup tab on the control panel and configure your displays, internet port, reader information, etc.

## Testing the Installation

To verify that a new BViewer installation is working, open the program.  Three test images that are included in the installation file are initially imported.  You can select any of these on the study selection tab.  You should see the image displayed on the display panel designated for study images.

Clicking an image on the blue standard reference image selector should result in a reference image displaying on the reference display panel.  Initially these reference images are dummy place holders.  For professional interpretations you need a licensed set of reference images from the International Labor Organization (ILO).

What follows is a template that will gradually be replaced with the corresponding information as time permits.
______________________________________________________________________________________________________



And coding style tests

Explain what these tests test and why

Give an example

Deployment

Add additional notes about how to deploy this on a live system
Built With

    Dropwizard - The web framework used
    Maven - Dependency Management
    ROME - Used to generate RSS Feeds

Contributing

Please read CONTRIBUTING.md for details on our code of conduct, and the process for submitting pull requests to us.
Versioning

We use SemVer for versioning. For the versions available, see the tags on this repository.
Authors

    Billie Thompson - Initial work - PurpleBooth

See also the list of contributors who participated in this project.
License

This project is licensed under the MIT License - see the LICENSE.md file for details
Acknowledgments

    Hat tip to anyone whose code was used
    Inspiration
    etc
