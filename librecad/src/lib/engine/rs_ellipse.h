/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2011-2012 Dongxu Li (dongxuli2011@gmail.com)
** Copyright (C) 2010 R. van Twisk (librecad@rvt.dds.nl)
** Copyright (C) 2001-2003 RibbonSoft. All rights reserved.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/


#ifndef RS_ELLIPSE_H
#define RS_ELLIPSE_H

// Workaround for Qt bug: https://bugreports.qt-project.org/browse/QTBUG-22829
// TODO: the Q_MOC_RUN detection shouldn't be necessary after this Qt bug is resolved
#ifndef Q_MOC_RUN
#include <boost/version.hpp>
#include <boost/math/tools/roots.hpp>
#include <boost/math/special_functions/ellint_2.hpp>
#endif

class LC_Quadratic;
#include "rs_atomicentity.h"

/**
 * Holds the data that defines an ellipse.
 * angle1=angle2=0.0 is reserved for whole ellipses
 * add 2*M_PI to angle1 or angle2 to make whole range ellipse arcs
 */
class RS_EllipseData {
public:
	RS_EllipseData() = default;
	RS_EllipseData(const RS_EllipseData& ) = default;
    RS_EllipseData(const RS_Vector& center,
                   const RS_Vector& majorP,
                   double ratio,
                   double angle1, double angle2,
                   bool reversed) {

        this->center = center;
        this->majorP = majorP;
        this->ratio = ratio;
        this->angle1 = angle1;
        this->angle2 = angle2;
        this->reversed = reversed;
    }

    friend class RS_Ellipse;

    friend std::ostream& operator << (std::ostream& os, const RS_EllipseData& ed) {
        os << "(" << ed.center <<
           " " << ed.majorP <<
           " " << ed.ratio <<
           " " << ed.angle1 <<
           "," << ed.angle2 <<
           ")";
        return os;
    }

private:
    //! Ellipse center
    RS_Vector center;
    //! Endpoint of major axis relative to center.
    RS_Vector majorP;
    //! Ratio of minor axis to major axis.
    double ratio;
    //! Start angle
    double angle1;
    //! End angle
    double angle2;
    //! Reversed (cw) flag
    bool reversed;
};




/**
 * Class for an ellipse entity. All angles are in Rad.
 *
 * @author Andrew Mustun
 */
class RS_Ellipse : public RS_AtomicEntity {
public:
    RS_Ellipse(RS_EntityContainer* parent,
               const RS_EllipseData& d);
    virtual ~RS_Ellipse() {}

    virtual RS_Entity* clone() {
        RS_Ellipse* e = new RS_Ellipse(*this);
        e->initId();
        return e;
    }

    /**	@return RS2::EntityEllipse */
    virtual RS2::EntityType rtti() const {
        return RS2::EntityEllipse;
    }


    /**
     * @return Start point of the entity.
     */
    virtual RS_Vector getStartpoint() const;
    virtual RS_VectorSolutions getFoci() const;

    /**
     * @return End point of the entity.
     */
    virtual RS_Vector getEndpoint() const;
    virtual RS_Vector getEllipsePoint(const double& a) const; //find the point according to ellipse angle

    virtual void moveStartpoint(const RS_Vector& pos);
    virtual void moveEndpoint(const RS_Vector& pos);
#ifdef  HAS_BOOST
    virtual double getLength() const;

    /**
    //Ellipse must have ratio<1, and not reversed
    *@ x1, ellipse angle
    *@ x2, ellipse angle
    //@return the arc length between ellipse angle x1, x2
    **/
    double getEllipseLength(double a1, double a2) const;
    double getEllipseLength(double a2) const;
#else
    virtual double getLength() const{
        return -1.;
   }
#endif
    virtual RS_VectorSolutions getTangentPoint(const RS_Vector& point) const;//find the tangential points seeing from given point
    virtual RS_Vector getTangentDirection(const RS_Vector& point)const;
    virtual RS2::Ending getTrimPoint(const RS_Vector& trimCoord,
                                     const RS_Vector& trimPoint);

    virtual RS_Vector prepareTrim(const RS_Vector& trimCoord,
                                  const RS_VectorSolutions& trimSol);

    double getEllipseAngle (const RS_Vector& pos) const;

    /** @return Copy of data that defines the ellipse. **/
    RS_EllipseData getData() const {
        return data;
    }

    virtual RS_VectorSolutions getRefPoints();

    /**
     * @retval true if the arc is reversed (clockwise),
     * @retval false otherwise
     */
    bool isReversed() const {
        return data.reversed;
    }
    /** sets the reversed status. */
    void setReversed(bool r) {
        data.reversed = r;
    }

    /** @return The rotation angle of this ellipse */
    double getAngle() const {
        return data.majorP.angle();
    }

    /** @return The start angle of this arc */
    double getAngle1() const {
        return data.angle1;
    }
    /** Sets new start angle. */
    void setAngle1(double a1) {
        data.angle1 = a1;
    }
    /** @return The end angle of this arc */
    double getAngle2() const {
        return data.angle2;
    }
    /** Sets new end angle. */
    void setAngle2(double a2) {
        data.angle2 = a2;
    }


    /** @return The center point (x) of this arc */
    virtual RS_Vector getCenter() const {
        return data.center;
    }
    /** Sets new center. */
    void setCenter(const RS_Vector& c) {
        data.center = c;
    }

    /** @return The endpoint of the major axis (relative to center). */
    RS_Vector getMajorP() const {
        return data.majorP;
    }
    /** Sets new major point (relative to center). */
    void setMajorP(const RS_Vector& p) {
        data.majorP = p;
    }

