/*
 This program is free software; you can redistribute it and/or modify it under
 the terms of the European Union Public Licence - EUPL v.1.1 as published by
 the European Commission.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the European Union Public Licence - EUPL v.1.1
 for more details.

 You should have received a copy of the European Union Public Licence - EUPL v.1.1
 along with this program.

 Further information about the European Union Public Licence - EUPL v.1.1 can
 also be found on the world wide web at http://ec.europa.eu/idabc/eupl

*/

/*
 ------ Copyright (C) 2010 STA Steering Board (space.trajectory.analysis AT gmail.com) ----
*/

/*
------------------ Author: Chris Laurel  -------------------------------------------------
 ------------------ E-mail: (claurel@gmail.com) -----------------------------------------
 Patched by Guillermo on June 2010 to include the constellations module
 Patched by Guillermo August 2010 to include the equqtorial grid button
 */

#include "VisualizationToolBar.h"
#include "Main/propagatedscenario.h"
#include "Astro-Core/stabody.h"
#include <QAction>
#include <QMenu>


static double TickIntervals[] =
{
    0.0,
    10.0, 30.0,                        // < 1 minute
    60.0, 120.0, 300.0, 600.0, 1800.0, // < 1 hour
    3600.0, 7200.0
};


VisualizationToolBar::VisualizationToolBar(const QString& title, QWidget* parent) :
    QToolBar(title, parent)
{
    // Create the body selection combo box
    m_bodySelectCombo = new QComboBox(this);
    const QList<StaBody*>& majorBodies = STA_SOLAR_SYSTEM->majorBodies();
    foreach (const StaBody* body, majorBodies)
    {
        // Don't add the Sun to the combo box.
        if (!body->baseTexture().isEmpty() && body->id() != STA_SUN)
        {
            int SolarBody = body->id();
            QString MyIconWithPath;
            if (SolarBody == STA_EARTH) MyIconWithPath = ":/icons/ComboEarth.png";
            else if (SolarBody == STA_MERCURY) MyIconWithPath = ":/icons/ComboMercury.png";
            else if (SolarBody == STA_VENUS) MyIconWithPath = ":/icons/ComboVenus.png";
            else if (SolarBody == STA_MARS) MyIconWithPath = ":/icons/ComboMars.png";
            else if (SolarBody == STA_JUPITER) MyIconWithPath = ":/icons/ComboJupiter.png";
            else if (SolarBody == STA_SATURN) MyIconWithPath = ":/icons/ComboSaturn.png";
            else if (SolarBody == STA_URANUS) MyIconWithPath = ":/icons/ComboUranus.png";
            else if (SolarBody == STA_NEPTUNE) MyIconWithPath = ":/icons/ComboNeptune.png";
            else MyIconWithPath = ":/icons/ComboPluto.png";

            m_bodySelectCombo->addItem(QIcon(MyIconWithPath), body->name(), SolarBody);
        }
    }

    m_bodySelectCombo->addItem(QIcon(":/icons/ComboIo.png"), "Io", STA_IO);
    m_bodySelectCombo->addItem(QIcon(":/icons/ComboEuropa.png"), "Europa", STA_EUROPA);
    m_bodySelectCombo->addItem(QIcon(":/icons/ComboGanymede.png"), "Ganymede", STA_GANYMEDE);
    m_bodySelectCombo->addItem(QIcon(":/icons/ComboCallisto.png"), "Callisto", STA_CALLISTO);
    m_bodySelectCombo->addItem(QIcon(":/icons/ComboEnceladus.png"), "Enceladus", STA_ENCELADUS);
    m_bodySelectCombo->addItem(QIcon(":/icons/ComboTitan.png"), "Titan", STA_TITAN);

    m_bodySelectCombo->setVisible(false);

    // Create the tick interval action
    m_tickIntervalAction = new QAction(QIcon(":/icons/IconCLOCK.png"), tr("Ticks"), this);
    QActionGroup* tickActionGroup = new QActionGroup(this);
    QMenu* tickMenu = new QMenu(tr("Ticks"), this);

    for (unsigned int i = 0; i < sizeof(TickIntervals) / sizeof(TickIntervals[0]); ++i)
    {
        double seconds = TickIntervals[i];

        QString label;
        if (seconds == 0.0)
        {
            label = tr("Disabled");
        }
        else if (seconds < 60.0)
        {
            label = tr("%1 seconds").arg(seconds);
        }
        else if (seconds < 3600.0)
        {
            label = tr("%1 minutes").arg(seconds / 60.0);
        }
        else
        {
            label = tr("%1 hours").arg(seconds / 3600.0);
        }

        QAction* action = new QAction(label, tickMenu);
        action->setData(seconds);
        action->setCheckable(true);
        tickMenu->addAction(action);
        tickActionGroup->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(mapSetTickInterval()));
    }

    tickActionGroup->actions().at(0)->setChecked(true);
    m_tickIntervalAction->setMenu(tickMenu);
    m_tickIntervalAction->setToolTip(tr("Set tick interval"));

    // Making the icons on the visualization bar a little smaller
    setIconSize(QSize(35, 35)); // Smaller icons are a bit better

    m_gridAction = new QAction(QIcon(":/icons/IconGRID.png"), tr("Grid"), this);
    m_gridAction->setCheckable(true);
    m_gridAction->setToolTip(tr("Toggle long/lat grid"));

    m_equatorAction = new QAction(QIcon(":/icons/IconEQUATOR.png"), tr("Equator"), this);
    m_equatorAction->setCheckable(true);
    m_equatorAction->setToolTip(tr("Toggle Equator plane"));

    m_enable25DViewAction = new QAction(QIcon(":/icons/Icon25D.png"), tr("2.5D View"), this);
    m_enable25DViewAction->setCheckable(true);
    m_enable25DViewAction->setToolTip(tr("Toggle 2.5D view"));

    m_terminatorAction = new QAction(QIcon(":/icons/IconSUN.png"), tr("T"), this);
    m_terminatorAction->setCheckable(true);
    m_terminatorAction->setToolTip(tr("Toggle terminator"));

    m_saveImageAction = new QAction(QIcon(":/icons/IconDOWNLOAD.png"), tr("Save"), this);
    m_saveImageAction->setToolTip(tr("Save plot to a file"));

    m_sensorFovsAction = new QAction(QIcon(":/icons/IconBEAM.png"), tr("Sensor FOVs"), this);
    m_sensorFovsAction->setToolTip(tr("Show sensor FOVs"));
    m_sensorFovsAction->setCheckable(true);

    m_observerFrameAction = new QAction(QIcon(":/icons/CoordinateSystem.png"), tr("Observer Frame"), this);
    m_observerFrameAction->setToolTip(tr("Select observer reference frame"));
    m_observerFrameAction->setMenu(new QMenu());
    QAction* inertialAction = m_observerFrameAction->menu()->addAction("Inertial");
    QAction* bodyFixedAction = m_observerFrameAction->menu()->addAction("Body-fixed");
    connect(inertialAction, SIGNAL(triggered()), this, SIGNAL(inertialObserver()));
    connect(bodyFixedAction, SIGNAL(triggered()), this, SIGNAL(bodyFixedObserver()));

    // Analysis (Claas Grohnfeldt, Steffen Peter)
    m_discretizationAction = new QAction(tr("Show Discretization"), this);
    m_discretizationAction->setCheckable(true);
    m_discretizationAction->setToolTip(tr("Toggle discretization"));
    m_discretizationAction->setVisible(false);

    m_coverageCurrentAction = new QAction(tr("Show Current Coverage"), this);
    m_coverageCurrentAction->setCheckable(true);
    m_coverageCurrentAction->setToolTip(tr("Toggle current coverage"));
    m_coverageCurrentAction->setVisible(false);

    m_coverageHistoryAction = new QAction(tr("Show History Coverage"), this);
    m_coverageHistoryAction->setCheckable(true);
    m_coverageHistoryAction->setToolTip(tr("Toggle history coverage"));
    m_coverageHistoryAction->setVisible(false);

    m_linkSOAction = new QAction(tr("Show Links Sat-Sat"), this);
    m_linkSOAction->setCheckable(true);
    m_linkSOAction->setToolTip(tr("Show links between satellites"));
    m_linkSOAction->setVisible(false);

    m_linkGOAction = new QAction(tr("Show Links Sat-GS"), this);
    m_linkGOAction->setCheckable(true);
    m_linkGOAction->setToolTip(tr("Show links between satellites and ground stations"));
    m_linkGOAction->setVisible(false);

    // create Analysis Menu
    m_analysisMenu = new QMenu(this);
    m_analysisMenu->addAction(m_linkSOAction);
    m_analysisMenu->addAction(m_linkGOAction);
    m_analysisMenu->addAction(m_discretizationAction);
    m_analysisMenu->addAction(m_coverageCurrentAction);
    m_analysisMenu->addAction(m_coverageHistoryAction);
    m_analysisAction = new QAction(QIcon(":/icons/IconCONSTELLATION.png"), tr("Constellation Tools"), this);
    m_analysisAction->setMenu(m_analysisMenu);

    QMenu* cameraMenu = new QMenu("Camera viewpoint", this);
    m_cameraAction = new QAction(QIcon(":/icons/IconPARTICIPANTS.png"), "", this);
    m_cameraAction->setToolTip(tr("Select camera viewpoint"));
    m_cameraAction->setMenu(cameraMenu);

    m_trajectoryVisiblePortionAction = new QAction(QIcon(":/icons/IconCLOCK.png"), tr("Trajectory Duration"), this);
    m_trajectoryVisiblePortionAction->setMenu(new QMenu(this));
    m_trajectoryVisiblePortionAction->setToolTip(tr("Visible trajectory portion"));

    // Set the initial state of the actions and widgets
    m_enable25DViewAction->setChecked(false);
    // Next line patch by Guillermo to set to false initial grid and equator
    m_gridAction->setChecked(false);
    m_equatorAction->setChecked(false);
    m_terminatorAction->setChecked(false);
    // Guillermo
    m_analysisAction->setVisible(false);

    connect(m_bodySelectCombo,     SIGNAL(activated(QString)),           this, SLOT(mapBodyChanged(QString)));
    connect(m_gridAction,          SIGNAL(triggered(bool)),              this, SIGNAL(gridToggled(bool)));
    connect(m_equatorAction,       SIGNAL(triggered(bool)),              this, SIGNAL(equatorToggled(bool)));
    connect(m_terminatorAction,    SIGNAL(triggered(bool)),              this, SIGNAL(terminatorToggled(bool)));
    connect(m_enable25DViewAction, SIGNAL(toggled(bool)),                this, SIGNAL(projectionChanged(bool)));
    connect(m_saveImageAction,     SIGNAL(triggered()),                  this, SIGNAL(saveImageRequested()));
    connect(cameraMenu,            SIGNAL(triggered(QAction*)),          this, SLOT(setCameraViewpoint(QAction*)));
    connect(m_sensorFovsAction,    SIGNAL(triggered(bool)),              this, SIGNAL(sensorFovsToggled(bool)));

    // Analysis (Claas Grohnfeldt, Steffen Peter)
    m_discretizationAction->setChecked(false);
    connect(m_discretizationAction,	  SIGNAL(triggered(bool)), this, SIGNAL(discretizationToggled(bool)));
    connect(m_coverageCurrentAction, SIGNAL(triggered(bool)), this, SIGNAL(coverageCurrentToggled(bool)));
    connect(m_coverageHistoryAction, SIGNAL(triggered(bool)), this, SIGNAL(coverageHistoryToggled(bool)));
    connect(m_linkSOAction,          SIGNAL(triggered(bool)), this, SIGNAL(linkSOToggled(bool)));
    connect(m_linkGOAction,          SIGNAL(triggered(bool)), this, SIGNAL(linkGOToggled(bool)));
}


