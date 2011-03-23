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
------------------ Author:       Chris Laurel                   -------------------
-----------------------------------------------------------------------------------
*/

#ifndef _PLOTTING_PLOTSCALE_H_
#define _PLOTTING_PLOTSCALE_H_

#include "PlotLabelFormatter.h"
#include <QList>


/** Abstract base class for transformations that scale data before it is
  * drawn in a PlotView. Subclasses are required to override the scaled()
  * which maps values into the unit interval.
  */
class PlotScale
{
public:
    PlotScale();

    virtual ~PlotScale();

    /** The scaled() method should map values in the visible area of a
      * plot to [0, 1]
      */
    virtual double scaled(double x) const = 0;

    /** unscaled() is the inverse of the scaled() method. It maps values
      * in [0, 1] to the range of the scale.
      */
    virtual double unscaled(double x) const = 0;

    PlotScale* clone() const;

    /** Get a list of tick positions for this scale. This method will
      * return a list of between about 5 and 20 evenly spaced tick
      * positions at round numbers.
      */
    virtual QList<double> ticks() const = 0;

    void setLabelFormatter(PlotLabelFormatter* formatter);

    QString label(double value) const;

protected:
    /** This method must be overridden by PlotScale subclasses. It should
      * copy all subclass-specific values to the new instance.
      */
    virtual PlotScale* cloneContents() const = 0;

private:
    PlotLabelFormatter* m_labelFormatter;
};


/** LinearPlotScale maps values between the specified minimum and
  * maximum values onto the unit interval.
  */
class LinearPlotScale : public PlotScale
{
public:
    LinearPlotScale(double minValue, double maxValue) :
        m_minValue(minValue),
        m_maxValue(maxValue)
    {
    }

    virtual double scaled(double x) const
    {
        return (x - m_minValue) / (m_maxValue - m_minValue);
    }

    virtual double unscaled(double x) const
    {
        return x * (m_maxValue - m_minValue) + m_minValue;
    }

    virtual PlotScale* cloneContents() const
    {
        return new LinearPlotScale(m_minValue, m_maxValue);
    }

    virtual QList<double> ticks() const;

    double minValue() const
    {
        return m_minValue;
    }

    double maxValue() const
    {
        return m_maxValue;
    }

private:
    double m_minValue;
    double m_maxValue;
};

#endif // _PLOTTING_PLOTSCALE_H_
