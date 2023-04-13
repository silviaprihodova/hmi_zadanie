#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPainter>
#include <math.h>
#include <cmath>
#include <thread>
#include <chrono>

// PRIHODOVA S., SVEC D.

bool stop_switch = true;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)

{


    //tu je napevno nastavena ip. treba zmenit na to co ste si zadali do text boxu alebo nejaku inu pevnu. co bude spravna
    ipaddress="127.0.0.1";//192.168.1.11toto je na niektory realny robot.. na lokal budete davat "127.0.0.1"
  //  cap.open("http://192.168.1.11:8000/stream.mjpg");
    ui->setupUi(this);
    datacounter=0;
  //  timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), this, SLOT(getNewFrame()));
    actIndex=-1;
    useCamera1=false;

    datacounter=0;

}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::resizeEvent(QResizeEvent *event)
{
//    int height = this->size().height();
//    int width = this->size().width();


//    resize(width,width);
     if (hig_w != this->size().height()){
         hig_w = this->size().height();
         resize(hig_w*1.25,hig_w);
         wid_w = this->size().width();

     }else if (wid_w != this->size().width()){
         wid_w = this->size().width();
         resize(wid_w,wid_w*0.8);
         hig_w = this->size().height();

     }

}
void MainWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    ///prekreslujem obrazovku len vtedy, ked viem ze mam nove data. paintevent sa
    /// moze pochopitelne zavolat aj z inych dovodov, napriklad zmena velkosti okna
    painter.setBrush(Qt::black);//cierna farba pozadia(pouziva sa ako fill pre napriklad funkciu drawRect)
    QPen pero;
    pero.setStyle(Qt::SolidLine);//styl pera - plna ciara
    pero.setWidth(3);//hrubka pera -3pixely
    pero.setColor(Qt::red);//farba je zelena
    QRect rect;
    rect= ui->frame->geometry();//ziskate porametre stvorca,do ktoreho chcete kreslit
    rect.translate(0,15);
    painter.drawRect(rect);

    QRect rect3;
    rect3.setHeight(rect.height()/2);
    rect3.setWidth(rect.width()*0.75);
    rect3.translate(rect.topLeft().x() + rect.width()*0.25/2, rect.topLeft().y() + rect3.height()/2);

    QRect rect2;
    rect2= ui->frame_2->geometry();//ziskate porametre stvorca, do ktoreho chcete kreslit

    // zvascenie pri maximalizacii
    rect2.setHeight(rect.height()/3);
    rect2.setWidth(rect.width()/3);
    rect2.translate(rect.bottomRight().x() - rect2.width(), rect.bottomRight().y() - rect2.height());

    if( actIndex>-1)/// ak zobrazujem data z kamery a aspon niektory frame vo vectore je naplneny
    {
        QImage image = QImage((uchar*)frame[actIndex].data, frame[actIndex].cols, frame[actIndex].rows, frame[actIndex].step, QImage::Format_RGB888  );//kopirovanie cvmat do qimage
        painter.drawImage(rect,image.rgbSwapped());
    }


    int firstPoint = 1;

    if(updateLaserPicture==1) ///ak mam nove data z lidaru
    {
        updateLaserPicture=0;

        painter.setPen(pero);
        //teraz tu kreslime random udaje... vykreslite to co treba... t.j. data z lidaru
     //   std::cout<<copyOfLaserData.numberOfScans<<std::endl;
        for(int k=0;k<copyOfLaserData.numberOfScans/*360*/;k++)
        {
            int dist=copyOfLaserData.Data[k].scanDistance/(12000/(rect2.height()-60)-5); ///vzdialenost nahodne predelena 20 aby to nejako vyzeralo v okne.. zmen podla uvazenia
            int xp=rect2.width()-(rect2.width()/2+dist*2*sin((360.0-copyOfLaserData.Data[k].scanAngle)*3.14159/180.0))+rect2.topLeft().x(); //prepocet do obrazovky
            int yp=rect2.height()-(rect2.height()/2+dist*2*cos((360.0-copyOfLaserData.Data[k].scanAngle)*3.14159/180.0))+rect2.topLeft().y();//prepocet do obrazovky
            if(rect2.contains(xp,yp))//ak je bod vo vnutri nasho obdlznika tak iba vtedy budem chciet kreslit
                painter.drawEllipse(QPoint(xp, yp),2,2);

            // warning
            if ((copyOfLaserData.Data[k].scanDistance/10 < 30) && firstPoint == 1 && copyOfLaserData.Data[k].scanDistance != 0) // cca 30 polomer robota + 20 cm k prekazke warning
            {
                QFont font("Arial", 40);
                painter.setFont(font);
                 // vytvorenie QPainter objektu a nastavenie farby a priehľadnosti
                QColor color(255, 0, 0, 128); // nastavenie farby na červenú s priehľadnosťou 50%
                painter.setBrush(color);

                painter.drawRect(rect3);
                painter.setPen(Qt::white);
                painter.drawText(rect3, Qt::AlignCenter, "WARNING");
                firstPoint += 1;

            }

        }

        // vykreslenie robota

        painter.setPen(Qt::black);
        painter.setBrush(Qt::black);
        painter.drawEllipse(QPoint(rect2.x()+rect2.width()/2, rect2.y()+rect2.height()/2),4,4);


    }

