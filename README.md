# Delsgolf

Delsgolf is a First Person Shooter for the TI-89 Calculator.

# How to build

Build environment used for development was Windows with Cygwin/GCC. You also need TIGCC for building the calculator assembly application.
Additional libraries required are libpng and for the editor an older version of gtk.  

*Build Texconv*  
Go into ./texconv and execute make.sh, copy texconv.exe to ./data.

*Generate the Image Data*  
Go into ./data and execute gendata.bat, look for errors, if successful copy texture.h to ./datastore

*Generate the Game Data File*  
Go into ./datastore and execute make.sh, execute datastore.exe which will create the game data file in little and big endian (eng_le/be.dat), execute enggendata.bat (requires a TIGCC tool) which will generate the TI-89 game data variable, upload the delsrepo.89y variable to the calculator.

*Building the TI-89 Calculator assembly application*  
Go into ./delsgolf, execute make.sh (requires TIGCC). This will build the calculator executable delsgolf.89z. Upload that to your TI-89 Calculator.

# The Windows Version

There is an old Visual Studio eng_win32.sln solution in the ./delsgolf directory. It may or may not work. The Windows specific code is somwhat ancient too but it will probably still work. The Windows version looks for the little endian game data file eng_le.dat in the working directory.

# The Map Editor

The ./editor directory contains the Map Editor which requires GTK to build. It is **very** hard and cumbersome to use. It can be used to convert the .map files from ./data to little and big endian compiled map files .dat in ./datastore. I supplied the pre-compiled maps there already.


# If you should get it running

Have Fun !  

Keys:  
ESC        - Menu  
2nd        - Menu Select / Fire  
Arrow Keys - Movement  
X          - Pistol  
Y          - Rifle  
Z          - Use Button  
  
# If building does not work

You can check [this page](https://www.ticalc.org/archives/files/fileinfo/457/45704.html) for some screenshots and binary downloads.