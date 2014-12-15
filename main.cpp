/*
BBFNavidad
Copyright (C) 2014 by  Iñigo 'Dark_eye' Ruiz (http://d-eye.eu)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <cstdlib>
#include <ctime>
#include <vector>
using namespace std;
using namespace cv;

//Configuraciones del banner
#define banner_size 1.75
#define banner_thick 2.3
#define banner_font 3
#define banner_color grey

//Tolerancia a falsos positivos
#define false_thres 2 // 200%

//Definiciones de colores varios
Scalar black = Scalar(0,0,0,1);
Scalar grey = Scalar(220,220,220,1);
Scalar blue = Scalar(0,0,255,1);
Scalar red = Scalar(255,0,0,1);
Scalar white = Scalar(255,255,255,1);

//Archivos de clasificadores en cascada
string cascadeName = "data/haarcascades/haarcascade_frontalface_alt.xml";
string nestedCascadeName = "data/haarcascades/haarcascade_eye.xml";

//Mostrar cuadros de debug?
bool debug=false;

void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                    double scale, vector<Mat> gorros);

//http://jepsonsblog.blogspot.com.es/2012/10/overlay-transparent-image-in-opencv.html
void overlayImage(const Mat &background, const Mat &foreground,
  Mat &output, Point2i location);

/*
 *
 */
int main(int argc, char** argv) {
    CascadeClassifier cascade, nestedCascade;

    srand ( unsigned ( time (NULL) ) );

#ifdef camera
    VideoCapture cap(0);
#endif

    //Escala para la imagen de reconocimiento
    double scale = 1;

#ifndef camera
    if ( argc < 2 || argc > 3 )
    {
        cout << "Uso: BBFNavidad <imagePath> [debug]" << endl;
        return -1;
    }

    if (argc > 2 && string(argv[2])== "debug")
    debug=true;
#else
    if (argc > 1 && string(argv[1])== "debug")
    debug=true;

#endif

    //Cargar el clasificador
    if( !cascade.load( cascadeName ) )
    {
        cerr << "ERROR: No se ha podido cargar el archivo del clasificador (" << cascadeName << ")" << endl;
        return -1;
    }

    //Cargar el clasificador anidado
    if( !nestedCascade.load( nestedCascadeName ) )
    {
        cerr << "ERROR: No se ha podido cargar el archivo del clasificador anidado (" << nestedCascadeName << ")" << endl;
        return -1;
    }

    //Cargar Imagen
    Mat image, gorror, gorrov, gorroa, gorroy, marco;
    double t = 0; // Variable de tiempos

    //Tomar referencia de tiempo
    t = (double)getTickCount();

    marco = imread( "data/images/frame.png", -1 );
    gorror = imread( "data/images/gorro.png", -1 );
    gorrov = imread( "data/images/gorrov.png", -1 );
    gorroa = imread( "data/images/gorroa.png", -1 );
    gorroy = imread( "data/images/gorroy.png", -1 );

#ifdef camera
         namedWindow("BBFNavidad",1);
        for(;;) {
        cap >> image;

        vector<Mat> gorros;
        gorros.push_back(gorroa);
        gorros.push_back(gorror);
        gorros.push_back(gorroy);
        gorros.push_back(gorrov);
        detectAndDraw( image, cascade, nestedCascade, scale, gorros);

        //Añadir marco
        //overlayImage(image,marco,image,Point(0,0));

        //Añadir Banner
        putText(image,"Creado con BBFNavidad 0.1",Point(20,image.size().height-20),
                banner_font,banner_size,banner_color, banner_thick, LINE_AA);

        imshow("BBFNavidad", image);
        if(waitKey(30) >= 0) break;
    }
#else
    image = imread( argv[1], 1 );

    if( !image.empty() && !marco.empty() && !gorror.empty())
    {
        vector<Mat> gorros;
        gorros.push_back(gorroa);
        gorros.push_back(gorrov);
        gorros.push_back(gorroy);
        gorros.push_back(gorror);

        detectAndDraw( image, cascade, nestedCascade, scale, gorros);

        //Añadir marco
       overlayImage(image,marco,image,Point(0,0));

        //Añadir Banner
        putText(image,"Creado con BBFNavidad 0.1",Point(20,image.size().height-20),
                banner_font,banner_size,banner_color, banner_thick, LINE_AA);

    }else
    {
        cerr << "ERROR: No se ha podido cargar la imagen (" << string(argv[1]) << ")" << endl;
        cerr << "Tambien puede ser que no se hallan podido acceder a las imagenes frame.png o gorro.png" << endl;
        return -1;
    }

    t = (double)getTickCount() - t;
    printf( "Detection process time = %g ms\n", t/((double)getTickFrequency()*1000.) );

    imwrite( "out/xmas_"+string(basename(argv[1])), image );
#endif
    return 0;
}

