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
 ------------------ E-mail: (claurel@gmail.com) ----------------------------
 */

#include "timelinewidget.h"
#include "timelineview.h"
#include "Astro-Core/date.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QToolBar>
#include <QAction>
#include <QToolButton>

#include <iostream>

TimelineWidget::TimelineWidget(QWidget* parent) :
    QWidget(parent),
    m_timelineView(NULL),
    m_timeRate(1.0),
    m_paused(true)
{
    m_timelineView = new TimelineView(this);
    connect(m_timelineView, SIGNAL(currentTimeChanged(double)), this, SIGNAL(currentTimeChanged(double)));
    
    QWidget* controls = new QWidget(this);
    controls->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout* controlsLayout = new QHBoxLayout(controls);
    
    QLabel* zoomLabel = new QLabel(controls);
    zoomLabel->setText("Zoom: "
                       "<a href=\"0.04167\">1h</a> "
                       "<a href=\"0.08333\">6h</a> "
                       "<a href=\"0.5\">12h</a> "
                       "<a href=\"1\">1d</a> "
                       "<a href=\"5\">5d</a> "
                       "<a href=\"10\">10d</a> "
                       "<a href=\"30\">1m</a> "
                       "<a href=\"180\">6m</a> "
                       "<a href=\"365\">1y</a> "
                       "<a href=\"all\">all</a>");

    QToolBar* toolbar = new QToolBar(controls);
    toolbar->setIconSize(QSize(24, 24));
    toolbar->setMaximumHeight(24);
    toolbar->setMovable(false);
    toolbar->setFloatable(false);
    toolbar->setContentsMargins(0, 0, 0, 0);
    
    QAction* reverseTimeAction = new QAction(QIcon(":/icons/timeline-reverse.png"),
                                             tr("Reverse time"), toolbar);
    QAction* slowTimeAction = new QAction(QIcon(":/icons/timeline-slower.png"),
                                          tr("10x slower"), toolbar);
    QAction* halfTimeAction = new QAction(QIcon(":/icons/timeline-half.png"),
                                          tr("2x slower"), toolbar);
    QAction* pauseAction = new QAction(QIcon(":/icons/timeline-pause.png"),
                                       tr("Pause time"), toolbar);
    QAction* realTimeAction = new QAction(QIcon(":/icons/timeline-realtime.png"),
                                          tr("Real time"), toolbar);
    QAction* doubleTimeAction = new QAction(QIcon(":/icons/timeline-double.png"),
                                            tr("2x faster"), toolbar);
    QAction* fastTimeAction = new QAction(QIcon(":/icons/timeline-faster.png"),
                                          tr("10x faster"), toolbar);
    
    connect(reverseTimeAction, SIGNAL(triggered()), this, SLOT(reverseTime()));
    toolbar->addAction(reverseTimeAction);
    
    connect(slowTimeAction, SIGNAL(triggered()), this, SLOT(slowerTime()));
    toolbar->addAction(slowTimeAction);
    
    connect(halfTimeAction, SIGNAL(triggered()), this, SLOT(halfTime()));
    toolbar->addAction(halfTimeAction);
    
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(togglePaused()));
    toolbar->addAction(pauseAction);
    
    connect(realTimeAction, SIGNAL(triggered()), this, SLOT(realTime()));
    toolbar->addAction(realTimeAction);
    
    connect(doubleTimeAction, SIGNAL(triggered()), this, SLOT(doubleTime()));
    toolbar->addAction(doubleTimeAction);
    
    connect(fastTimeAction, SIGNAL(triggered()), this, SLOT(fasterTime()));
    toolbar->addAction(fastTimeAction);
    
    
    controlsLayout->addWidget(toolbar);
    
    controlsLayout->addStretch(1);
    controlsLayout->addWidget(zoomLabel);
    controls->setLayout(controlsLayout);
    
    connect(zoomLabel, SIGNAL(linkActivated(const QString&)), this, SLOT(setZoom(const QString&)));
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    layout->addWidget(m_timelineView);
    layout->addWidget(controls);
    //layout->addLayout(controlsLayout);
    layout->setContentsMargins(0, 0, 0, 0);
    
    this->setLayout(layout);
    this->show();
    
    m_timelineView->setTimeRange(0.0, 5.0);
}


TimelineWidget::~TimelineWidget()
{
}


void
TimelineWidget::setZoom(const QString& duration)
{
    bool ok = false;
    double days = 0.0;
    if (duration == "all")
    {
        days = m_timelineView->endTime() - m_timelineView->startTime();
        ok = true;
    }
    else
    {
        days = duration.toDouble(&ok);
    }
    
    if (ok)
    {
        if (days > 0.0)
        {
            m_timelineView->setVisibleSpan(days);
        }
    }
}


void 
TimelineWidget::reverseTime()
{
    m_timeRate = -m_timeRate;
}

void
TimelineWidget::slowerTime()
{
    m_timeRate *= 0.1;
}

void 
TimelineWidget::halfTime()
{
    m_timeRate *= 0.5;
}

void
TimelineWidget::togglePaused()
{
    setPaused(!isPaused());
}

void 
TimelineWidget::realTime()
{
    m_timeRate = 1.0;
    setPaused(false);
}

void
TimelineWidget::doubleTime()
{
    m_timeRate *= 2.0;
}

void
TimelineWidget::fasterTime()
{
    m_timeRate *= 10.0;
}


double
TimelineWidget::currentTime() const
{
    return m_timelineView->currentTime();
}


void
TimelineWidget::setCurrentTime(double mjd)
{
    m_timelineView->setCurrentTime(mjd);
}

void
TimelineWidget::setTimeRate(double timeRate)
{
    m_timeRate = timeRate;
}


void
TimelineWidget::setPaused(bool pause)
{
    m_paused = pause;
}


/** Update the timeline widget.
  *
  * \param dt seconds of real time elapsed since the last call to tick
  */
void
TimelineWidget::tick(double dt)
{
    if (!m_paused && m_timeRate != 0.0)
    {
        setCurrentTime(currentTime() + sta::secsToDays(dt) * m_timeRate);
    }
}