//    if(updateSkeletonPicture==1 )
//    {
//        painter.setPen(Qt::red);
//        for(int i=0;i<75;i++)
//        {
//            int xp=rect.width()-rect.width() * skeleJoints.joints[i].x+rect.topLeft().x();
//            int yp= (rect.height() *skeleJoints.joints[i].y)+rect.topLeft().y();
//            if(rect.contains(xp,yp))
//                painter.drawEllipse(QPoint(xp, yp),2,2);
//        }
//    }

    painter.setPen(Qt::red);
    painter.setBrush(Qt::red);

    for(int k=0;k<copyOfLaserData.numberOfScans/*360*/;k++)
    {
        // orezanie, ale z nejakeho dovodu nefunguje, uz funguje
        if (copyOfLaserData.Data[k].scanAngle < 30.0  || copyOfLaserData.Data[k].scanAngle > 330.0) // rozsah 60 stupnov
        {
            float X = cos((360.0-copyOfLaserData.Data[k].scanAngle)*3.14159/180.0)*copyOfLaserData.Data[k].scanDistance/10; // uhol pre lavotocivy lidar
            float Y = sin((360.0-copyOfLaserData.Data[k].scanAngle)*3.14159/180.0)*copyOfLaserData.Data[k].scanDistance/10;
            float Zd = -14.5, Z = 21, Yd = 11.5;
            float Xobr = 853.0/2 - ((681.743*Y)/(X + Zd));
            float Yobr = 480.0/2 + ((681.743*(-Z + Yd))/(X + Zd));

            // pomer obrazka kamery ku vykreslovanim bodom
            float Xpomer = rect.width()/853.0;
            float Ypomer = rect.height()/480.0;
            Xobr *= Xpomer;
            Yobr *= Ypomer;

            // posunutie
            Xobr += rect.topLeft().x();
            Yobr += rect.topLeft().y();

            if (rect.contains(Xobr, Yobr))
                painter.drawEllipse(QPoint(Xobr, Yobr),4,4);


           // printf("%f\n",copyOfLaserData.Data[k].scanAngle);
        }

    }

}


/// toto je slot. niekde v kode existuje signal, ktory je prepojeny. pouziva sa napriklad (v tomto pripade) ak chcete dostat data z jedneho vlakna (robot) do ineho (ui)
/// prepojenie signal slot je vo funkcii  on_pushButton_9_clicked
void  MainWindow::setUiValues(double robotX,double robotY,double robotFi)
{

}

