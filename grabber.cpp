#include "grabber.h"
#include "ui_grabber.h"
#include <QDir>
#include <QDirIterator>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QTime>
#include <QBarSeries>
#include <QChart>
#include <QBarSet>
#include <QBarCategoryAxis>
#include <QValueAxis>
#include "login.h"
#include <QRect>
#include <QFileDialog>
#include "options.h"
#include <QMessageBox>
#include <QSettings>
#include <QShortcut>
#include <QKeySequence>
#include <QObject>
#include <QBitmap>

Grabber::Grabber(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Grabber)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint);
    ui->dateEdit->setDate(QDate::currentDate());

    //init. options
    optionDlg= new options();

    if (!::paths.valid)
    {
        optionDlg->exec();
    }

    //init. login
    login Dlg(0);
    setPalette(Dlg.palette());
    qApp->setPalette(Dlg.palette());
    Dlg.setModal(true);
    Dlg.exec();
    ui->activeusrEdit->setText(Dlg.getusr());

    // use CTRL+ALT+Q to close CTGrabber.exe
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+Alt+Q"), this);
    QObject::connect(shortcut, SIGNAL(activated()), qApp, SLOT(quit()));

    //load "speicher.txt"
    load();
    
    // init. minimize to systray
    sysIcon= new QSystemTrayIcon(QIcon(":/img/spy.png"));
    QMenu * menu = new QMenu();
    menu->addAction("show", this, SLOT(show()));
    sysIcon->setContextMenu(menu);
    sysIcon->show();
    
    //show logo
    QPixmap aPixmap(":/img/spy.png");
    aPixmap = aPixmap.scaledToWidth(121);
    QBitmap bmp=aPixmap.createHeuristicMask();
    ui->piclabel->setMask(bmp);
    ui->piclabel->setPixmap(aPixmap);

    //init. chartviews
    motimeview=new QChartView(ui->motimetab);
    motimeview->setGeometry(5,5,557,314);
    motimeview->show();
    
    momemoryview=new QChartView(ui->momemorytab);
    momemoryview->setGeometry(5,5,557,314);
    momemoryview->show();
    
    yetimeview=new QChartView(ui->yetimetab);
    yetimeview->setGeometry(5,5,557,314);
    yetimeview->show();
    
    yememoryview=new QChartView(ui->yememorytab);
    yememoryview->setGeometry(5,5,557,314);
    yememoryview->show();
    //init. user specific chartviews
    umotimeview=new QChartView(ui->umotimetab);
    umotimeview->setGeometry(5,5,557,314);
    umotimeview->show();

    umomemoryview=new QChartView(ui->umomemorytab);
    umomemoryview->setGeometry(5,5,557,314);
    umomemoryview->show();

    uyetimeview=new QChartView(ui->uyetimetab);
    uyetimeview->setGeometry(5,5,557,314);
    uyetimeview->show();

    uyememoryview=new QChartView(ui->uyememorytab);
    uyememoryview->setGeometry(5,5,557,314);
    uyememoryview->show();


    //init. timer for searchDir
    searchDir();
    timer = new QTimer();
    timer->setInterval(120000);
    timer->setSingleShot(false);
    connect(timer,SIGNAL(timeout()),this,SLOT(searchDir()));
    timer->start();


}

Grabber::~Grabber()
{
    delete ui;
}

void Grabber::closeEvent(QCloseEvent * ce)
{
    ce->ignore();
}

void Grabber::keyPressEvent(QKeyEvent * ke)
{
    if(ke->key() == Qt::Key_F4 && ke->modifiers() == Qt::AltModifier)
            ke->accept();
    else
        QWidget::keyPressEvent(ke);
}


fileInfo::fileInfo(QTextStream & t)
{
    bname=t.readLine();
    user=t.readLine();
    pname=t.readLine();
    created= QDate::fromString(t.readLine(),"yyyy.MM.dd");
    ctime= QTime::fromString(t.readLine(),"hh:mm:ss");
    fsize=t.readLine().toFloat();
    scantime=t.readLine().toFloat();
    kV=t.readLine().toInt();
    uA=t.readLine().toInt();
    FOV=t.readLine().toInt();
}