VisualizationToolBar::~VisualizationToolBar()
{
}


/** Configure the tool bar for use with the ground track view.
  */
void
VisualizationToolBar::configureFor2DView(ViewActionGroup* viewActions)
{
    // TODO: The 2D and 3D versions of the tool bar have evolved to be
    // different enough that a better design would be to make them
    // distinct classes.
    clear();

    QAction* widgetAction = addWidget(m_bodySelectCombo);
    widgetAction->setVisible(true);

    addSeparator();  // Guillermo says: in windows, it looks better to be separated from the combobox
    addAction(m_tickIntervalAction);
    addAction(m_gridAction);
    addAction(m_equatorAction);
    addAction(m_terminatorAction);
    addAction(m_enable25DViewAction);
    addAction(m_saveImageAction);
    addSeparator();  // Guillermo says: in windows, it looks better to be separated from the combobox
    addAction(m_analysisAction);

    addViewSelectActions(viewActions->viewSelectGroup());
}


/** Configure the tool bar for use with the 3D view.
  */
void
VisualizationToolBar::configureFor3DView(ViewActionGroup* viewActions)
{
    clear();

    QAction* widgetAction = addWidget(m_bodySelectCombo);
    widgetAction->setVisible(true);


    addAction(m_cameraAction);
    configureCameraMenu(NULL);
    addSeparator();  // Guillermo says: in windows, it looks better to be separated from the combobox

    // For now, omit observer frame menu; this function is handled in a more
    // obvious way via the context menu.
    /*
    addAction(m_observerFrameAction);
    addSeparator();
    */

    const double day = 24.0;
    const double year = 365.25 * day;
    double trajectoryPortions[] = {
        1.0, 2.0, 3.0, 6.0, 12.0,
        1*day, 2*day, 3*day, 5*day, 10*day, 30*day, 100*day,
        1*year, 2*year, 5*year
    };

    QAction* entireAction = m_trajectoryVisiblePortionAction->menu()->addAction("Entire trajectory");
    entireAction->setData(1000*year);

    for (unsigned int i = 0; i < sizeof(trajectoryPortions) / sizeof(trajectoryPortions[0]); ++i)
    {
        double hours = trajectoryPortions[i];
        QAction* action = m_trajectoryVisiblePortionAction->menu()->addAction(tr("1 hour"));
        if (hours < day)
        {
            action->setText(tr("%1 hours").arg(hours));
        }
        else if (hours < year)
        {
            action->setText(tr("%1 days").arg(hours / day));
        }
        else
        {
            action->setText(tr("%1 years").arg(hours / year));
        }
        action->setData(hours / day);
    }

    connect(m_trajectoryVisiblePortionAction->menu(), SIGNAL(triggered(QAction*)), this, SLOT(setTrajectoryPortion(QAction*)));

    // Add all actions and widgets to the toolbar
    addAction(m_trajectoryVisiblePortionAction);
    addAction(m_gridAction);
    addAction(m_equatorAction);
    addAction(m_sensorFovsAction);


    addAction(m_saveImageAction);
    addSeparator();  // Guillermo

    addViewSelectActions(viewActions->viewSelectGroup());
}