///toto je calback na data z robota, ktory ste podhodili robotu vo funkcii on_pushButton_9_clicked
/// vola sa vzdy ked dojdu nove data z robota. nemusite nic riesit, proste sa to stane
int MainWindow::processThisRobot(TKobukiData robotdata)
{



    ///tu mozete robit s datami z robota
    /// ale nic vypoctovo narocne - to iste vlakno ktore cita data z robota
    ///teraz tu posielam rychlosti na zaklade toho co setne joystick a vypisujeme data z robota(kazdy 5ty krat. ale mozete skusit aj castejsie). vyratajte si polohu. a vypiste spravnu
    /// tuto joystick cast mozete vklude vymazat,alebo znasilnit na vas regulator alebo ake mate pohnutky... kazdopadne, aktualne to blokuje gombiky cize tak
//    if(forwardspeed==0 && rotationspeed!=0)
//        robot.setRotationSpeed(rotationspeed);
//    else if(forwardspeed!=0 && rotationspeed==0)
//        robot.setTranslationSpeed(forwardspeed);
//    else if((forwardspeed!=0 && rotationspeed!=0))
//        robot.setArcSpeed(forwardspeed,forwardspeed/rotationspeed);
//    else
//        robot.setTranslationSpeed(0);


///TU PISTE KOD... TOTO JE TO MIESTO KED NEVIETE KDE ZACAT,TAK JE TO NAOZAJ TU. AK AJ TAK NEVIETE, SPYTAJTE SA CVICIACEHO MA TU NATO STRING KTORY DA DO HLADANIA XXX

    return 0;

}

///toto je calback na data z lidaru, ktory ste podhodili robotu vo funkcii on_pushButton_9_clicked
/// vola sa ked dojdu nove data z lidaru
int MainWindow::processThisLidar(LaserMeasurement laserData)
{


    memcpy( &copyOfLaserData,&laserData,sizeof(LaserMeasurement));
    //tu mozete robit s datami z lidaru.. napriklad najst prekazky, zapisat do mapy. naplanovat ako sa prekazke vyhnut.
    // ale nic vypoctovo narocne - to iste vlakno ktore cita data z lidaru

    updateLaserPicture=1;
    update();//tento prikaz prinuti prekreslit obrazovku.. zavola sa paintEvent funkcia


    return 0;

}

///toto je calback na data z kamery, ktory ste podhodili robotu vo funkcii on_pushButton_9_clicked
/// vola sa ked dojdu nove data z kamery
int MainWindow::processThisCamera(cv::Mat cameraData)
{

    cameraData.copyTo(frame[(actIndex+1)%3]);//kopirujem do nasej strukury
    actIndex=(actIndex+1)%3;//aktualizujem kde je nova fotka
    updateLaserPicture=1;

    return 0;
}

