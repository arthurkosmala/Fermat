#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "potential.h"
#include "particlesource.h"
#include <iostream>
#include <qstring.h>
#include <sstream>
#include <vector>
#include <iostream>


// note: see mainwindow.h for further comments on this class
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    //********************************************************************
    //                              UI SETUP
    //********************************************************************
    //--------------------------------------------------------------------
    ui->setupUi(this);
    // set initial time label to 0s
    ui->tlabel->setText("t = 0 s");

    // set up a QTimer for the ray animation
    playBackTimer = new QTimer(this);
    connect(playBackTimer, SIGNAL(timeout()), this, SLOT(PlaybackStep()));

    // set up mouse input events
    connect(ui->plot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(displayValues(QMouseEvent*)));
    connect(ui->plot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(handleMouseClick(QMouseEvent*)));
    connect(ui->plot->xAxis,    SIGNAL(rangeChanged(QCPRange)), this,   SLOT(onXRangeChanged(QCPRange)));
    connect(ui->plot->yAxis,    SIGNAL(rangeChanged(QCPRange)), this,   SLOT(onYRangeChanged(QCPRange)));

    // text field for entering the refractive index function
    ui->ninput->setPlaceholderText("Enter refractive index");

    // the next part sets up the color map (showing the refractive index or potential)
    colorMap = new QCPColorMap(ui->plot->xAxis, ui->plot->yAxis);
    colorScale = new QCPColorScale(ui->plot);
    ui->plot->plotLayout()->addElement(0, 1, colorScale); // add it to the right of the main axis rect
    ui->plot->setCursor(QCursor(Qt::CrossCursor));
    colorScale->setType(QCPAxis::atRight); // scale shall be vertical bar with tick/axis labels right (actually atRight is already the default)
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    colorScale->axis()->setLabel("Refractive Index");
    QCPMarginGroup *marginGroup = new QCPMarginGroup(ui->plot);
    ui->plot->axisRect()->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    colorScale->setMarginGroup(QCP::msBottom|QCP::msTop, marginGroup);
    ui->plot->moveLayer(ui->plot->layer("grid"),ui->plot->layer("main"));

    //set up a layer above the color map for drawing the light rays
    ui->plot->addLayer("curveLayer", 0, QCustomPlot::limAbove);
    curveLayer = ui->plot->layer("curveLayer");
    curveLayer->setMode(QCPLayer::lmBuffered);

    // configure axis rect:
    ui->plot->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom); // this will also allow rescaling the color scale by dragging/zooming
    ui->plot->axisRect()->setupFullAxesBox(true);
    ui->plot->xAxis->setLabel("x");
    ui->plot->yAxis->setLabel("y");

    colorMap->data()->setSize(nx, ny);
    // set coordinate range for the color map
    colorMap->data()->setRange(QCPRange(coordrange[0], coordrange[1]), QCPRange(coordrange[2], coordrange[3]));
    // set color gradient to gpNight when in refractive index mode and to gpGeography when in mechanical potential mode
    colorMap->setGradient(QCPColorGradient::gpNight);
    colorMap->setColorScale(colorScale); // associate the color map with the color scale
    ui->plot->rescaleAxes();

    //--------------------------------------------------------------------

}


MainWindow::~MainWindow()
{
    delete ui;
}

//**************************************************************************************************************
/*The following functions (on_ninput_returnPressed(), handleStartButton(), handleResetButton(), handleVButton(),
                          displayValues(QMouseEvent*), handleMouseClick(QMouseEvent*),
                          onXRangeChanged(const QCPRange), onYRangeChanged(const QCPRange))
  are input slots -- for a quick overview of the UI elements they correspond to, see mainwindow.h
*/
//**************************************************************************************************************

void MainWindow::on_ninput_returnPressed(){
    std::stringstream ss;

    // ...well :)
    ss << "-1/2*(" << ui->ninput->text().toStdString() << ")^2";
    funcString = ss.str();
    plotV();
}