QAction*
VisualizationToolBar::createCameraMenuAction(const QString& label, const QString& iconName, const QString& name)
{
    QAction* action = m_cameraAction->menu()->addAction(QIcon(iconName), label);
    action->setData(name);

    return action;
}


void
VisualizationToolBar::addViewSelectActions(QActionGroup* actionGroup)
{
    // Add actions to toggle between 2D and 3D views (and perhaps others eventually). We
    // create an empty, expandable widget first so that the view select buttons end up on
    // the right side of the tool bar.
    QWidget* stretchWidget = new QWidget(this);
    stretchWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    addWidget(stretchWidget);

    foreach (QAction* action, actionGroup->actions())
    {
        addAction(action);
    }
}


void
VisualizationToolBar::configureCameraMenu(const PropagatedScenario* scenario)
{
    m_cameraAction->menu()->clear();

    // Next lines patched by Guillermo to avoid duplication if planets on the 3D bar
    //createCameraMenuAction(tr("Earth"),  ":/icons/ComboEarth.png", "Earth");
    //createCameraMenuAction(tr("Moon"),   ":/icons/ComboMoon.png", "Moon");
    createCameraMenuAction(tr("Earth-Moon System"), ":/icons/ComboEarthMoon.png", "Earth-Moon");

    if (scenario != NULL && !scenario->spaceObjects().isEmpty())
    {
        m_cameraAction->menu()->addSeparator();
        foreach (SpaceObject* spaceObj, scenario->spaceObjects())
        {
            createCameraMenuAction(spaceObj->name(), ":/icons/Satellite.png", QString("S#%1").arg(spaceObj->name()));
        }
    }

    // Next lines patched by Guillermo to avoid duplication if planets on the 3D bar
    m_cameraAction->menu()->addSeparator();
    createCameraMenuAction(tr("Inner Solar System"), ":/icons/ComboInnerSolarSystem.png", "Inner Solar System");
    createCameraMenuAction(tr("Outer Solar System"), ":/icons/ComboOuterSolarSystem.png", "Outer Solar System");
    //m_cameraAction->menu()->addSeparator();
    //createCameraMenuAction(tr("Mercury"), ":/icons/ComboMercury.png", "Mercury");
    //createCameraMenuAction(tr("Venus"),   ":/icons/ComboVenus.png", "Venus");
    //createCameraMenuAction(tr("Mars"),    ":/icons/ComboMars.png", "Mars");
    //createCameraMenuAction(tr("Jupiter"), ":/icons/ComboJupiter.png", "Jupiter");
    //createCameraMenuAction(tr("Saturn"),  ":/icons/ComboSaturn.png", "Saturn");
    //createCameraMenuAction(tr("Uranus"),  ":/icons/ComboUranus.png", "Uranus");
    //createCameraMenuAction(tr("Neptune"), ":/icons/ComboNeptune.png", "Neptune");
    //createCameraMenuAction(tr("Pluto"),   ":/icons/ComboPluto.png", "Pluto");
    m_cameraAction->menu()->addSeparator();
    createCameraMenuAction(tr("Jupiter System"), ":/icons/ComboJupiter.png", "jupiter system");
    createCameraMenuAction(tr("Saturn System"), ":/icons/ComboSaturn.png", "saturn system");
    m_cameraAction->menu()->addSeparator();
    //createCameraMenuAction(tr("Io"),       ":/icons/ComboIo.png", "Io");
    //createCameraMenuAction(tr("Europa"),   ":/icons/ComboEuropa.png", "Europa");
    //createCameraMenuAction(tr("Ganymede"), ":/icons/ComboGanymede.png", "Ganymede");
    //createCameraMenuAction(tr("Callisto"), ":/icons/ComboCallisto.png", "Callisto");
}