///toto je calback na data zo skeleton trackera, ktory ste podhodili robotu vo funkcii on_pushButton_9_clicked
/// vola sa ked dojdu nove data z trackera
int MainWindow::processThisSkeleton(skeleton skeledata)
{

    memcpy(&skeleJoints,&skeledata,sizeof(skeleton));

    updateSkeletonPicture=1;

    if(stop_switch == true){
        switch (detectGestures()) {
        case LIKE:

            robot.setTranslationSpeed(200);
          //  cout <<"like"<< endl;
            break;
        case DISLIKE:
            robot.setTranslationSpeed(-200);
         //    cout <<"dislike"<< endl;
            break;

        case ROTATE_R:
            robot.setRotationSpeed(-3.14159/8);
            break;
        case ROTATE_L:
            robot.setRotationSpeed(3.14159/8);
            break;
        case STOP:
            robot.setTranslationSpeed(0);
            break;
        default:
            break;
        }
    }
    return 0;
}
void MainWindow::on_pushButton_9_clicked() //start button
{

    forwardspeed=0;
    rotationspeed=0;
    //tu sa nastartuju vlakna ktore citaju data z lidaru a robota
    connect(this,SIGNAL(uiValuesChanged(double,double,double)),this,SLOT(setUiValues(double,double,double)));

    ///setovanie veci na komunikaciu s robotom/lidarom/kamerou.. su tam adresa porty a callback.. laser ma ze sa da dat callback aj ako lambda.
    /// lambdy su super, setria miesto a ak su rozumnej dlzky,tak aj prehladnost... ak ste o nich nic nepoculi poradte sa s vasim doktorom alebo lekarnikom...
    robot.setLaserParameters(ipaddress,52999,5299,/*[](LaserMeasurement dat)->int{std::cout<<"som z lambdy callback"<<std::endl;return 0;}*/std::bind(&MainWindow::processThisLidar,this,std::placeholders::_1));
    robot.setRobotParameters(ipaddress,53000,5300,std::bind(&MainWindow::processThisRobot,this,std::placeholders::_1));
    //---simulator ma port 8889, realny robot 8000
    robot.setCameraParameters("http://"+ipaddress+":8889/stream.mjpg",std::bind(&MainWindow::processThisCamera,this,std::placeholders::_1));
    robot.setSkeletonParameters("127.0.0.1",23432,23432,std::bind(&MainWindow::processThisSkeleton,this,std::placeholders::_1));
    ///ked je vsetko nasetovane tak to tento prikaz spusti (ak nieco nieje setnute,tak to normalne nenastavi.cize ak napr nechcete kameru,vklude vsetky info o nej vymazte)
    robot.robotStart();

    //ziskanie joystickov
    instance = QJoysticks::getInstance();


    /// prepojenie joysticku s jeho callbackom... zas cez lambdu. neviem ci som to niekde spominal,ale lambdy su super. okrem toho mam este rad ternarne operatory a spolocneske hry ale to tiez nikoho nezaujima
    /// co vas vlastne zaujima? citanie komentov asi nie, inak by ste citali toto a ze tu je blbosti
    connect(
        instance, &QJoysticks::axisChanged,
        [this]( const int js, const int axis, const qreal value) { if(/*js==0 &&*/ axis==1){forwardspeed=-value*300;}
            if(/*js==0 &&*/ axis==0){rotationspeed=-value*(3.14159/2.0);}}
    );
}

void MainWindow::on_pushButton_2_clicked() //forward
{
    //pohyb dopredu
    robot.setTranslationSpeed(500);

}

void MainWindow::on_pushButton_3_clicked() //back
{
    robot.setTranslationSpeed(-250);

}

void MainWindow::on_pushButton_6_clicked() //left
{
robot.setRotationSpeed(3.14159/2);

}

void MainWindow::on_pushButton_5_clicked()//right
{
robot.setRotationSpeed(-3.14159/2);

}

void MainWindow::on_pushButton_4_clicked() //stop
{
    if(stop_switch == true){
         stop_switch = false;
    }else{
         stop_switch = true;
    }

    robot.setTranslationSpeed(0);


}




void MainWindow::on_pushButton_clicked() // use camera
{
//    if(useCamera1==true)
//    {
//        useCamera1=false;

//        ui->pushButton->setText("use camera");
//    }
//    else
//    {
//        useCamera1=true;

//        ui->pushButton->setText("use laser");
//    }
}

void MainWindow::getNewFrame()
{

}