void fileInfo::save(QTextStream & t)
{
    t << bname << endl;
    t << user << endl;
    t << pname << endl;
    t << created.toString("yyyy.MM.dd") << endl;
    t << ctime.toString("hh:mm:ss") << endl;
    t << fsize << endl;
    t << scantime << endl;
    t << kV << endl;
    t << uA << endl;
    t << FOV << endl;
}

QString fileInfo::toString()
{
    QString s;
    s += bname+" ";
    s += user+" ";
    s += pname+" ";
    s += created.toString("yyyy.MM.dd")+" ";
    s += ctime.toString("hh:mm:ss")+" ";
    s += QString("%1").arg(fsize)+" ";
    s += QString("%1").arg(scantime)+" ";
    s += QString("%1").arg(kV)+" ";
    s += QString("%1").arg(uA)+" ";
    s += QString("%1").arg(FOV);
    return s;
}

void Grabber::save()
{
    QFile f(::paths.txtDir + "\\speicher.txt");
    if (f.open(QFile::WriteOnly))
    {
        QTextStream t(&f);
        t << dataBase.count() << endl;

        for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
        {
            t << it.key() << endl;
            it->save(t);
        }
        f.close();
    }
}

void Grabber::load()
{
    QFile f(::paths.txtDir + "\\speicher.txt");
    if (f.open(QFile::ReadOnly))
    {
        QTextStream t(&f);
        dataBase.clear();
        long nr;
        nr = t.readLine().toLong();
        for (long i=0; i<nr; i++)
        {
            QString key = t.readLine();
            dataBase[key] = fileInfo(t);
        }
        f.close();
        for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
        {
            ui->infoBrowser->append(it->toString());
        }
    }
}


void Grabber::searchDir()
{
    QDirIterator it(::paths.dataDir, QStringList() << "*.VOX", QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext())
    {
       QFileInfo info(it.next());
       ui->infoBrowser->append(info.absoluteFilePath());


       if (!dataBase.contains(info.baseName()))
       {
           dataBase[info.baseName()].bname=info.baseName();
           dataBase[info.baseName()].user=ui->activeusrEdit->text();
           dataBase[info.baseName()].pname=info.absolutePath().section("/",-2,-2);
           QString s2= info.baseName().mid(3,15);
           QString year= s2.left(4);
           QString month= s2.mid(4,2);
           QString day= s2.mid(6,2);
           QString hour= s2.mid(9,2);
           QString min= s2.mid(11,2);
           QString sec= s2.right(2);
           QTime ti(hour.toInt(),min.toInt(),sec.toInt());
           dataBase[info.baseName()].created.setDate(year.toInt(),month.toInt(),day.toInt());
           dataBase[info.baseName()].ctime=ti;
           float fround=info.size();
           fround=qRound(fround/1000000);
           dataBase[info.baseName()].fsize=fround;

           QString infofname = info.absolutePath()+"\\DataInfo.xml";
           QFile f(infofname);
           if (f.open(QFile::ReadOnly))
           {
               QTextStream t(&f);
               QString s;

               while (!t.atEnd())
               {
                   s=t.readLine();
                   if (s.contains("<ScanTime>"))
                   {
                       s=s.section("<",1,1).section(">",1,1);
                       float s1=0;
                       if (s.contains("2min"))
                       {
                       s1=2;
                       }
                       if (s.contains("4.5min"))
                       {
                       s1=4.5;
                       }
                       if (s.contains("34sec"))
                       {
                       s1=0.566;
                       }
                       if (s.contains("17sec"))
                       {
                       s1=0.283;
                       }
                       dataBase[info.baseName()].scantime=s1;
                   }
                   if (s.contains("KV"))
                   {
                       s=s.section("<",1,1).section(">",1,1);
                       int s2=0;
                       s2=s.toInt();
                       dataBase[info.baseName()].kV=s2;
                   }
                   if (s.contains("UA"))
                   {
                       s=s.section("<",1,1).section(">",1,1);
                       int s3=0;
                       s3=s.toInt();
                       dataBase[info.baseName()].uA=s3;
                   }
                   if (s.contains("FOV"))
                   {
                       s=s.section("<",1,1).section(">",1,1);
                       int s4=0;
                       s4=s.toInt();
                       dataBase[info.baseName()].FOV=s4;
                   }
               }
            }
        }
    save();
    }
}