// Private slot to translate a tick interval menu event into
// a signal with a double parameter.
void VisualizationToolBar::mapSetTickInterval()
{
    QAction* action = qobject_cast<QAction*>(sender());
    if (action)
    {
        double interval = action->data().toDouble();
        emit tickIntervalChanged(interval);
    }
}


// Private slot to translate a body name into an STA Body
void VisualizationToolBar::mapBodyChanged(QString bodyName)
{
    const StaBody* body = STA_SOLAR_SYSTEM->lookup(bodyName);
    if (body)
    {
        emit bodyChanged(body);
    }
}



// Analysis (Claas Grohnfeldt, Steffen Peter)
// Procedure to create and show the Analysis Toolbar
void VisualizationToolBar::enableAnalysisTools(ConstellationStudy* studyOfConstellations)
{
    // reset
    m_analysisAction->setVisible(false);
    m_linkSOAction->setVisible(false);
    m_linkGOAction->setVisible(false);
    m_discretizationAction->setVisible(false);
    m_coverageCurrentAction->setVisible(false);
    m_coverageHistoryAction->setVisible(false);
    m_analysisAction->setChecked(false);
    m_linkSOAction->setChecked(false);
    m_linkGOAction->setChecked(false);
    m_discretizationAction->setChecked(false);
    m_coverageCurrentAction->setChecked(false);
    m_coverageHistoryAction->setChecked(false);
    // set
    if (studyOfConstellations != NULL)
    {
        if (!studyOfConstellations->m_constellationStudySpaceObjectList.at(0).linksamples.isEmpty()) // link SO
        {
            m_linkSOAction->setVisible(true);
            m_analysisAction->setVisible(true);
        }
        if (!studyOfConstellations->m_constellationStudySpaceObjectList.at(0).groundlinksamples.isEmpty()) // link GO
        {
            m_linkGOAction->setVisible(true);
            m_analysisAction->setVisible(true);
        }
        if(!studyOfConstellations->m_discreteMesh->meshAsList.isEmpty()) // discretization
        {
            m_discretizationAction->setVisible(true);
            m_analysisAction->setVisible(true);
        }
        if(!studyOfConstellations->m_constellationStudySpaceObjectList.at(0).coveragesample.isEmpty()) // coverage
        {
            m_coverageCurrentAction->setVisible(true);
            m_coverageHistoryAction->setVisible(true);
            m_analysisAction->setVisible(true);
        }
    }
}

void VisualizationToolBar::disableAnalysisTools()
{
    m_analysisAction->setVisible(false);
}


void VisualizationToolBar::setCameraViewpoint(QAction* action)
{
    emit cameraViewpointChanged(action->data().toString());
}


void VisualizationToolBar::setTrajectoryPortion(QAction* action)
{
    emit visibleTrajectoryPortionChanged(action->data().toDouble());
}