int MainWindow::detectGestures()
{
    // like
     if (skeleJoints.joints[left_thumb_tip].y < skeleJoints.joints[left_index_ip].y) {

         if (skeleJoints.joints[left_index_ip].y < skeleJoints.joints[left_middle_ip].y) {

             if (skeleJoints.joints[left_middle_ip].y < skeleJoints.joints[left_ring_ip].y) {

                 if (skeleJoints.joints[left_ring_ip].y < skeleJoints.joints[left_pink_ip].y ){
                    if (skeleJoints.joints[left_index_tip].x > skeleJoints.joints[left_index_ip].x ){
                        if (skeleJoints.joints[left_middle_tip].x > skeleJoints.joints[left_middle_ip].x ){

                            if (skeleJoints.joints[left_ringy_tip].x > skeleJoints.joints[left_ring_ip].x ){

                                if (skeleJoints.joints[left_pink_tip].x > skeleJoints.joints[left_pink_ip].x ){
                                    if (skeleJoints.joints[left_thumb_tip].y < skeleJoints.joints[left_index_mcp].y){

                                     return LIKE;
                                    }
                                }
                          }
                    }
                }
             }
           }
        }
     }


    // dislike
    if (skeleJoints.joints[left_thumb_tip].y > skeleJoints.joints[left_index_ip].y) {

     if (skeleJoints.joints[left_index_ip].y > skeleJoints.joints[left_middle_ip].y) {

         if (skeleJoints.joints[left_middle_ip].y > skeleJoints.joints[left_ring_ip].y) {

             if (skeleJoints.joints[left_ring_ip].y > skeleJoints.joints[left_pink_ip].y ){
                 //new
                 if (skeleJoints.joints[left_index_tip].x > skeleJoints.joints[left_index_ip].x ){
                     if (skeleJoints.joints[left_middle_tip].x > skeleJoints.joints[left_middle_ip].x ){

                         if (skeleJoints.joints[left_ringy_tip].x > skeleJoints.joints[left_ring_ip].x ){

                             if (skeleJoints.joints[left_pink_tip].x > skeleJoints.joints[left_pink_ip].x ){
                                if (skeleJoints.joints[left_thumb_tip].y > skeleJoints.joints[left_index_mcp].y){
                                        return DISLIKE;
                                    }

                            }
                         }
                     }
                 }
            }
       }
    }
    }

    // rotate right, three fingers
    if (skeleJoints.joints[left_thumb_tip].y < skeleJoints.joints[left_thumb_ip].y){

     if (skeleJoints.joints[left_index_tip].y < skeleJoints.joints[left_index_ip].y){

         if (skeleJoints.joints[left_middle_tip].y < skeleJoints.joints[left_middle_ip].y){

             if (skeleJoints.joints[left_ringy_tip].y > skeleJoints.joints[left_ring_ip].y){

                 if (skeleJoints.joints[left_pink_tip].y > skeleJoints.joints[left_pink_ip].y){

                     if (skeleJoints.joints[left_thumb_tip].x < skeleJoints.joints[left_index_tip].x){

                                 return ROTATE_R;

                   }

               }

           }

       }
    }
}
    // rotate left, one finger
     if (skeleJoints.joints[left_index_tip].y < skeleJoints.joints[left_index_ip].y){

         if (skeleJoints.joints[left_middle_tip].y > skeleJoints.joints[left_middle_ip].y){
            if (skeleJoints.joints[left_thumb_tip].x < skeleJoints.joints[left_index_tip].x){
                if (skeleJoints.joints[left_ringy_tip].y > skeleJoints.joints[left_ring_ip].y){

                    if (skeleJoints.joints[left_pink_tip].y > skeleJoints.joints[left_pink_ip].y){
                     return ROTATE_L;
                    }
                }
            }
       }
    }

     // stop, five fingers
     if (skeleJoints.joints[left_middle_tip].y < skeleJoints.joints[left_middle_ip].y) {

      if (skeleJoints.joints[left_middle_tip].y < skeleJoints.joints[left_pink_tip].y) {

          if (skeleJoints.joints[left_pink_tip].y < skeleJoints.joints[left_pink_ip].y) {

              if (skeleJoints.joints[left_pink_tip].y < skeleJoints.joints[left_thumb_tip].y ){

                  if (skeleJoints.joints[left_thumb_tip].y < skeleJoints.joints[left_wrist].y ){

                      return STOP;
              }

          }
        }
     }
     }

     return 0;
}