    /** @return The ratio of minor to major axis */
    double getRatio() const {
        return data.ratio;
    }
    /** Sets new ratio. */
    void setRatio(double r) {
        data.ratio = r;
    }


    /**
     * @return Angle length in rad.
     */
    virtual double getAngleLength() const;

    /** @return The major radius of this ellipse. Same as getRadius() */
    double getMajorRadius() const {
        return data.majorP.magnitude();
    }

    /** @return the point by major minor radius directions */
    RS_Vector getMajorPoint() const{
        return data.center + data.majorP;
    }
    RS_Vector getMinorPoint() const{
        return data.center +
                RS_Vector(-data.majorP.y, data.majorP.x)*data.ratio;
    }

    /** @return The minor radius of this ellipse */
    double getMinorRadius() const {
        return data.majorP.magnitude()*data.ratio;
    }
    bool isArc() const; //is the ellipse an Arc, i.e., angle1/angle2 not both 0
    virtual bool isEdge() const {
        return true;
    }
    bool createFrom4P(const RS_VectorSolutions& sol);
    bool createFromCenter3Points(const RS_VectorSolutions& sol);
    bool createFromQuadratic(const QVector<double>& dn);//from quadratic form: dn[0] x^2 + dn[1] xy + dn[2] y^2 =1
	bool createInscribeQuadrilateral(const std::vector<RS_Line*>& lines);
    virtual RS_Vector getMiddlePoint(void)const;
    virtual RS_Vector getNearestEndpoint(const RS_Vector& coord,
                                         double* dist = NULL) const;
    virtual RS_Vector getNearestPointOnEntity(const RS_Vector& coord,
            bool onEntity = true, double* dist = NULL, RS_Entity** entity=NULL) const;
    virtual RS_Vector getNearestCenter(const RS_Vector& coord,
                                       double* dist = NULL);
    virtual RS_Vector getNearestMiddle(const RS_Vector& coord,
                                       double* dist = NULL,
                                       int middlePoints = 1
                                       )const;
    virtual RS_Vector getNearestDist(double distance,
                                     const RS_Vector& coord,
                                     double* dist = NULL);
    virtual RS_Vector getNearestOrthTan(const RS_Vector& coord,
                                    const RS_Line& normal,
                                     bool onEntity = false);
    bool switchMajorMinor(void); //switch major minor axes to keep major the longer ellipse radius
    virtual void correctAngles();//make sure angleLength() is not more than 2*M_PI
    virtual bool isPointOnEntity(const RS_Vector& coord,
                                 double tolerance=RS_TOLERANCE) const;

    virtual void move(const RS_Vector& offset);
    virtual void rotate(const double& angle);
    virtual void rotate(const RS_Vector& angleVector);
    virtual void rotate(const RS_Vector& center, const double& angle);
    virtual void rotate(const RS_Vector& center, const RS_Vector& angle);
    virtual void scale(const RS_Vector& center, const RS_Vector& factor);
    virtual void mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2);
    virtual void moveRef(const RS_Vector& ref, const RS_Vector& offset);

    /** whether the entity's bounding box intersects with visible portion of graphic view
    */
    bool isVisibleInWindow(RS_GraphicView* view) const;
    virtual void draw(RS_Painter* painter, RS_GraphicView* view, double& patternOffset);
    virtual void drawVisible(RS_Painter* painter, RS_GraphicView* view, double& patternOffset);

    friend std::ostream& operator << (std::ostream& os, const RS_Ellipse& a);

    //virtual void calculateEndpoints();
    virtual void calculateBorders();

    //direction of tangent at endpoints
    virtual double getDirection1() const;
    virtual double getDirection2() const;

    /** return the equation of the entity
    for quadratic,

    return a vector contains:
    m0 x^2 + m1 xy + m2 y^2 + m3 x + m4 y + m5 =0

    for linear:
    m0 x + m1 y + m2 =0
    **/
    virtual LC_Quadratic getQuadratic() const;
/**
 * @brief areaLineIntegral, line integral for contour area calculation by Green's Theorem
 * Contour Area =\oint x dy
 * @return line integral \oint x dy along the entity
 * \oint x dy = Cx y + \frac{1}{4}((a^{2}+b^{2})sin(2a)cos^{2}(t)-ab(2sin^{2}(a)sin(2t)-2t-sin(2t)))
 */
virtual double areaLineIntegral() const;

protected:
    RS_EllipseData data;

};

//functor to solve for distance, used by snapDistance
class EllipseDistanceFunctor
{
public:
    EllipseDistanceFunctor(RS_Ellipse* ellipse, double const& target) : distance(target)
    { // Constructor
        e=ellipse;
        ra=e->getMajorRadius();
        k2=1.- e->getRatio()*e->getRatio();
    }
    void setDistance(const double& target){
        distance=target;
    }
#if BOOST_VERSION > 104500
    boost::math::tuple<double, double, double> operator()(double const& z) const {
#else
    boost::fusion::tuple<double, double, double> operator()(double const& z) const {
#endif
    double cz=cos(z);
    double sz=sin(z);
        //delta amplitude
    double d=sqrt(1-k2*sz*sz);
        // return f(x), f'(x) and f''(x)
#if BOOST_VERSION > 104500
    return boost::math::make_tuple(
#else
    return boost::fusion::make_tuple(
#endif
                    e->getEllipseLength(z)-distance,
                    ra*d,
                    k2*ra*sz*cz/d
                    );
    }

private:

    double distance;
    RS_Ellipse* e;
    double ra;
    double k2;
};


#endif
//EOF