void MainWindow::handleStartButton(){
    resetSources();
    // In later versions, the number of iteration steps should be specifiable by the user.
    // For now, this number proved large enough so trajectories evolve to a considerable length,
    // but is also an acceptable upper bound on trajectory size
    calcHistories(20e4);
    animateHistories();
    ui->plot->replot();
    // start button gets greyed out until the trajectory animation is finished or reset by either
    // clicking the reset button, or entering a new refractive index function, or toggling vmode
    ui->startButton->setEnabled(false);
}

void MainWindow::handleResetButton(){
    playBackTimer->stop();
    resetSources();
    ui->plot->replot();
    ui->startButton->setEnabled(true);
}

void MainWindow::handleVButton(){
    // toggle between refractive index and potential mode
    if(vMode == false){
        vMode = true;
        colorScale->axis()->setLabel("Potential");
        // switch color gradient to gpGeography when changing to mechanical potential mode
        colorScale->setGradient(QCPColorGradient::gpGeography);
        ui->vnButton->setText("Show n");
    }
    else{
        vMode = false;
        colorScale->axis()->setLabel("Refractive Index");
        // switch color gradient to gpNight when changing to refractive index mode
        colorScale->setGradient(QCPColorGradient::gpNight);
        ui->vnButton->setText("Show V");
    }

    // Replot the potential with the new vmode setting
    plotV();
}

// this is called at any QMouseEvent
void MainWindow::displayValues(QMouseEvent * event)
{

    // update x and y coordinates of the curser and change the xcontent and ycontent labels
    double x = ui->plot->xAxis->pixelToCoord(event->pos().x());
    double y = ui->plot->yAxis->pixelToCoord(event->pos().y());
    double xypos [2]= {x,y};

    std::stringstream ssx;
    // round the position labels to four decimals
    ssx << (int)(x*10000.0+0.5)/10000.0;
    std::string sx = ssx.str();
    QString qsx = QString::fromStdString(sx);
    ui->xcontent->setText(qsx);

    std::stringstream ssy;
    ssy << (int)(y*10000.0+0.5)/10000.0;
    std::string sy = ssy.str();
    QString qsy = QString::fromStdString(sy);
    ui->ycontent->setText(qsy);

    // evaluate the potential value at the new cursor position
    std::stringstream ssv;
    ssv << (int)(V.Eval(xypos)*10000.0+0.5)/10000.0;
    std::string sv = ssv.str();
    QString qsv = QString::fromStdString(sv);
    ui->vcontent->setText(qsv);

    // evaluate the refractive index value at the new cursor position
    std::stringstream ssn;
    ssn << (int)(sqrt(-2*V.Eval(xypos))*10000.0+0.5)/10000.0;
    std::string sn = ssn.str();
    QString qsn = QString::fromStdString(sn);
    ui->ncontent->setText(qsn);
}

// this function draws/updates a force arrow at the cursor position at a right click if in vMode,
// and deletes a drawn force arrow (if present) at a left click
void MainWindow::handleMouseClick(QMouseEvent * event){
    if((event->button() == Qt::RightButton)&&(vMode == true)){

        double x = ui->plot->xAxis->pixelToCoord(event->pos().x());
        double y = ui->plot->yAxis->pixelToCoord(event->pos().y());
        // if there already is a force arrow, delete it
        if(fMode == true){
            ui->plot->removePlottable(arrowCurve);
            ui->plot->removePlottable(tipCurve);
        }
        arrowCurve = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);
        tipCurve = new QCPCurve(ui->plot->xAxis, ui->plot->yAxis);

        // divide the force vector (f_x, f_y) to max_force so the length of the drawn force vectors
        // cannot get arbitrarily large, but is limited to 1
        double f_x = V.get_force(x,y,dx)[0]/max_force;
        double f_y = V.get_force(x,y,dx)[1]/max_force;
        // first, draw the arrow line in black, then draw its tip (the last percent of the force arrow) in green
        QPen pen(Qt::black);
        arrowCurve->setPen(pen);
        arrowCurve->addData(0,x,y);
        arrowCurve->addData(1,x+0.99*f_x, y+0.99*f_y);
        QPen pen2(Qt::green);
        tipCurve->setPen(pen2);
        tipCurve->addData(0,x+0.99*f_x, y+0.99*f_y);
        tipCurve->addData(1,x+f_x,y+f_y);

        // for updating the force text field, we need to rescale f back to its actual length
        double f = sqrt(f_x*f_x+f_y*f_y)*max_force;
        std::stringstream ssf;
        // round to 3 decimals
        ssf << (int)(f*1000.0+0.5)/1000.0;
        std::string sf = ssf.str();
        QString qsf = QString::fromStdString(sf);
        ui->fcontent->setText(qsf);
        ui->plot->replot();
        // set fMode to true to indicate that there now is a drawn force arrow
        fMode = true;
    }

    // if there currently is a force arrow, delete it at left click
    if(event->button() == Qt::LeftButton){
        if (fMode == true){
            ui->plot->removePlottable(arrowCurve);
            ui->plot->removePlottable(tipCurve);
            ui->plot->replot();
            fMode = false;
        }
    }
}

