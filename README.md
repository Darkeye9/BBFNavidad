#BBFNavidad

Small Automatic Christmas hat decorator. Used by the photo-booth at the BBF during christmas 2014.

##Compiling
This program uses [OpenCV 3.0.0 Beta](http://opencv.org/downloads.html). At the time of publishing there is NOT packages for common Linux distributions, but if you use Windows it should be all right.

This program uses [CMake](http://www.cmake.org/download/). It should not be difficult to find the appropriate packages for your OS.

Once all the dependencies have been met, it should be as easy as:

    cmake -D CMAKE_BUILD_TYPE=RELEASE .
    make
If you're using Windows, you are at your own. You'll have to create some Visual Studio project and import the code, or god knows what...

#Usage
Simply call the compiled program with the following sintax

    ./BBFnavidad PATH_TO_IMAGE [debug]

It will load the image selected by the path and after recognizing the faces present in the photo, it will print some nice christmas hats on top of their heads.

If you include the debug word as the second argument, boxes will be printed around each face, for debugging purposes.

_**Note**: As this program was built exclusively for the mentioned installation, it works best with input images of size 2592×1944, and it outputs the images to ./out, so make sure is writable._

#License
The project is licensed under the terms of the GNU GPL v3, with the following exceptions:
* data/haarcascades/* files are created by their respective authors and licensed under their own terms.