void Grabber::plot()
{

    QBarSeries * series = new QBarSeries();
    QMap<QString,float> motiMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.month()==plotDate.month() && it->created.year()== plotDate.year())
        {
            motiMap.contains(it->pname) ? motiMap[it->pname]+=it->scantime : motiMap[it->pname]=it->scantime;
        }
    }
    QStringList categories;
    float motisum=0;
    QBarSet * motibar= new QBarSet("");
    for (QMap<QString,float>::iterator it=motiMap.begin(); it!=motiMap.end(); it++)
    {
        motisum+=(*it);
        *motibar << (*it);
        series->append(motibar);
        categories << it.key();
    }

    *motibar << motisum;
    categories << "sum";
    series->append(motibar);
    QChart * chart = new QChart();
    motimeview->setChart(chart);
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->legend()->setVisible(false);
    chart->setTitle("scantime @ " + plotDate.toString("MMMM yyyy"));
    motibar->setBorderColor("black");
    motibar->setBrush(Qt::Dense2Pattern);
    motibar->setColor("red");
    QBarCategoryAxis *axisX = new QBarCategoryAxis();
    axisX->setLabelsAngle(90);
    axisX->append(categories);
    chart->setAxisX(axisX);
    QValueAxis *axisy =new QValueAxis();
    axisy->setLabelFormat("%.1f min");
    axisy->setTitleText("time");
    axisy->setTickType(QValueAxis::TicksFixed);
    axisy->setRange(0,motisum);
    chart->setAxisY(axisy);

//TAB2-----------------------------------------------------------------------------------------------------------------------------------------
    QBarSeries * series2 = new QBarSeries();
    QMap<QString,float> momeMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.month()==plotDate.month() && it->created.year()== plotDate.year())
        {
            momeMap.contains(it->pname) ? momeMap[it->pname]+=it->fsize : momeMap[it->pname]=it->fsize;
        }
    }
    QStringList categories2;
    float momesum=0;
    QBarSet * momebar= new QBarSet("");
    for (QMap<QString,float>::iterator it=momeMap.begin(); it!=momeMap.end(); it++)
    {
        momesum+=(*it);
        *momebar << (*it);
        series2->append(momebar);
        categories2 << it.key();
    }

    *momebar << momesum;
    categories2 << "sum";
    series2->append(momebar);
    QChart * chart2 = new QChart();
    momemoryview->setChart(chart2);
    chart2->addSeries(series2);
    chart2->createDefaultAxes();
    chart2->legend()->setVisible(false);
    chart2->setTitle("memory used @ "+ plotDate.toString("MMMM yyyy"));
    momebar->setBorderColor("black");
    momebar->setBrush(Qt::Dense2Pattern);
    momebar->setColor("blue");
    QBarCategoryAxis *axisX2 = new QBarCategoryAxis();
    axisX2->setLabelsAngle(90);
    axisX2->append(categories2);
    chart2->setAxisX(axisX2);
    QValueAxis *axisy2 =new QValueAxis();
    axisy2->setLabelFormat("%.1f mB");
    axisy2->setTitleText("memory");
    axisy2->setTickType(QValueAxis::TicksFixed);
    axisy2->setRange(0,momesum);
    chart2->setAxisY(axisy2);

