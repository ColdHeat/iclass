# iclass

In order to read and write to cards, you will, at the minimum, need a reader/writer. The simplest way to get one is to get a reader/writer from the HID OMNIKEY line. Either the HID OMNIKEY 5321 or 6321 will work and both are fairly cheap at around $50. The model number does not matter very much, contrary to what you may think. I used an HID OMNIKEY 6321 CLi and an HID OMNIKEY 5321 v2 CLi.

To build the software you want to start off by downloading the [MinGW installer assistant](https://sourceforge.net/projects/mingw/files/Installer/mingw-get-setup.exe/download). Install it and select:

* mingw-developer-toolkit
* mingw32-gcc-g++
* msys-base

It should look something like this:

![mingw](https://cloud.githubusercontent.com/assets/166333/15988849/91724b5c-302d-11e6-994c-33d24211e87e.png)

Then go to `Installation > Apply Changes` on the menu bar to install part of your build environment. 

Now go to `C:\MinGW\msys\1.0` and copy everything over to `C:\MinGW`. 

![copying](https://cloud.githubusercontent.com/assets/166333/15988850/98ea89a8-302d-11e6-9620-c5b45406ff87.png)

Open the `msys.bat` file to get a small shell environment. Clone the provided source code into the home folder, go into the `iclassified` directory, and run `make`.

If everything runs well you should get `iclass.exe` and `iclassified.o`

At this point you should plug in your OMNIKEY reader and follow the instructions provided alongside [the drivers](http://www.proxmark.org/files/Various%20Hardware/OMNIKEY%205x21/OMNIKESY5x21_V1_2_0_14.exe) to get the reader setup. 

If all goes well you should be able to execute `iclass.exe read` to read a card and `iclass.exe write` to write to a card.

![writing](https://cloud.githubusercontent.com/assets/166333/15988852/a08fa5d0-302d-11e6-99c5-3b80d4a7d195.png)
