#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include "potential.h"
#include "particlesource.h"
#include <vector>

/*
--------------------------------------------------------------------------------------------------------------------
The MainWindow class ...
... manages all user input, ...
... plots the color map displaying the refractive index or potential (depending on the chosen program mode), ...
... coordinates the calculation and visualization of light beams in the specified refractive index distribution.
--------------------------------------------------------------------------------------------------------------------

**********
VARIABLES
**********

-- vMode : bool, specifies whether the program is in refractive index or mechanical potential mode

-- fMode : bool, toggles the presence of force arrows showing the potential gradient at the mouse position

-- dx : double, spatial discretization stepsize for numerical computation of the potential gradient

-- dt : double, time stepsize for trajectory calculation with the RK4 numerical integrator

-- t : double, time variable for the light ray trajectories
   (note: this variable has physical meaning as "time" only from the classical mechanics, not the optics viewpoint)

-- timestep : double, timestep factor for the light ray animation in units of dt

-- max_size : double, size of the longest of all the particle history arrays

-- max_force : double, stores the maximum potential gradient in the coordinate range

-- ui : MainWindow*, points to the MainWindow object (derived from a QMainWindow) that manages plots and user input

-- colorMap : QCPColorMap*, the color map showing the refractive index/potential  distribution

-- colorScale : QCPColorScale*, scale providing a legend for the different hues of the color map

-- coordrange : double[4], coordinate range for the color map (format: {x_min, x_max, y_min, y_max})

-- nx, ny : int, the color map has nx * ny data points

-- playBackTimer : QTimer*, points to the QTimer object that times the light ray animation

-- V : Potential, derived from FunctionParser (see 3rdparty/fparser4),
   parses the input string in the refractive index text field to get the user-specified refractive index/potential
   function, numerically calculates potential gradients at specified points

-- funcString : string, stores the input string for the refractive index input field

-- sources : vector<ParticleSource>, stores particle source objects (see particlesource.cpp)

-- myCurves : vector<QCPCurve*>, points to the QCPCurve objects used to plot individual light rays

-- curveLayer : QCPLayer*, points to an additional layer used to draw the light rays above the color map

-- arrowCurve, tipCurve : QCPCurve*, QCPCurves for drawing a force arrow with differently colored tip

*********
FUNCTIONS
*********

plotV() : void, sets V to the parsed input from the refractive index text field and plots the new color map
resetSources() : void, clears all particle histories, the light ray plot and (if in fMode) the drawn force arrow
calcHistories(double) : void, calculates and stores the histories of all particles for a specified number of
                        RK4 iteration steps
animateHistories() : void, stores all particle histories in the myCurves object, sets up the curve plot and
                     then starts the playBackTimer for the light ray animation
Slots for input:
on_ninput_returnPressed() : void, handles input text field for refractive index function
handleStartButton() : void, handles the button starting the light ray animation
PlaybackStep() : void, called with each playback step of the playBackTimer, progresses the light ray animation
onXRangeChanged(const QCPRange), onYRangeChanged(const QCPRange) : void, for zooming in and out by scroll actions
handleResetButton() : void, handles button for resetting the light ray animation
handleVButton() : void, toggles between refractive index and potential mode
displayValues(QMouseEvent*) : void, for updating values of x, y, n, V, F at mouse movement
handleMouseClick(QMouseEvent*) : void, draws force arrow at right click, removes it at left click
*/
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // program modes:
    /* The program can be in refractive index mode or mechanical potential mode. If vMode is set to true,
       the color map shows the potential corresponding to the refractive index distribution, and right-clicking
       draws force vectors at the indicated position */
    bool vMode = false;
    /* fMode is set to true when the first force arrow is drawn upon right-clicking (if vMode is true). Becomes deactivated
       (and the drawn arrow deleted) upon left-clicking. */
    bool fMode = false;

    void plotV();
    void calcHistories(double);
    void resetSources();
    void animateHistories();

private slots:
    void on_ninput_returnPressed();
    void handleStartButton();
    void PlaybackStep();
    void onXRangeChanged(const QCPRange &newRange);
    void onYRangeChanged(const QCPRange &newRange);
    void handleResetButton();
    void handleVButton();
    void displayValues(QMouseEvent*);
    void handleMouseClick(QMouseEvent*);

private:
    Ui::MainWindow * ui;

    //Variables concerning the simulation:
    //------------------------------------
    Potential V;
    std::string funcString;
    /* In this preliminary version, only one particle source instance is later on
       constructed, but it is not hard anymore to extend this to multiple sources. Possibly, these could be
       custom-placed at different positions by the user in future versions */
    std::vector<ParticleSource> sources;

    //These values performed fine during testing. For future versions, make this specifiable by the user.
    double dx = 1e-6;
    double dt = 1e-3;
    //set initial time to 0
    double t = 0;
    //------------------------------------

    //Variables concerning the plot:
    //------------------------------------
    double timestep = 50;
    double max_size = 0;
    /* When a new potential is drawn, the maximum potential gradient in the coordinate range is calculated and all
       drawn force arrows are scaled with reference to this value */
    double max_force = 0;
    // COLOR MAP SETTINGS
    // coordinate range adapted to the window dimensions (note: make this less of a bodge!)
    double coordrange [4] = {-10, 10, -1*520/880.0*10, 520/880.0*10};
    //
    int nx = 300;
    // scale by screen proportions to have same resolution on both axes
    int ny = (int)(300*520.0/880.0);

    QTimer * playBackTimer;
    QCPCurve * arrowCurve;
    QCPCurve * tipCurve;
    QCPColorScale * colorScale;
    QCPColorMap * colorMap;
    std::vector<QCPCurve*> myCurves;
    QCPLayer * curveLayer;
    //------------------------------------
};

#endif // MAINWINDOW_H