//TAB 3----------------------------------------------------------------------------------------------------------
    QBarSeries * series3 = new QBarSeries();
    QMap<QString,float> yetiMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.year() ==  plotDate.year())
        {
            yetiMap.contains(it->pname) ? yetiMap[it->pname]+=it->scantime : yetiMap[it->pname]=it->scantime;
        }
    }
    float yetisum=0;
    QStringList categories3;
    QBarSet * yetibar= new QBarSet("");
    for (QMap<QString,float>::iterator it=yetiMap.begin(); it!=yetiMap.end(); it++)
    {
        yetisum+=(*it);
        *yetibar << (*it);
        series3->append(yetibar);
        categories3 << it.key();
    }
    *yetibar << yetisum;
    categories3 << "sum";
    series3->append(yetibar);
    QChart * chart3 = new QChart();
    yetimeview->setChart(chart3);
    chart3->addSeries(series3);
    chart3->createDefaultAxes();
    chart3->legend()->setVisible(false);
    chart3->setTitle("scantime @ " + plotDate.toString("yyyy"));
    yetibar->setBorderColor("black");
    yetibar->setBrush(Qt::Dense2Pattern);
    yetibar->setColor("green");
    QBarCategoryAxis *axisX3 = new QBarCategoryAxis();
    axisX3->setLabelsAngle(90);
    axisX3->append(categories3);
    chart3->setAxisX(axisX3);
    QValueAxis *axisy3 =new QValueAxis();
    axisy3->setLabelFormat("%.1f min");
    axisy3->setTitleText("time");
    axisy3->setTickType(QValueAxis::TicksFixed);
    chart3->setAxisY(axisy3);
    axisy3->setRange(0,yetisum);


//TAB 4---------------------------------------------------------------------------------------------------
    QBarSeries * series4 = new QBarSeries();
    QMap<QString,float> yemeMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if ( it->created.year() == plotDate.year())
        {
            yemeMap.contains(it->pname) ? yemeMap[it->pname]+=it->fsize : yemeMap[it->pname]=it->fsize;
        }
    }
    float yemesum=0;
    QStringList categories4;
    QBarSet * yemebar= new QBarSet("");
    for (QMap<QString,float>::iterator it=yemeMap.begin(); it!=yemeMap.end(); it++)
    {
        yemesum+=(*it);
        *yemebar << (*it);
        series4->append(yemebar);
        categories4 << it.key();
    }
    *yemebar << yemesum;
    categories4 << "sum";
    series4->append(yemebar);
    QChart * chart4 = new QChart();
    yememoryview->setChart(chart4);
    chart4->addSeries(series4);
    chart4->createDefaultAxes();
    chart4->legend()->setVisible(false);
    chart4->setTitle("memory used @ " + plotDate.toString("yyyy"));
    yemebar->setBorderColor("black");
    yemebar->setBrush(Qt::Dense2Pattern);
    yemebar->setColor("orange");
    QBarCategoryAxis *axisX4 = new QBarCategoryAxis();
    axisX4->setLabelsAngle(90);
    axisX4->append(categories4);
    chart4->setAxisX(axisX4);
    QValueAxis *axisy4 =new QValueAxis();
    axisy4->setLabelFormat("%.1f mB");
    axisy4->setTitleText("memory");
    axisy4->setTickType(QValueAxis::TicksFixed);
    axisy4->setRange(0,yemesum);
    chart4->setAxisY(axisy4);

 //USER TABS
 //User tab1---------------------------------------------------------------------------------------------------------------------------------------------------------
    QBarSeries * seriesu1 = new QBarSeries();
    QMap<QString,float> umotiMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.month() == plotDate.month() && it->created.year() == plotDate.year())
        {
            umotiMap.contains(it->user) ? umotiMap[it->user]+=it->scantime : umotiMap[it->user]=it->scantime;
        }
    }
    QStringList categoriesu1;
    float umotisum=0;
    QBarSet * umotibar= new QBarSet("");
    for (QMap<QString,float>::iterator it=umotiMap.begin(); it!=umotiMap.end(); it++)
    {
        umotisum+=(*it);
        *umotibar << (*it);
        seriesu1->append(umotibar);
        categoriesu1 << it.key();
    }

    *umotibar << umotisum;
    categoriesu1 << "sum";
    seriesu1->append(umotibar);
    QChart * chartu1 = new QChart();
    umotimeview->setChart(chartu1);
    chartu1->addSeries(seriesu1);
    chartu1->createDefaultAxes();
    chartu1->legend()->setVisible(false);
    chartu1->setTitle("scantime @ " + plotDate.toString("MMMM yyyy"));
    umotibar->setBorderColor("black");
    umotibar->setBrush(Qt::Dense3Pattern);
    umotibar->setColor("red");
    QBarCategoryAxis *axisXu1 = new QBarCategoryAxis();
    axisXu1->setLabelsAngle(90);
    axisXu1->append(categoriesu1);
    chartu1->setAxisX(axisXu1);
    QValueAxis *axisyu1 =new QValueAxis();
    axisyu1->setLabelFormat("%.1f min");
    axisyu1->setTitleText("time");
    axisyu1->setTickType(QValueAxis::TicksFixed);
    axisyu1->setRange(0,umotisum);
    chartu1->setAxisY(axisyu1);