// These two functions make the color map zoomable. Snippet taken from the QCP support forum post at
// qcustomplot.com/index.php/support/forum/713 (last checked August 18, 2020), provided by DerManu. Thanks! :)
void MainWindow::onXRangeChanged(const QCPRange &newRange)
{
    double lowerBound = coordrange[0];
    double upperBound = coordrange[1]; // note: the code assumes lowerBound < upperBound
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < lowerBound)
    {
      fixedRange.lower = lowerBound;
      fixedRange.upper = lowerBound + newRange.size();
      if (fixedRange.upper > upperBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.upper = upperBound;
      ui->plot->xAxis->setRange(fixedRange);
    } else if (fixedRange.upper > upperBound)
    {
      fixedRange.upper = upperBound;
      fixedRange.lower = upperBound - newRange.size();
      if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.lower = lowerBound;
      ui->plot->xAxis->setRange(fixedRange);
    }
}
// same as previously, but now for the y axis
void MainWindow::onYRangeChanged(const QCPRange &newRange)
{
    double lowerBound = coordrange[2];
    double upperBound = coordrange[3];
    QCPRange fixedRange(newRange);
    if (fixedRange.lower < lowerBound)
    {
      fixedRange.lower = lowerBound;
      fixedRange.upper = lowerBound + newRange.size();
      if (fixedRange.upper > upperBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.upper = upperBound;
      ui->plot->yAxis->setRange(fixedRange);
    } else if (fixedRange.upper > upperBound)
    {
      fixedRange.upper = upperBound;
      fixedRange.lower = upperBound - newRange.size();
      if (fixedRange.lower < lowerBound || qFuzzyCompare(newRange.size(), upperBound-lowerBound))
        fixedRange.lower = lowerBound;
      ui->plot->yAxis->setRange(fixedRange);
    }
}


//*****************************************************************************************************************
//Parse input for new refractive index distribution (or mechanical potential if vMode == true), plot new color map,
//calculate max force in the coordinate range to appropriately scale the force arrows
//*****************************************************************************************************************
void MainWindow::plotV(){
    V.Parse(funcString, "x,y");

    // delete the previously drawn light rays. Stop the ray animation in case it is currently running
    playBackTimer->stop();
    resetSources();
    ui->startButton->setEnabled(true);

    // assign the parsed function data to the color map
    double x, y, z;
    // reset max_force to 0 (back from the value it had for a previous potential)
    max_force = 0;
    for (int xIndex=0; xIndex<nx; ++xIndex)
    {
      for (int yIndex=0; yIndex<ny; ++yIndex)
      {
        colorMap->data()->cellToCoord(xIndex, yIndex, &x, &y);
        double pos [2]= {x,y};

        // draw potential if vMode == true, otherwise draw its corresponding refractive index distribution
        if (vMode == true)
            z = V.Eval(pos);
        else
            z = sqrt(-2*V.Eval(pos));

        /* At any point x,y, calculate the potential gradient numerically and check if its norm is greater than
           the current value of max_force */
        double f_x = V.get_force(x,y,dx)[0];
        double f_y = V.get_force(x,y,dx)[1];
        if(sqrt(f_x*f_x+f_y*f_y) > max_force){
            max_force = sqrt(f_x*f_x+f_y*f_y);
        }
        // handling the case that the potential is constant
        colorMap->data()->setCell(xIndex, yIndex, z);
      }
      if (max_force == 0) max_force = 1;
    }

    // rescale the data dimension (color) such that all data points lie in the span visualized by the color gradient:
    colorMap->rescaleDataRange(true);

    // rescale the key (x) and value (y) axes so the whole color map is visible:
    ui->plot->rescaleAxes();
    ui->plot->replot();

}


//*****************************************************************************************************************
//Clear all particle histories, plotted light ray curves and (if fMode == true) the drawn force arrow
//*****************************************************************************************************************
void MainWindow::resetSources(){
    // set time back to 0, as well as the time label (have to convert to QString)
    t = 0;
    std::stringstream ss;
    ss << "t = " << t << " s";
    std::string s = ss.str();
    QString qs = QString::fromStdString(s);
    ui->tlabel->setText(qs);

    // clear light ray plots
    for (std::vector<QCPCurve*>::iterator it = myCurves.begin() ; it != myCurves.end(); ++it){
        ui->plot->removePlottable(*it);
    }
    // also have to clear the myCurves vector
    myCurves.clear();
    // clearing the sources vector and then generating a source at the same position as previously is
    // necessary as we want to re-initialize the particles belonging to the source at their original positions
    sources.clear();
    double sourcePos [2] = {-7,0};
    ParticleSource def_src = ParticleSource(-7, 0, 0, 1, 1, -1*V.Eval(sourcePos), 50);
    sources.push_back(def_src);

    // delete a drawn force arrow
    if (fMode == true){
        ui->plot->removePlottable(arrowCurve);
        ui->plot->removePlottable(tipCurve);
        ui->plot->replot();
        fMode = false;
    }
}


//******************************************************************
//Calculate the histories of the particles belonging to each source.
//******************************************************************
//Currently, this function is a stub. It will, however, become useful once multiple
//user-positioned (and individually handled) sources become a feature.
void MainWindow::calcHistories(double steps){
    sources[0].propHistories(steps, V, dt, dx, coordrange);
}


//************************************************************************************************************
//Prepare the curve plots for each particle, determine the size of the longest trajectory, set off the QTimer.
//************************************************************************************************************
void MainWindow::animateHistories(){
    max_size = 0;
    for (int j = 0; j < (int)sources[0].myParticles.size(); j++){
        myCurves.push_back(new QCPCurve(ui->plot->xAxis, ui->plot->yAxis));
        myCurves[j]->setLayer("curveLayer");
        if (vMode == false){
        myCurves[j]->setPen(QPen(QColor( 0xD2, 0xB8, 0x00 )));
        }
        else{
        myCurves[j]->setPen(QPen(Qt::red));
        }

        /* The max_size variable is necessary since even though calcHistories is called with a fixed number of
           iteration steps (thus giving an upper bound on trajectory size), max_length will be shorter than that
           (and so the QTimer will already need to be stopped) if all particles leave the coordinate bounds at an
           earlier time. In fact, for the large current number of steps passed to calcHistories, this will usually
           be the case.
        */
        if(sources[0].myParticles[j].histx.size() > max_size){max_size = sources[0].myParticles[j].histx.size();}
    }
    // 20 milliseconds between each timer step
    playBackTimer->start(20);
}


//***********************************************************************************************************************************
//With each timer step, refresh the time label, add a new point from the particle history (timestep*dt later) to each light ray curve
//***********************************************************************************************************************************
void MainWindow::PlaybackStep()
{
    std::stringstream ss;
    ss << "t = " << t << " s";
    std::string s = ss.str();
    QString qs = QString::fromStdString(s);
    ui->tlabel->setText(qs);

    t += dt*timestep;
    // currently there is just one source stored in the sources vector, but this code can easily be extended to multiple ones
    for (int j = 0; j < (int)sources[0].myParticles.size(); j++){
        if(t/dt < sources[0].myParticles[j].histx.size()){
            myCurves[j]->addData(t/(dt*timestep),sources[0].myParticles[j].histx[t/dt],sources[0].myParticles[j].histy[t/dt]);
        }
    }
    curveLayer->replot();
    // stop the QTimer if the time exceeds the length of the longest particle history
    if(t/dt > max_size){
        playBackTimer->stop();
        ui->startButton->setEnabled(true);
    }
}