void detectAndDraw( Mat& img, CascadeClassifier& cascade, CascadeClassifier& nestedCascade,
                    double scale, vector<Mat> gorros)
{
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );
    vector<Rect> faces, faces2;  //Arays de caras
    double t = 0; // Variable de tiempos
    bool nestedDetected = false; //Utilizado para saber si algún ojo se ha detectado.

    //Convertir la imagen a escala de grises
    cvtColor(img, gray, COLOR_BGR2GRAY);

    //Redimensionar la imagen segun la escala
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );

    //Normalizar el histograma
    equalizeHist( smallImg, smallImg );

    //Tomar referencia de tiempo
    t = (double)getTickCount();

    //Realizar la detección
    cascade.detectMultiScale( smallImg, faces,
    1.1, 2, 0
    //|CASCADE_FIND_BIGGEST_OBJECT
    //|CASCADE_DO_ROUGH_SEARCH
    |CASCADE_SCALE_IMAGE
    ,
    Size(30, 30) );

    t = (double)getTickCount() - t;
    printf( "1st phase detection time = %g ms\n", t/((double)getTickFrequency()*1000.) );

    //Tomar referencia de tiempo
    t = (double)getTickCount();

    //Hallar las medias de las caras
    int avgarea=0;
    for ( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
    {
        avgarea+=r->area();
    }
    avgarea/=faces.size();

    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++)
    {
        vector<Rect> nestedObjects; // Array de ojos

        if (avgarea/r->area() > false_thres)
            continue;

        //Dibujar rectángulo
        if (debug)
       rectangle( img,
            Point(cvRound(r->x*scale), cvRound(r->y*scale)), //Vertice 1
            Point(cvRound((r->x + r->width-1)*scale), cvRound((r->y + r->height-1)*scale)), //Vertice 2
            blue, 3, LINE_AA, 0 //Color, grosor, linea, shift
            );

            Mat gorro = gorros.at(rand() % 4);

            float factor = ((float)r->size().width / (float)gorro.size().width)*1.40;
            Mat gorro2(cvRound(gorro.size().height*factor), cvRound(gorro.size().width*factor), CV_8UC4 );
            resize( gorro, gorro2, gorro2.size(), 0, 0, INTER_LINEAR );
            overlayImage(img,gorro2,img,Point(cvRound(r->x-gorro2.size().width*0.05),cvRound(r->y-gorro2.size().height*0.70)));
            nestedDetected=true;
    }
}

void overlayImage(const Mat &background, const Mat &foreground,
  Mat &output, Point2i location)
{
  background.copyTo(output);


  // start at the row indicated by location, or at row 0 if location.y is negative.
  for(int y = std::max(location.y , 0); y < background.rows; ++y)
  {
    int fY = y - location.y; // because of the translation

    // we are done of we have processed all rows of the foreground image.
    if(fY >= foreground.rows)
      break;

    // start at the column indicated by location,

    // or at column 0 if location.x is negative.
    for(int x = std::max(location.x, 0); x < background.cols; ++x)
    {
      int fX = x - location.x; // because of the translation.

      // we are done with this row if the column is outside of the foreground image.
      if(fX >= foreground.cols)
        break;

      // determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
      double opacity =
        ((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

        / 255.;


      // and now combine the background and foreground pixel, using the opacity,

      // but only if opacity > 0.
      for(int c = 0; opacity > 0 && c < output.channels(); ++c)
      {
        unsigned char foregroundPx =
          foreground.data[fY * foreground.step + fX * foreground.channels() + c];
        unsigned char backgroundPx =
          background.data[y * background.step + x * background.channels() + c];
        output.data[y*output.step + output.channels()*x + c] =
          backgroundPx * (1.-opacity) + foregroundPx * opacity;
      }
    }
  }
}