//USERTAB2-----------------------------------------------------------------------------------------------------------------------------------------
    QBarSeries * seriesu2 = new QBarSeries();
    QMap<QString,float> umomeMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.month()==plotDate.month() && it->created.year()== plotDate.year())
        {
            umomeMap.contains(it->user) ? umomeMap[it->user]+=it->fsize : umomeMap[it->user]=it->fsize;
        }
    }
    QStringList categoriesu2;
    float umomesum=0;
    QBarSet * umomebar= new QBarSet("");
    for (QMap<QString,float>::iterator it=umomeMap.begin(); it!=umomeMap.end(); it++)
    {
        umomesum+=(*it);
        *umomebar << (*it);
        seriesu2->append(umomebar);
        categoriesu2 << it.key();
    }

    *umomebar << umomesum;
    categoriesu2 << "sum";
    seriesu2->append(umomebar);
    QChart * chartu2 = new QChart();
    umomemoryview->setChart(chartu2);
    chartu2->addSeries(seriesu2);
    chartu2->createDefaultAxes();
    chartu2->legend()->setVisible(false);
    chartu2->setTitle("memory used @ "+ plotDate.toString("MMMM yyyy"));
    umomebar->setBorderColor("black");
    umomebar->setBrush(Qt::Dense3Pattern);
    umomebar->setColor("blue");
    QBarCategoryAxis *axisXu2 = new QBarCategoryAxis();
    axisXu2->setLabelsAngle(90);
    axisXu2->append(categoriesu2);
    chartu2->setAxisX(axisXu2);
    QValueAxis *axisyu2 =new QValueAxis();
    axisyu2->setLabelFormat("%.1f mB");
    axisyu2->setTitleText("memory");
    axisyu2->setTickType(QValueAxis::TicksFixed);
    axisyu2->setRange(0,umomesum);
    chartu2->setAxisY(axisyu2);

//USERTAB 3----------------------------------------------------------------------------------------------------------
    QBarSeries * seriesu3 = new QBarSeries();
    QMap<QString,float> uyetiMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if (it->created.year() ==  plotDate.year())
        {
            uyetiMap.contains(it->user) ? uyetiMap[it->user]+=it->scantime : uyetiMap[it->user]=it->scantime;
        }
    }
    float uyetisum=0;
    QStringList categoriesu3;
    QBarSet * uyetibar= new QBarSet("");
    for (QMap<QString,float>::iterator it=uyetiMap.begin(); it!=uyetiMap.end(); it++)
    {
        uyetisum+=(*it);
        *uyetibar << (*it);
        seriesu3->append(uyetibar);
        categoriesu3 << it.key();
    }
    *uyetibar << uyetisum;
    categoriesu3 << "sum";
    seriesu3->append(uyetibar);
    QChart * chartu3 = new QChart();
    uyetimeview->setChart(chartu3);
    chartu3->addSeries(seriesu3);
    chartu3->createDefaultAxes();
    chartu3->legend()->setVisible(false);
    chartu3->setTitle("scantime @ " + plotDate.toString("yyyy"));
    uyetibar->setBorderColor("black");
    uyetibar->setBrush(Qt::Dense3Pattern);
    uyetibar->setColor("green");
    QBarCategoryAxis *axisXu3 = new QBarCategoryAxis();
    axisXu3->setLabelsAngle(90);
    axisXu3->append(categoriesu3);
    chartu3->setAxisX(axisXu3);
    QValueAxis *axisyu3 =new QValueAxis();
    axisyu3->setLabelFormat("%.1f min");
    axisyu3->setTitleText("time");
    axisyu3->setTickType(QValueAxis::TicksFixed);
    chartu3->setAxisY(axisyu3);
    axisyu3->setRange(0,uyetisum);


//USERTAB 4---------------------------------------------------------------------------------------------------
    QBarSeries * seriesu4 = new QBarSeries();
    QMap<QString,float> uyemeMap;
    for (QMap<QString,fileInfo>::iterator it=dataBase.begin(); it!=dataBase.end(); it++)
    {
        if ( it->created.year() ==  plotDate.year())
        {
            uyemeMap.contains(it->user) ? uyemeMap[it->user]+=it->fsize : uyemeMap[it->user]=it->fsize;
        }
    }
    float uyemesum=0;
    QStringList categoriesu4;
    QBarSet * uyemebar= new QBarSet("");
    for (QMap<QString,float>::iterator it=uyemeMap.begin(); it!=uyemeMap.end(); it++)
    {
        uyemesum+=(*it);
        *uyemebar << (*it);
        seriesu4->append(uyemebar);
        categoriesu4 << it.key();
    }
    *uyemebar << uyemesum;
    categoriesu4 << "sum";
    seriesu4->append(uyemebar);
    QChart * chartu4 = new QChart();
    uyememoryview->setChart(chartu4);
    chartu4->addSeries(seriesu4);
    chartu4->createDefaultAxes();
    chartu4->legend()->setVisible(false);
    chartu4->setTitle("memory used @ " + plotDate.toString("yyyy"));
    uyemebar->setBorderColor("black");
    uyemebar->setBrush(Qt::Dense3Pattern);
    uyemebar->setColor("orange");
    QBarCategoryAxis *axisXu4 = new QBarCategoryAxis();
    axisXu4->setLabelsAngle(90);
    axisXu4->append(categoriesu4);
    chartu4->setAxisX(axisXu4);
    QValueAxis *axisyu4 =new QValueAxis();
    axisyu4->setLabelFormat("%.1f mB");
    axisyu4->setTitleText("memory");
    axisyu4->setTickType(QValueAxis::TicksFixed);
    axisyu4->setRange(0,uyemesum);
    chartu4->setAxisY(axisyu4);

}



void Grabber::on_minButton_clicked()
{
    hide();
}



void Grabber::on_setButton_clicked()
{
    long updateTime = abs(ui->setintEdit->time().msecsTo(QTime(0,0,0)));
    timer->setInterval(updateTime);
    ui->infoBrowser->append(QString("New Update Interval: %1ms").arg(updateTime));

}

void Grabber::on_plotButton_clicked()
{
    plot();
}

void Grabber::on_exportButton_clicked()
{
    QPixmap expfile;
    QRect frame(500, 35, 571, 326);
    expfile = QWidget::grab(frame);
    QString fname=QFileDialog::getSaveFileName(this, tr("save chart")," ", tr("Images(*.png)"));
    if (!fname.isEmpty())
        expfile.save(fname, "PNG");
}

void Grabber::on_logoutButton_clicked()
{
    hide();
    login Dlg(0);
    Dlg.setModal(true);
    Dlg.exec();
    ui->activeusrEdit->setText(Dlg.getusr());
    show();
}


void Grabber::on_optionsButton_clicked()
{
    if (ui->activeusrEdit->text() == "admin")
    {
        optionDlg->setModal(true);
        optionDlg->exec();
    }
    else
        QMessageBox::critical(0,"ERROR","need admin privileges to access this function!");
}

void Grabber::on_dateEdit_userDateChanged(const QDate &date)
{
    plotDate=date;
}
