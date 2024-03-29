/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
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

#include <QVector>
#include <array>
#include "rs_ellipse.h"

#include "rs_circle.h"
#include "rs_line.h"
#include "rs_graphic.h"
#include "rs_graphicview.h"
#include "rs_painter.h"
#include "rs_information.h"
#include "rs_linetypepattern.h"
#include  "lc_quadratic.h"
#include "rs_painterqt.h"

#ifdef EMU_C99
#include "emu_c99.h" /* C99 math */
#endif

/**
 * Constructor.
 */
RS_Ellipse::RS_Ellipse(RS_EntityContainer* parent,
                       const RS_EllipseData& d)
    :RS_AtomicEntity(parent), data(d) {

    //calculateEndpoints();
    calculateBorders();
}


/**
 * Recalculates the endpoints using the angles and the radius.
 */
/*
void RS_Ellipse::calculateEndpoints() {
   double angle = data.majorP.angle();
   double radius1 = getMajorRadius();
   double radius2 = getMinorRadius();

   startpoint.set(data.center.x + cos(data.angle1) * radius1,
                  data.center.y + sin(data.angle1) * radius2);
   startpoint.rotate(data.center, angle);
   endpoint.set(data.center.x + cos(data.angle2) * radius1,
                data.center.y + sin(data.angle2) * radius2);
   endpoint.rotate(data.center, angle);
}
*/


/**
 * Calculates the boundary box of this ellipse.
 */
void RS_Ellipse::calculateBorders() {
//    RS_DEBUG->print("RS_Ellipse::calculateBorders");

    RS_Vector startpoint = getStartpoint();
    RS_Vector endpoint = getEndpoint();

    double minX = std::min(startpoint.x, endpoint.x);
    double minY = std::min(startpoint.y, endpoint.y);
    double maxX = std::max(startpoint.x, endpoint.x);
    double maxY = std::max(startpoint.y, endpoint.y);

    RS_Vector vp;

    double amin,amax,a;
//      x range
//    vp.set(radius1*cos(angle),radius2*sin(angle));
    vp.set(getMajorP().x,getRatio()*getMajorP().y);
    a=vp.angle();

    amin=RS_Math::correctAngle(getAngle1()+a); // to the range of 0 to 2*M_PI
    amax=RS_Math::correctAngle(getAngle2()+a); // to the range of 0 to 2*M_PI
    if( RS_Math::isAngleBetween(M_PI,amin,amax,isReversed()) ) {
        minX= data.center.x-vp.magnitude();
    }

    if( RS_Math::isAngleBetween(2.*M_PI,amin,amax,isReversed()) ) {
        maxX= data.center.x+vp.magnitude();
    }
    //y range
    vp.set(getMajorP().y, -getRatio()*getMajorP().x);
    a=vp.angle();
    amin=RS_Math::correctAngle(getAngle1()+a); // to the range of 0 to 2*M_PI
    amax=RS_Math::correctAngle(getAngle2()+a); // to the range of 0 to 2*M_PI
    if( RS_Math::isAngleBetween(M_PI,amin,amax,isReversed()) ) {
        minY= data.center.y-vp.magnitude();
    }
    if( RS_Math::isAngleBetween(2.*M_PI,amin,amax,isReversed()) ) {
        maxY= data.center.y+vp.magnitude();
    }

    minV.set(minX, minY);
    maxV.set(maxX, maxY);
//    RS_DEBUG->print("RS_Ellipse::calculateBorders: OK");
}


/**
  * return the foci of ellipse
  *
  *@Author: Dongxu Li
  */

RS_VectorSolutions RS_Ellipse::getFoci() const {
    RS_Vector vp(getMajorP()*sqrt(1.-getRatio()*getRatio()));
    return RS_VectorSolutions(getCenter()+vp, getCenter()-vp);
}

RS_VectorSolutions RS_Ellipse::getRefPoints() {
    RS_VectorSolutions ret;
    if(isArc()){
        //no start/end point for whole ellipse
        ret.push_back(getStartpoint());
        ret.push_back(getEndpoint());
    }
    ret.push_back(data.center);
    ret.appendTo(getFoci());
    ret.push_back(getMajorPoint());
    ret.push_back(getMinorPoint());
    return ret;
}



RS_Vector RS_Ellipse::getNearestEndpoint(const RS_Vector& coord, double* dist)const {
    double dist1, dist2;
    RS_Vector startpoint = getStartpoint();
    RS_Vector endpoint = getEndpoint();

    dist1 = (startpoint-coord).squared();
    dist2 = (endpoint-coord).squared();

    if (dist2<dist1) {
        if (dist!=NULL) {
            *dist = sqrt(dist2);
        }
        return endpoint;
    } else {
        if (dist!=NULL) {
            *dist = sqrt(dist1);
        }
        return startpoint;
    }
}

/**
  *find the tangential points from a given point, i.e., the tangent lines should pass
  * the given point and tangential points
  *
  *Author: Dongxu Li
  */
RS_VectorSolutions RS_Ellipse::getTangentPoint(const RS_Vector& point) const {
    RS_Vector point2(point);
    point2.move(-getCenter());
    RS_Vector aV(-getAngle());
    point2.rotate(aV);
    RS_VectorSolutions sol;
    double a=getMajorRadius();
    if(a<RS_TOLERANCE || getRatio()<RS_TOLERANCE) return sol;
    RS_Circle c(NULL, RS_CircleData(RS_Vector(0.,0.),a));
    point2.y /=getRatio();
    sol=c.getTangentPoint(point2);
    sol.scale(RS_Vector(1.,getRatio()));
    aV.y *= -1.;
    sol.rotate(aV);
    sol.move(getCenter());
    return sol;
}

RS_Vector RS_Ellipse::getTangentDirection(const RS_Vector& point) const {
    RS_Vector vp(point-getCenter());
    RS_Vector aV(-getAngle());
    vp.rotate(aV);
    double a=getMajorRadius();
    if(a<RS_TOLERANCE || getRatio()<RS_TOLERANCE) return RS_Vector(false);
    RS_Circle c(NULL, RS_CircleData(RS_Vector(0.,0.),a));
    RS_Vector direction=c.getTangentDirection(vp);
    direction.y *= getRatio();
    aV.y *= -1.;
    direction.rotate(aV);
    return direction;
}

/**
  * find total length of the ellipse (arc)
  *
  *Author: Dongxu Li
  */
double RS_Ellipse::getLength() const
{
        RS_Ellipse e(NULL, data);
        //switch major/minor axis, because we need the ratio smaller than one
    if(e.getRatio()>1.)  e.switchMajorMinor();
    if(e.isReversed()) {
        e.setReversed(false);
        std::swap(e.data.angle1,e.data.angle2);
    }
    return e.getEllipseLength(e.data.angle1,e.data.angle2);
}

/**
//Ellipse must have ratio<1, and not reversed
*@ x1, ellipse angle
*@ x2, ellipse angle
//@return the arc length between ellipse angle x1, x2
**/
double RS_Ellipse::getEllipseLength(double x1, double x2) const
{
    double a(getMajorRadius()),k(getRatio());
    k= 1-k*k;//elliptic modulus, or eccentricity
//    std::cout<<"1, angle1="<<x1/M_PI<<" angle2="<<x2/M_PI<<std::endl;
//    if(isReversed())  std::swap(x1,x2);
    x1=RS_Math::correctAngle(x1);
    x2=RS_Math::correctAngle(x2);
//    std::cout<<"2, angle1="<<x1/M_PI<<" angle2="<<x2/M_PI<<std::endl;
    if(x2 < x1+RS_TOLERANCE_ANGLE) x2 += 2.*M_PI;
    double ret;
//    std::cout<<"3, angle1="<<x1/M_PI<<" angle2="<<x2/M_PI<<std::endl;
    if( x2 >= M_PI) {
        // the complete elliptic integral
        ret=  (static_cast< int>((x2+RS_TOLERANCE_ANGLE)/M_PI) -
               (static_cast<int>((x1+RS_TOLERANCE_ANGLE)/M_PI)
                ))*2;
//        std::cout<<"Adding "<<ret<<" of E("<<k<<")\n";
        ret*=boost::math::ellint_2<double>(k);
    } else {
        ret=0.;
    }
    x1=fmod(x1,M_PI);
    x2=fmod(x2,M_PI);
    if( fabs(x2-x1)>RS_TOLERANCE_ANGLE)  {
        ret += RS_Math::ellipticIntegral_2(k,x2)-RS_Math::ellipticIntegral_2(k,x1);
    }
    return a*ret;
}

/**
  * arc length from start point (angle1)
  */
double RS_Ellipse::getEllipseLength( double x2) const
{
    return getEllipseLength(getAngle1(),x2);
}


/**
  * get the point on the ellipse arc and with arc distance from the start point
  * the distance is expected to be within 0 and getLength()
  * using Newton-Raphson from boost
  *
  *Author: Dongxu Li
  */

RS_Vector RS_Ellipse::getNearestDist(double distance,
                                     const RS_Vector& coord,
                                     double* dist) {
//    RS_DEBUG->print("RS_Ellipse::getNearestDist() begin\n");
    if( ! isArc() ) {
        // both angles being 0, whole ellipse
        // no end points for whole ellipse, therefore, no snap by distance from end points.
        return RS_Vector(false);
    }
    RS_Ellipse e(NULL,data);
    if(e.getRatio()>1.) e.switchMajorMinor();
    double ra=e.getMajorRadius();
    double rb=e.getRatio()*ra;
    if(e.isReversed()) {
        std::swap(e.data.angle1,e.data.angle2);
        e.setReversed(false);
    }
    if(ra<RS_TOLERANCE) { //elipse too small
        return(RS_Vector(false));
    }
    if(getRatio()<RS_TOLERANCE) {
        //treat the ellipse as a line
        RS_Line line(NULL,RS_LineData(e.minV,e.maxV));
        return line.getNearestDist(distance,coord,dist);
    }
    double x1=e.getAngle1();
    double x2=e.getAngle2();
    if(x2<x1+RS_TOLERANCE_ANGLE) x2 += 2.*M_PI;
    double l=e.getEllipseLength(x1,x2); // the getEllipseLength() function only defined for proper e
    distance=fabs(distance);
    if(distance > l+RS_TOLERANCE) return(RS_Vector(false));
    if(distance > l-RS_TOLERANCE) return(getNearestEndpoint(coord,dist));
    double guess= distance*(ra+rb)/(2.*ra*rb);

    guess=(RS_Vector(x1+guess).scale(RS_Vector(e.getRatio(),1.))).angle();//convert to ellipse angle
    if( guess < x1) guess += 2.*M_PI;
    if( ! RS_Math::isAngleBetween(guess,x1,x2,false)) {
        guess=x1 +0.5*RS_Math::getAngleDifference(x1,x2);
    }
    int digits=std::numeric_limits<double>::digits;

    //    solve equation of the distance by second order newton_raphson
    EllipseDistanceFunctor X(&e,distance);

//std::cout<<"RS_Ellipse::getNearestDist() dist="<<distance<<" out of total="<<l<<std::endl;
    RS_Vector vp1(e.getEllipsePoint(boost::math::tools::halley_iterate<EllipseDistanceFunctor,double>(
                                      X, guess, x1, x2, digits)));
    X.setDistance(l-distance);
    guess=x1+(x2-guess);
    RS_Vector vp2(e.getEllipsePoint(boost::math::tools::halley_iterate<EllipseDistanceFunctor,double>(
                                      X, guess, x1, x2, digits)));
    x1= (vp1-coord).squared();
    x2= (vp2-coord).squared();
    if( x1 > x2 ){
        if(dist !=NULL)  *dist=sqrt(x2);
        return vp2;
    }else{
        if(dist !=NULL)  *dist=sqrt(x1);
        return vp1;
    }
}


/**
  * switch the major/minor axis naming
  *
  *Author: Dongxu Li
  */
bool RS_Ellipse::switchMajorMinor(void)
//switch naming of major/minor, return true if success
{
    if (fabs(data.ratio) < RS_TOLERANCE) return false;
    RS_Vector vp_start=getStartpoint();
    RS_Vector vp_end=getEndpoint();
    RS_Vector vp=getMajorP();
    setMajorP(RS_Vector(- data.ratio*vp.y, data.ratio*vp.x)); //direction pi/2 relative to old MajorP;
    setRatio(1./data.ratio);
    if( isArc() )  {
        //only reset start/end points for ellipse arcs, i.e., angle1 angle2 are not both zero
        setAngle1(getEllipseAngle(vp_start));
        setAngle2(getEllipseAngle(vp_end));
    }
    return true;
}

/**
 * @return Start point of the entity.
 */
RS_Vector  RS_Ellipse::getStartpoint() const {
    return getEllipsePoint(data.angle1);
}
/**
 * @return End point of the entity.
 */
RS_Vector  RS_Ellipse::getEndpoint() const {
    return getEllipsePoint(data.angle2);
}
/**
 * @return Ellipse point by ellipse angle
 */
RS_Vector  RS_Ellipse::getEllipsePoint(const double& a) const {
    RS_Vector p(a);
    double ra=getMajorRadius();
    p.scale(RS_Vector(ra,ra*getRatio()));
    p.rotate(getAngle());
    p.move(getCenter());
    return p;
}
//implemented using an analytical aglorithm
// find nearest point on ellipse to a given point
//
// @author Dongxu Li <dongxuli2011@gmail.com>
//

RS_Vector RS_Ellipse::getNearestPointOnEntity(const RS_Vector& coord,
        bool onEntity, double* dist, RS_Entity** entity)const
{

    RS_DEBUG->print("RS_Ellipse::getNearestPointOnEntity");
    RS_Vector ret(false);

    if( ! coord.valid ) {
        if ( dist != NULL ) *dist=RS_MAXDOUBLE;
        return ret;

    }

    if (entity!=NULL) {
        *entity = const_cast<RS_Ellipse*>(this);
    }
    ret=coord;
    ret.move(-getCenter());
    ret.rotate(-getAngle());
    double x=ret.x,y=ret.y;
    double a=getMajorRadius();
    double b=a*getRatio();
    //std::cout<<"(a= "<<a<<" b= "<<b<<" x= "<<x<<" y= "<<y<<" )\n";
    //std::cout<<"finding minimum for ("<<x<<"-"<<a<<"*cos(t))^2+("<<y<<"-"<<b<<"*sin(t))^2\n";
    double twoa2b2=2*(a*a-b*b);
    double twoax=2*a*x;
    double twoby=2*b*y;
    double a0=twoa2b2*twoa2b2;
    std::vector<double> ce(4,0.);
    std::vector<double> roots(0,0.);

    //need to handle a=b
    if(a0 > RS_TOLERANCE2 ) { // a != b , ellipse
        ce[0]=-2.*twoax/twoa2b2;
        ce[1]= (twoax*twoax+twoby*twoby)/a0-1.;
        ce[2]= - ce[0];
        ce[3]= -twoax*twoax/a0;
        //std::cout<<"1::find cosine, variable c, solve(c^4 +("<<ce[0]<<")*c^3+("<<ce[1]<<")*c^2+("<<ce[2]<<")*c+("<<ce[3]<<")=0,c)\n";
        roots=RS_Math::quarticSolver(ce);
    } else {//a=b, quadratic equation for circle
        a0=twoby/twoax;
        roots.push_back(sqrt(1./(1.+a0*a0)));
        roots.push_back(-roots[0]);
    }
    if(roots.size()==0) {
        //this should not happen
        std::cout<<"(a= "<<a<<" b= "<<b<<" x= "<<x<<" y= "<<y<<" )\n";
        std::cout<<"finding minimum for ("<<x<<"-"<<a<<"*cos(t))^2+("<<y<<"-"<<b<<"*sin(t))^2\n";
        std::cout<<"2::find cosine, variable c, solve(c^4 +("<<ce[0]<<")*c^3+("<<ce[1]<<")*c^2+("<<ce[2]<<")*c+("<<ce[3]<<")=0,c)\n";
        std::cout<<ce[0]<<' '<<ce[1]<<' '<<ce[2]<<' '<<ce[3]<<std::endl;
        std::cerr<<"RS_Math::RS_Ellipse::getNearestPointOnEntity() finds no root from quartic, this should not happen\n";
        return RS_Vector(coord); // better not to return invalid: return RS_Vector(false);
    }

//    RS_Vector vp2(false);
    double d,d2,s,dDistance(RS_MAXDOUBLE*RS_MAXDOUBLE);
    //double ea;
    for(size_t i=0; i<roots.size(); i++) {
        //I don't understand the reason yet, but I can do without checking whether sine/cosine are valid
        //if ( fabs(roots[i])>1.) continue;
        s=twoby*roots[i]/(twoax-twoa2b2*roots[i]); //sine
        //if (fabs(s) > 1. ) continue;
        d2=twoa2b2+(twoax-2.*roots[i]*twoa2b2)*roots[i]+twoby*s;
        if (d2<0) continue; // fartherest
        RS_Vector vp3;
        vp3.set(a*roots[i],b*s);
        d=(vp3-ret).squared();
//        std::cout<<i<<" Checking: cos= "<<roots[i]<<" sin= "<<s<<" angle= "<<atan2(roots[i],s)<<" ds2= "<<d<<" d="<<d2<<std::endl;
        if( ret.valid && d>dDistance) continue;
        ret=vp3;
        dDistance=d;
//			ea=atan2(roots[i],s);
    }
    if( ! ret.valid ) {
        //this should not happen
//        std::cout<<ce[0]<<' '<<ce[1]<<' '<<ce[2]<<' '<<ce[3]<<std::endl;
//        std::cout<<"(x,y)=( "<<x<<" , "<<y<<" ) a= "<<a<<" b= "<<b<<" sine= "<<s<<" d2= "<<d2<<" dist= "<<d<<std::endl;
//        std::cout<<"RS_Ellipse::getNearestPointOnEntity() finds no minimum, this should not happen\n";
        RS_DEBUG->print(RS_Debug::D_ERROR,"RS_Ellipse::getNearestPointOnEntity() finds no minimum, this should not happen\n");
    }
    if (dist!=NULL) {
        *dist = sqrt(dDistance);
    }
    ret.rotate(getAngle());
    ret.move(getCenter());
//    ret=vp2;
    if (onEntity) {
        if (!RS_Math::isAngleBetween(getEllipseAngle(ret), getAngle1(), getAngle2(), isReversed())) { // not on entity, use the nearest endpoint
               //std::cout<<"not on ellipse, ( "<<getAngle1()<<" "<<getEllipseAngle(ret)<<" "<<getAngle2()<<" ) reversed= "<<isReversed()<<"\n";
            ret=getNearestEndpoint(coord,dist);
        }
    }

//    if(! ret.valid) {
//        std::cout<<"RS_Ellipse::getNearestOnEntity() returns invalid by mistake. This should not happen!"<<std::endl;
//    }
    return ret;
}




/**
 * @param tolerance Tolerance.
 *
 * @retval true if the given point is on this entity.
 * @retval false otherwise
 */
bool RS_Ellipse::isPointOnEntity(const RS_Vector& coord,
                                 double tolerance) const {
    double t=fabs(tolerance);
    double a=getMajorRadius();
    double b=a*getRatio();
    RS_Vector vp((coord - getCenter()).rotate(-getAngle()));
    if ( a<RS_TOLERANCE ) {
        //radius treated as zero
        if(fabs(vp.x)<RS_TOLERANCE && fabs(vp.y) < b) return true;
        return false;
    }
    if ( b<RS_TOLERANCE ) {
        //radius treated as zero
        if (fabs(vp.y)<RS_TOLERANCE && fabs(vp.x) < a) return true;
        return false;
    }
    vp.scale(RS_Vector(1./a,1./b));

    if (fabs(vp.squared()-1.) > t) return false;
    return RS_Math::isAngleBetween(vp.angle(),getAngle1(),getAngle2(),isReversed());

//    if ( getCenter().distanceTo(coord) < tolerance ) {
//            if (getMajorRadius() < tolerance || getMinorRadius() < tolerance ) {
//                    return true;
//            } else {
//                    return false;
//            }
//    }
//    double dist = getDistanceToPoint(coord, NULL, RS2::ResolveNone);
//    return (dist<=tolerance);
}



RS_Vector RS_Ellipse::getNearestCenter(const RS_Vector& coord,
                                       double* dist) {
    RS_Vector   vCenter = data.center;
    double      distCenter = coord.distanceTo(data.center);

    RS_VectorSolutions  vsFoci = getFoci();
    if( 2 == vsFoci.getNumber()) {
        RS_Vector vFocus1 = vsFoci.get(0);
        RS_Vector vFocus2 = vsFoci.get(1);

        double distFocus1 = coord.distanceTo(vFocus1);
        double distFocus2 = coord.distanceTo(vFocus2);

        /* if (distFocus1 < distCenter) is true
         * then (distFocus1 < distFocus2) must be true too
         * and vice versa
         * no need to check this */
        if( distFocus1 < distCenter) {
            vCenter = vFocus1;
            distCenter = distFocus1;
        }
        else if( distFocus2 < distCenter) {
            vCenter = vFocus2;
            distCenter = distFocus2;
        }
    }

    if (NULL != dist) {
        *dist = distCenter;
    }
    return vCenter;
}

/**
//create Ellipse with axes in x-/y- directions from 4 points
*
*
*@Author Dongxu Li
*/
bool	RS_Ellipse::createFrom4P(const RS_VectorSolutions& sol)
{
    if (sol.getNumber() != 4 ) return (false); //only do 4 points
    QVector<QVector<double> > mt;
    QVector<double> dn;
    int mSize(4);
    mt.resize(mSize);
    for(int i=0;i<mSize;i++) {//form the linear equation, c0 x^2 + c1 x + c2 y^2 + c3 y = 1
        mt[i].resize(mSize+1);
        mt[i][0]=sol.get(i).x * sol.get(i).x;
        mt[i][1]=sol.get(i).x ;
        mt[i][2]=sol.get(i).y * sol.get(i).y;
        mt[i][3]=sol.get(i).y ;
        mt[i][4]=1.;
    }
    if ( ! RS_Math::linearSolver(mt,dn)) return false;
    double d(1.+0.25*(dn[1]*dn[1]/dn[0]+dn[3]*dn[3]/dn[2]));
    if(fabs(dn[0])<RS_TOLERANCE15
            ||fabs(dn[2])<RS_TOLERANCE15
            ||d/dn[0]<RS_TOLERANCE15
            ||d/dn[2]<RS_TOLERANCE15
            ) {
        //ellipse not defined
        return false;
    }
    data.center.set(-0.5*dn[1]/dn[0],-0.5*dn[3]/dn[2]); // center
    d=sqrt(d/dn[0]);
    data.majorP.set(d,0.);
    data.ratio=sqrt(dn[0]/dn[2]);
    data.angle1=0.;
    data.angle2=0.;
//    DEBUG_HEADER();
//    std::cout<<"center="<<data.center;
//    std::cout<<"majorP="<<data.majorP;
//    std::cout<<"ratio="<<data.ratio;
//    std::cout<<"successful"<<std::endl;
    return true;

}

/**
//create Ellipse with center and 3 points
*
*
*@Author Dongxu Li
*/
bool	RS_Ellipse::createFromCenter3Points(const RS_VectorSolutions& sol) {
    if(sol.getNumber()<3) return false; //need one center and 3 points on ellipse
    QVector<QVector<double> > mt;
    int mSize(sol.getNumber() -1);
    if( (sol.get(mSize) - sol.get(mSize-1)).squared() < RS_TOLERANCE15 ) {
        //remove the last point
        mSize--;
    }

    mt.resize(mSize);
    QVector<double> dn(mSize);
    switch(mSize){
    case 2:
        for(int i=0;i<mSize;i++){//form the linear equation
            mt[i].resize(mSize+1);
            RS_Vector vp(sol.get(i+1)-sol.get(0)); //the first vector is center
            mt[i][0]=vp.x*vp.x;
            mt[i][1]=vp.y*vp.y;
            mt[i][2]=1.;
        }
        if ( ! RS_Math::linearSolver(mt,dn) ) return false;
        if( dn[0]<RS_TOLERANCE15 || dn[1]<RS_TOLERANCE15) return false;
        setMajorP(RS_Vector(1./sqrt(dn[0]),0.));
        setRatio(sqrt(dn[0]/dn[1]));
        setAngle1(0.);
        setAngle2(0.);
        setCenter(sol.get(0));
        return true;

    case 3:
        for(int i=0;i<mSize;i++){//form the linear equation
            mt[i].resize(mSize+1);
            RS_Vector vp(sol.get(i+1)-sol.get(0)); //the first vector is center
            mt[i][0]=vp.x*vp.x;
            mt[i][1]=vp.x*vp.y;
            mt[i][2]=vp.y*vp.y;
            mt[i][3]=1.;
        }
        if ( ! RS_Math::linearSolver(mt,dn) ) return false;
        setCenter(sol.get(0));
        return createFromQuadratic(dn);
    default:
        return false;
    }
    return false; // only for compiler warning
}

/**create from quadratic form:
  * dn[0] x^2 + dn[1] xy + dn[2] y^2 =1
  * centered at (0,0)
  *
  *@Author: Dongxu Li
  */
bool RS_Ellipse::createFromQuadratic(const QVector<double>& dn){
	if(dn.size()<3) return false;
	if(fabs(dn[0]) <RS_TOLERANCE2 || fabs(dn[2])<RS_TOLERANCE2) return false; //invalid quadratic form
	RS_DEBUG->print("RS_Ellipse::createFromQuadratic() begin\n");

//		qDebug()<<"Quadratic: ";
//	for(auto& f: dn){
//		qDebug()<<"dn: "<<f;
//	}
	//eigenvalue and eigen vectors of quadratic form
    // (dn[0] 0.5*dn[1])
    // (0.5*dn[1] dn[2])
	double d(dn[0]-dn[2]);
	bool rotated=false;
	if(d<0.f && fabs(dn[1])<=10.*RS_TOLERANCE15) {
		rotated = true;
		d=fabs(d);
	}
    double s(sqrt(d*d+dn[1]*dn[1]));
    //        std::cout<<"d="<<d<<std::endl;
    //        std::cout<<"s="<<s<<std::endl;
    double lambda1(0.5*(s+dn[0]+dn[2]));
    double lambda2(0.5*(-s+dn[0]+dn[2]));
//			std::cout<<"lambda1="<<lambda1<<"\tlambda2="<<lambda2<<std::endl;
    if(lambda1<RS_TOLERANCE15 || lambda2<RS_TOLERANCE15) return false;
	RS_Vector mP(-dn[1]/(s+d),1.);
//	std::cout<<"mP="<<mP<<std::endl;
	mP /= sqrt(mP.squared()*lambda2);
//	std::cout<<"mP="<<mP<<std::endl;
//    ratio=sqrt(lambda2/lambda1);
//    setCenter(center);
	if(rotated) mP.rotate(RS_Vector(0., 1.));
	setMajorP(mP);
    setRatio(sqrt(lambda2/lambda1));
    setAngle1(0.);
    setAngle2(0.);
//    if(angleVector.valid) {//need to rotate back, for the parallelogram case
//        angleVector.y *= -1.;
//        rotate(angleVector);
//    }
    return true;
}

/**
//create Ellipse inscribed in a quadrilateral
*
*algorithm: http://chrisjones.id.au/Ellipses/ellipse.html
*finding the tangential points and ellipse center
*
*@Author Dongxu Li
*/
bool	RS_Ellipse::createInscribeQuadrilateral(const std::vector<RS_Line*>& lines)
{
	if(lines.size() != 4) return false; //only do 4 lines
	RS_EntityContainer container(NULL, false);
    QVector<RS_Line*> quad;
	for(RS_Line* p: lines){//copy the line pointers
		if(p->getLength()<RS_TOLERANCE) return false;
		quad.push_back(p);
		container.addEntity(p);
    }
    //    std::cout<<"0\n";
	for(size_t i=0;i<lines.size()*2;i++){//move parallel lines to opposite
		size_t j=(i+1)%lines.size();

        //        std::cout<<"("<<i<<","<<j<<")\n";
        //        std::cout<<*quad[i]<<std::endl;
        //        std::cout<<*quad[j]<<std::endl;
        RS_VectorSolutions sol=RS_Information::getIntersectionLineLine(quad[i%lines.size()],quad[j]);
        if(sol.getNumber()==0) {
            std::swap( quad[j],quad[ (i+2)%lines.size()]); //move to oppose
			++i;
        }
    }
    //    std::cout<<"========1========\n";

    QVector<RS_Line> ip;
	for(int i=1;i<4;++i){//find intersections
        //(0,i)
        //        std::cout<<"(0,"<<i<<")\n";
        RS_VectorSolutions sol0=RS_Information::getIntersectionLineLine(quad[0],quad[i]);
        if(sol0.getNumber()==0) continue;
        int l(1);
		if( l==i) ++l;
        int m(l+1);
		if( m==i) ++m;
        // lines in two pairs: (0, i) and (l,m)
        //        std::cout<<"(0,"<<i<<"):("<<l<<","<<m<<")\n";
        RS_VectorSolutions sol1=RS_Information::getIntersectionLineLine(quad[l],quad[m]);
        if(sol1.getNumber()==0) continue;

		ip.push_back(RS_Line(sol0.get(0),sol1.get(0)));

    }

    //    std::cout<<"20 ip.size()="<<ip.size()<<"\n";
    if(ip.size()<2) return false; //not enough connecting lines, so, no quadrilateral defined
    //    std::cout<<"22\n";
    if(ip.size() == 3) {//find intersecting pair
        //    RS_VectorSolutions sol0=RS_Information::getIntersection(line0,line1,true);
		RS_VectorSolutions sol1=RS_Information::getIntersection(&ip[2],&ip[1],true);
        if(sol1.getNumber()) {
            ip[0]=ip[2];
        }else{
            sol1=RS_Information::getIntersection(& ip[2],&ip[0],true);
            if(sol1.getNumber()) {
                ip[1]=ip[2];
            }
        }
    }
	RS_VectorSolutions sol=RS_Information::getIntersectionLineLine( & ip[0],& ip[1]);
    if(sol.getNumber()==0) {//this should not happen
//        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Ellipse::createInscribeQuadrilateral(): can not locate projection Center");
        RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): can not locate projection Center");
        return false;
    }
    RS_Vector centerProjection(sol.get(0));
//        std::cout<<"RS_Ellipse::createInscribe(): centerProjection="<<centerProjection<<std::endl;

	std::array<std::unique_ptr<RS_Line>, 4> edge; //form the closed quadrilateral with ordered edges
	edge[0].reset(new RS_Line(
					  container.getNearestEndpoint(ip[0].getStartpoint()),
				   container.getNearestEndpoint(ip[1].getStartpoint())));
	edge[1].reset(new RS_Line(
					  container.getNearestEndpoint(ip[1].getStartpoint()),
				   container.getNearestEndpoint(ip[0].getEndpoint())));
	edge[2].reset(new RS_Line(
					  container.getNearestEndpoint(ip[0].getEndpoint()),
				   container.getNearestEndpoint(ip[1].getEndpoint())));
	edge[3].reset(new RS_Line(
					  container.getNearestEndpoint(ip[1].getEndpoint()),
				   container.getNearestEndpoint(ip[0].getStartpoint())));
//	qDebug()<<"\nedge:{";
//	for(std::unique_ptr<RS_Line>& p: edge ){
//		std::cout<<p->getStartpoint()<<" , "<<p->getEndpoint()<<std::endl;
//	}
//	qDebug()<<"}edge:";
	std::vector<RS_Vector> tangent;//holds the tangential points on edges, in the order of edges: 1 3 2 0
	int parallel=0;
	int parallel_index=0;
	for(int i=0;i<=1;++i) {
		RS_VectorSolutions sol1=RS_Information::getIntersectionLineLine(edge[i].get(), edge[(i+2)%edge.size()].get());
        RS_Vector direction;
        if(sol1.getNumber()==0) {
			direction=edge[i]->getEndpoint()-edge[i]->getStartpoint();
			++parallel;
			parallel_index=i;
        }else{
            direction=sol1.get(0)-centerProjection;
        }
        //                std::cout<<"Direction: "<<direction<<std::endl;
        RS_Line l(centerProjection, centerProjection+direction);
        for(int k=1;k<=3;k+=2){
			RS_VectorSolutions sol2=RS_Information::getIntersectionLineLine(&l, edge[(i+k)%edge.size()].get());
			if(sol2.size())  tangent.push_back(sol2.get(0));
        }
    }

	if(tangent.size()<3) return false;

	RS_Line cl0(ip[0].getEndpoint(),(tangent[0]+tangent[2])*0.5);
	RS_Line cl1(ip[1].getEndpoint(),(tangent[1]+tangent[2])*0.5);
	sol=RS_Information::getIntersection(&cl0, &cl1,false);
    if(sol.getNumber()==0){
        //this should not happen
//        RS_DEBUG->print(RS_Debug::D_WARNING, "RS_Ellipse::createInscribeQuadrilateral(): can not locate Ellipse Center");
        RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): can not locate Ellipse Center");
        return false;
    }
	//ellipse center
    RS_Vector center(sol.get(0));
//                    std::cout<<"line0: "<<*cl0<<std::endl;
//                    std::cout<<"line1: "<<*cl1<<std::endl;
//                    std::cout<<"center: "<<center<<std::endl;
//	qDebug()<<"parallel="<<parallel;
	if(parallel==1){
		RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): trapezoid detected\n");
		//trapezoid
		RS_Line* l0=edge[parallel_index].get();
		RS_Line* l1=edge[(parallel_index+2)%4].get();
		RS_Vector&& centerPoint=(l0->getMiddlePoint()+l1->getMiddlePoint())*0.5;
		//not symmetric, no inscribed ellipse
		if( fabs(centerPoint.distanceTo(l0->getStartpoint()) - centerPoint.distanceTo(l0->getEndpoint()))>RS_TOLERANCE)
			return false;
		//symmetric
		RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): symmetric trapezoid detected\n");
		double d=l0->getDistanceToPoint(centerPoint);
		double l=((l0->getLength()+l1->getLength()))*0.25;
		double k= 4.*d/fabs(l0->getLength()-l1->getLength());
		double theta=d/(l*k);
		if(theta>1. || d<RS_TOLERANCE) {
			RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): this should not happen\n");
			return false;
		}
		theta=asin(theta);

		//major axis
		double a=d/(k*tan(theta));
		setCenter(RS_Vector(0., 0.));
		setMajorP(RS_Vector(a, 0.));
		setRatio(d/a);
		rotate(l0->getAngle1());
		setCenter(centerPoint);
		return true;

	}
//    double ratio;
    //        std::cout<<"dn="<<dn[0]<<' '<<dn[1]<<' '<<dn[2]<<std::endl;
	QVector<double> dn(3);
    RS_Vector angleVector(false);

	for(size_t i=0;i<tangent.size();i++) {
        tangent[i] -= center;//relative to ellipse center
    }
    QVector<QVector<double> > mt;
    mt.clear();
	const double symTolerance=100.*RS_TOLERANCE;
	for(const RS_Vector& vp: tangent){//form the linear equation
//		std::cout<<"point  : "<<vp<<std::endl;
        QVector<double> mtRow;
        mtRow.push_back(vp.x*vp.x);
        mtRow.push_back(vp.x*vp.y);
        mtRow.push_back(vp.y*vp.y);
        bool addRow(true);
        for(int j=0;j<mt.size();j++){
			if(fabs(mtRow[0]-mt[j][0])<symTolerance &&
				 fabs(mtRow[1]-mt[j][1])<symTolerance &&
				 fabs(mtRow[2]-mt[j][2])<symTolerance){
                //symmetric
                addRow=false;
                break;
            }
        }
        if(addRow) {
            mtRow.push_back(1.);
            mt.push_back(mtRow);
        }
    }
//    std::cout<<"mt.size()="<<mt.size()<<std::endl;
    switch(mt.size()){
    case 2:{// the quadrilateral is a parallelogram
		RS_DEBUG->print("RS_Ellipse::createInscribeQuadrilateral(): parallelogram detected\n");

        //fixme, need to handle degenerate case better
        //        double angle(center.angleTo(tangent[0]));
        RS_Vector majorP(tangent[0]);
        double dx(majorP.magnitude());
        if(dx<RS_TOLERANCE2) return false; //refuse to return zero size ellipse
        angleVector.set(majorP.x/dx,-majorP.y/dx);
		for(size_t i=0;i<tangent.size();i++)tangent[i].rotate(angleVector);

        RS_Vector minorP(tangent[2]);
        double dy2(minorP.squared());
        if(fabs(minorP.y)<RS_TOLERANCE || dy2<RS_TOLERANCE2) return false; //refuse to return zero size ellipse
        // y'= y
        // x'= x-y/tan
        // reverse scale
        // y=y'
        // x=x'+y' tan
        //
        double ia2=1./(dx*dx);
        double ib2=1./(minorP.y*minorP.y);
        //ellipse scaled:drawi
        // ia2*x'^2+ib2*y'^2=1
        // ia2*(x-y*minor.x/minor.y)^2+ib2*y^2=1
        // ia2*x^2 -2*ia2*minor.x/minor.y xy + ia2*minor.x^2*ib2 y^2 + ib2*y^2 =1
        dn[0]=ia2;
        dn[1]=-2.*ia2*minorP.x/minorP.y;
        dn[2]=ib2*ia2*minorP.x*minorP.x+ib2;
    }
        break;
    case 4:
        mt.pop_back(); //only 3 points needed to form the qudratic form
        if ( ! RS_Math::linearSolver(mt,dn) ) return false;
        break;
    default:
        RS_DEBUG->print(RS_Debug::D_WARNING,"No inscribed ellipse for non isosceles trapezoid");
        return false; //invalid quadrilateral
    }

    setCenter(center);
    if(! createFromQuadratic(dn)) return false;

    if(angleVector.valid) {//need to rotate back, for the parallelogram case
        angleVector.y *= -1.;
        rotate(center,angleVector);
    }
    return true;

}

/**
 * a naive implementation of middle point
 * to accurately locate the middle point from arc length is possible by using elliptic integral to find the total arc length, then, using elliptic function to find the half length point
 */
RS_Vector RS_Ellipse::getMiddlePoint()const{
    return getNearestMiddle(getCenter());
}
/**
  * get Nearest equidistant point
  *
  *Author: Dongxu Li
  */
RS_Vector RS_Ellipse::getNearestMiddle(const RS_Vector& coord,
                                       double* dist,
                                       int middlePoints
                                       ) const{
    RS_DEBUG->print("RS_Ellpse::getNearestMiddle(): begin\n");
    if ( ! isArc() ) {
        //no middle point for whole ellipse, angle1=angle2=0
        if (dist!=NULL) {
            *dist = RS_MAXDOUBLE;
        }
        return RS_Vector(false);
    }
    double ra(getMajorRadius());
    double rb(getRatio()*ra);
    if ( ra < RS_TOLERANCE || rb < RS_TOLERANCE ) {
        //zero radius, return the center
        RS_Vector vp(getCenter());
        if (dist!=NULL) {
            *dist = vp.distanceTo(coord);
        }
        return vp;
    }
    double amin=getCenter().angleTo(getStartpoint());
    double amax=getCenter().angleTo(getEndpoint());
    if(isReversed()) {
        std::swap(amin,amax);
    }
    double da=fmod(amax-amin+2.*M_PI, 2.*M_PI);
    if ( da < RS_TOLERANCE ) {
        da = 2.*M_PI; //whole ellipse
    }
    RS_Vector vp(getNearestPointOnEntity(coord,true,dist));
    double a=getCenter().angleTo(vp);
    int counts(middlePoints + 1);
    int i( static_cast<int>(fmod(a-amin+2.*M_PI,2.*M_PI)/da*counts+0.5));
    if(!i) i++; // remove end points
    if(i==counts) i--;
    a=amin + da*(double(i)/double(counts))-getAngle();
    vp.set(a);
    RS_Vector vp2(vp);
    vp2.scale( RS_Vector(1./ra,1./rb));
    vp.scale(1./vp2.magnitude());
    vp.rotate(getAngle());
    vp.move(getCenter());

    if (dist!=NULL) {
        *dist = vp.distanceTo(coord);
    }
    //RS_DEBUG->print("RS_Ellipse::getNearestMiddle: angle1=%g, angle2=%g, middle=%g\n",amin,amax,a);
    RS_DEBUG->print("RS_Ellpse::getNearestMiddle(): end\n");
    return vp;
}

/**
  * get the tangential point of a tangential line orthogonal to a given line
  *@ normal, the given line
  *@ onEntity, should the tangential be required to on entity of the elliptic arc
  *@ coord, current cursor position
  *
  *Author: Dongxu Li
  */

RS_Vector RS_Ellipse::getNearestOrthTan(const RS_Vector& coord,
                                        const RS_Line& normal,
                                        bool onEntity )
{
    if ( !coord.valid ) {
        return RS_Vector(false);
    }
    RS_Vector direction=normal.getEndpoint() - normal.getStartpoint();
	if (direction.squared()< RS_TOLERANCE15) {
        //undefined direction
        return RS_Vector(false);
    }
    //scale to ellipse angle
    RS_Vector aV(-getAngle());
    direction.rotate(aV);
    double angle=direction.scale(RS_Vector(1.,getRatio())).angle();
    double ra(getMajorRadius());
    direction.set(ra*cos(angle),getRatio()*ra*sin(angle));//relative to center
    QList<RS_Vector> sol;
    for(int i=0;i<2;i++){
        if(!onEntity ||
                RS_Math::isAngleBetween(angle,getAngle1(),getAngle2(),isReversed())) {
            if(i){
                sol.append(- direction);
            }else{
                sol.append(direction);
            }
        }
        angle=RS_Math::correctAngle(angle+M_PI);
    }
    if(sol.size()<1) return RS_Vector(false);
    aV.y*=-1.;
    for(int i=0;i<sol.size();i++) sol[i].rotate(aV);
    RS_Vector vp;
    switch(sol.count()) {
    case 0:
        return RS_Vector(false);
    case 2:
        if( RS_Vector::dotP(sol[1],coord-getCenter())>0.) {
            vp=sol[1];
            break;
        }
    default:
        vp=sol[0];
    }
    return getCenter() + vp;
}


void RS_Ellipse::move(const RS_Vector& offset) {
    data.center.move(offset);
    //calculateEndpoints();
    //    minV.move(offset);
    //    maxV.move(offset);
    moveBorders(offset);
}



void RS_Ellipse::rotate(const RS_Vector& center, const double& angle) {
    RS_Vector angleVector(angle);
    data.center.rotate(center, angleVector);
    data.majorP.rotate(angleVector);
    //calculateEndpoints();
    calculateBorders();
}
void RS_Ellipse::rotate(const RS_Vector& center, const RS_Vector& angleVector) {
    data.center.rotate(center, angleVector);
    data.majorP.rotate(angleVector);
    //calculateEndpoints();
    calculateBorders();
}

void RS_Ellipse::rotate( const double& angle) {//rotate around origin
    RS_Vector aV(angle);
    data.center.rotate(aV);
    data.majorP.rotate(aV);
    calculateBorders();
}
void RS_Ellipse::rotate( const RS_Vector& angleVector) {//rotate around origin
    data.center.rotate(angleVector);
    data.majorP.rotate(angleVector);
    //calculateEndpoints();
    calculateBorders();
}

/**
 * make sure angleLength() is not more than 2*M_PI
 */
void RS_Ellipse::correctAngles() {
        double *pa1= & data.angle1;
        double *pa2= & data.angle2;
        if (isReversed()) std::swap(pa1,pa2);
        *pa2 = *pa1 + fmod(*pa2 - *pa1, 2.*M_PI);
        if ( fabs(data.angle1 - data.angle2) < RS_TOLERANCE_ANGLE ) *pa2 += 2.*M_PI;
}

void RS_Ellipse::moveStartpoint(const RS_Vector& pos) {
    data.angle1 = getEllipseAngle(pos);
    //data.angle1 = data.center.angleTo(pos);
    //calculateEndpoints();
    correctAngles(); // make sure angleLength is no more than 2*M_PI
    calculateBorders();
}



void RS_Ellipse::moveEndpoint(const RS_Vector& pos) {
    data.angle2 = getEllipseAngle(pos);
    //data.angle2 = data.center.angleTo(pos);
    //calculateEndpoints();
    correctAngles(); // make sure angleLength is no more than 2*M_PI
    calculateBorders();
}


RS2::Ending RS_Ellipse::getTrimPoint(const RS_Vector& trimCoord,
                                     const RS_Vector& /*trimPoint*/) {

    //double angEl = getEllipseAngle(trimPoint);
    double angM = getEllipseAngle(trimCoord);
    if (RS_Math::getAngleDifference(angM, data.angle1,isReversed()) > RS_Math::getAngleDifference(data.angle2,angM,isReversed())) {
        return RS2::EndingStart;
    } else {
        return RS2::EndingEnd;
    }
}

RS_Vector RS_Ellipse::prepareTrim(const RS_Vector& trimCoord,
                                  const RS_VectorSolutions& trimSol) {
//special trimming for ellipse arc
        RS_DEBUG->print("RS_Ellipse::prepareTrim()");
    if( ! trimSol.hasValid() ) return (RS_Vector(false));
    if( trimSol.getNumber() == 1 ) return (trimSol.get(0));
    double am=getEllipseAngle(trimCoord);
    QList<double> ias;
    double ia(0.),ia2(0.);
    RS_Vector is,is2;
    for(int ii=0; ii<trimSol.getNumber(); ii++) { //find closest according ellipse angle
        ias.append(getEllipseAngle(trimSol.get(ii)));
        if( !ii ||  fabs( remainder( ias[ii] - am, 2*M_PI)) < fabs( remainder( ia -am, 2*M_PI)) ) {
            ia = ias[ii];
            is = trimSol.get(ii);
        }
    }
    std::sort(ias.begin(),ias.end());
    for(int ii=0; ii<trimSol.getNumber(); ii++) { //find segment to enclude trimCoord
        if ( ! RS_Math::isSameDirection(ia,ias[ii],RS_TOLERANCE)) continue;
        if( RS_Math::isAngleBetween(am,ias[(ii+trimSol.getNumber()-1)% trimSol.getNumber()],ia,false))  {
            ia2=ias[(ii+trimSol.getNumber()-1)% trimSol.getNumber()];
        } else {
            ia2=ias[(ii+1)% trimSol.getNumber()];
        }
        break;
    }
    for(int ii=0; ii<trimSol.getNumber(); ii++) { //find segment to enclude trimCoord
        if ( ! RS_Math::isSameDirection(ia2,getEllipseAngle(trimSol.get(ii)),RS_TOLERANCE)) continue;
        is2=trimSol.get(ii);
        break;
    }
    if(RS_Math::isSameDirection(getAngle1(),getAngle2(),RS_TOLERANCE_ANGLE)
            ||  RS_Math::isSameDirection(ia2,ia,RS_TOLERANCE) ) {
        //whole ellipse
        if( !RS_Math::isAngleBetween(am,ia,ia2,isReversed())) {
            std::swap(ia,ia2);
            std::swap(is,is2);
        }
        setAngle1(ia);
        setAngle2(ia2);
        double da1=fabs(remainder(getAngle1()-am,2*M_PI));
        double da2=fabs(remainder(getAngle2()-am,2*M_PI));
        if(da2<da1) {
            std::swap(is,is2);
        }

    } else {
        double dia=fabs(remainder(ia-am,2*M_PI));
        double dia2=fabs(remainder(ia2-am,2*M_PI));
        double ai_min=std::min(dia,dia2);
        double da1=fabs(remainder(getAngle1()-am,2*M_PI));
        double da2=fabs(remainder(getAngle2()-am,2*M_PI));
        double da_min=std::min(da1,da2);
        if( da_min < ai_min ) {
            //trimming one end of arc
            bool irev= RS_Math::isAngleBetween(am,ia2,ia, isReversed()) ;
            if ( RS_Math::isAngleBetween(ia,getAngle1(),getAngle2(), isReversed()) &&
                    RS_Math::isAngleBetween(ia2,getAngle1(),getAngle2(), isReversed()) ) { //
                if(irev) {
                    setAngle2(ia);
                    setAngle1(ia2);
                } else {
                    setAngle1(ia);
                    setAngle2(ia2);
                }
                da1=fabs(remainder(getAngle1()-am,2*M_PI));
                da2=fabs(remainder(getAngle2()-am,2*M_PI));
            }
            if( ((da1 < da2) && (RS_Math::isAngleBetween(ia2,ia,getAngle1(),isReversed()))) ||
                    ((da1 > da2) && (RS_Math::isAngleBetween(ia2,getAngle2(),ia,isReversed())))
              ) {
                std::swap(is,is2);
                //std::cout<<"reset: angle1="<<getAngle1()<<" angle2="<<getAngle2()<<" am="<< am<<" is="<<getEllipseAngle(is)<<" ia2="<<ia2<<std::endl;
            }
        } else {
            //choose intersection as new end
            if( dia > dia2) {
                std::swap(is,is2);
                std::swap(ia,ia2);
            }
            if(RS_Math::isAngleBetween(ia,getAngle1(),getAngle2(),isReversed())) {
                if(RS_Math::isAngleBetween(am,getAngle1(),ia,isReversed())) {
                    setAngle2(ia);
                } else {
                    setAngle1(ia);
                }
            }
        }
    }
    return is;
}

double RS_Ellipse::getEllipseAngle(const RS_Vector& pos) const {
    RS_Vector m = pos-data.center;
    m.rotate(-data.majorP.angle());
    m.x *= data.ratio;
    return m.angle();
}


/* Dongxu Li's Version, 19 Aug 2011
 * scale an ellipse
 * Find the eigen vactors and eigen values by optimization
 * original ellipse equation,
 * x= a cos t
 * y= b sin t
 * rotated by angle,
 *
 * x = a cos t cos (angle) - b sin t sin(angle)
 * y = a cos t sin (angle) + b sin t cos(angle)
 * scaled by ( kx, ky),
 * x *= kx
 * y *= ky
 * find the maximum and minimum of x^2 + y^2,
 */
void RS_Ellipse::scale(const RS_Vector& center, const RS_Vector& factor) {
    RS_Vector vpStart;
    RS_Vector vpEnd;
    if(isArc()){
        //only handle start/end points for ellipse arc
        vpStart=getStartpoint().scale(center,factor);
        vpEnd=getEndpoint().scale(center,factor);
    }
    data.center.scale(center, factor);
    RS_Vector vp1(getMajorP());
    double a(vp1.magnitude());
    if(a<RS_TOLERANCE) return; //ellipse too small
    vp1 *= 1./a;
    double ct=vp1.x;
    double ct2 = ct*ct; // cos^2 angle
    double st=vp1.y;
    double st2=1.0 - ct2; // sin^2 angle
    double kx2= factor.x * factor.x;
    double ky2= factor.y * factor.y;
//    double a=getMajorRadius();
    double b=getRatio()*a;
    double cA=0.5*a*a*(kx2*ct2+ky2*st2);
    double cB=0.5*b*b*(kx2*st2+ky2*ct2);
    double cC=a*b*ct*st*(ky2-kx2);
    if (factor.x < 0)
        setReversed(!isReversed());
    if (factor.y < 0)
        setReversed(!isReversed());
    RS_Vector vp(cA-cB,cC);
    vp1.set(a,b);
    vp1.scale(RS_Vector(0.5*vp.angle()));
    vp1.rotate(RS_Vector(ct,st));
    vp1.scale(factor);
    setMajorP(vp1);
    a=cA+cB;
    b=vp.magnitude();
    setRatio( sqrt((a - b)/(a + b) ));
    if( isArc() ) {
        //only reset start/end points for ellipse arcs, i.e., angle1 angle2 are not both zero
        setAngle1(getEllipseAngle(vpStart));
        setAngle2(getEllipseAngle(vpEnd));
    }
    correctAngles();//avoid extra 2.*M_PI in angles
    //calculateEndpoints();
    scaleBorders(center,factor);
// calculateBorders();

}


/**
 * is the Ellipse an Arc
 * @return false, if both angle1/angle2 are zero
 *
 *Author: Dongxu Li
 */
bool RS_Ellipse::isArc() const{
#ifndef EMU_C99
    using std::isnormal;
#endif
    return /*std::*/isnormal(getAngle1()) || /*std::*/isnormal(getAngle2());
}
/**
 * mirror by the axis of the line axisPoint1 and axisPoint2
 *
 *Author: Dongxu Li
 */
void RS_Ellipse::mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2) {
    RS_Vector center=getCenter();
    RS_Vector majorp = center + getMajorP();
    RS_Vector startpoint,endpoint;
    if( isArc() )  {
        startpoint = getStartpoint();
        endpoint = getEndpoint();
    }

    center.mirror(axisPoint1, axisPoint2);
    majorp.mirror(axisPoint1, axisPoint2);

    setCenter(center);
    setReversed(!isReversed());
    setMajorP(majorp - center);
    if( isArc() )  {
        //only reset start/end points for ellipse arcs, i.e., angle1 angle2 are not both zero
        startpoint.mirror(axisPoint1, axisPoint2);
        endpoint.mirror(axisPoint1, axisPoint2);
        setAngle1( getEllipseAngle(startpoint));
        setAngle2( getEllipseAngle(endpoint));
    }
    correctAngles();//avoid extra 2.*M_PI in angles
    calculateBorders();
}

/**
  * get direction1 and direction2
  * get the tangent pointing outside at end points
  *
  * Author: Dongxu Li
  */
//getDirection1 for start point
double RS_Ellipse::getDirection1() const {
    RS_Vector vp;
    if (isReversed()){
        vp.set(sin(getAngle1()), -getRatio()*cos(getAngle1()));
    } else {
        vp.set(-sin(getAngle1()), getRatio()*cos(getAngle1()));
    }
    return vp.angle()+getAngle();
}

//getDirection2 for end point
double RS_Ellipse::getDirection2() const {
    RS_Vector vp;
    if (isReversed()){
        vp.set(-sin(getAngle2()), getRatio()*cos(getAngle2()));
    } else {
        vp.set(sin(getAngle2()), -getRatio()*cos(getAngle2()));
    }
    return vp.angle()+getAngle();
}

void RS_Ellipse::moveRef(const RS_Vector& ref, const RS_Vector& offset) {
    if(isArc()){
        RS_Vector startpoint = getStartpoint();
        RS_Vector endpoint = getEndpoint();

        //    if (ref.distanceTo(startpoint)<1.0e-4) {
        if ((ref-startpoint).squared()<1.0e-8) {
            moveStartpoint(startpoint+offset);
            correctAngles();//avoid extra 2.*M_PI in angles
            return;
        }
        if ((ref-endpoint).squared()<1.0e-8) {
            moveEndpoint(endpoint+offset);
            correctAngles();//avoid extra 2.*M_PI in angles
            return;
        }
    }
    if ((ref-getCenter()).squared()<1.0e-8) {
        //move center
        setCenter(getCenter()+offset);
        return;
    }
    auto&& foci=getFoci();
    for(size_t i=0; i< 2 ; i++){
        if ((ref-foci.at(i)).squared()<1.0e-8) {
            auto&& focusNew=foci.at(i) + offset;
            //move focus
            auto&& center = getCenter() + offset*0.5;
            RS_Vector majorP;
            if(getMajorP().dotP( foci.at(i) - getCenter()) >= 0.){
                majorP = focusNew - center;
            }else{
                majorP = center - focusNew;
            }
            double d=getMajorP().magnitude();
            double c=0.5*focusNew.distanceTo(foci.at(1-i));
            double k=majorP.magnitude();
            if(k<RS_TOLERANCE2 || d < RS_TOLERANCE ||
                    c >= d - RS_TOLERANCE) return;
            //            DEBUG_HEADER();
            //            std::cout<<__FUNCTION__<<" : moving focus";
            majorP *= d/k;
            setCenter(center);
            setMajorP(majorP);
            setRatio(sqrt(d*d-c*c)/d);
            correctAngles();//avoid extra 2.*M_PI in angles
            return;
        }
    }

    //move major/minor points
    if ((ref-getMajorPoint()).squared()<1.0e-8) {
        RS_Vector majorP=getMajorP()+offset;
        double r=majorP.magnitude();
        if(r<RS_TOLERANCE) return;
        double ratio = getRatio()*getMajorRadius()/r;
        setMajorP(majorP);
        setRatio(ratio);
        return;
    }
    if ((ref-getMinorPoint()).squared()<1.0e-8) {
        RS_Vector minorP=getMinorPoint() + offset;
        double r2=getMajorP().squared();
        if(r2<RS_TOLERANCE2) return;
         RS_Vector projected= getCenter() +
                getMajorP()*getMajorP().dotP(minorP-getCenter())/r2;
        double r=(minorP - projected).magnitude();
        if(r<RS_TOLERANCE) return;
        double ratio = getRatio()*r/getMinorRadius();
        setRatio(ratio);
        return;
    }

}

/** whether the entity's bounding box intersects with visible portion of graphic view
//fix me, need to handle overlay container separately
*/
bool RS_Ellipse::isVisibleInWindow(RS_GraphicView* view) const
{
    RS_Vector vpMin(view->toGraph(0,view->getHeight()));
    RS_Vector vpMax(view->toGraph(view->getWidth(),0));
    //viewport
    QRectF visualRect(vpMin.x,vpMin.y,vpMax.x-vpMin.x, vpMax.y-vpMin.y);
    QPolygonF visualBox(visualRect);
    QVector<RS_Vector> vps;
    for(unsigned short i=0;i<4;i++){
        const QPointF& vp(visualBox.at(i));
        vps<<RS_Vector(vp.x(),vp.y());
    }
    //check for intersection points with viewport
    for(unsigned short i=0;i<4;i++){
        RS_Line line(NULL,RS_LineData(vps.at(i),vps.at((i+1)%4)));
        RS_Ellipse e0(NULL, getData());
        if( RS_Information::getIntersection(&e0, &line, true).size()>0) return true;
    }
    //is startpoint within viewport
    QRectF ellipseRect(minV.x, minV.y, maxV.x - minV.x, maxV.y - minV.y);
    return ellipseRect.intersects(visualRect);
}

/** return the equation of the entity
for quadratic,

return a vector contains:
m0 x^2 + m1 xy + m2 y^2 + m3 x + m4 y + m5 =0

for linear:
m0 x + m1 y + m2 =0
**/
LC_Quadratic RS_Ellipse::getQuadratic() const
{
    std::vector<double> ce(6,0.);
    ce[0]=data.majorP.squared();
    ce[2]= data.ratio*data.ratio*ce[0];
    if(ce[0]<RS_TOLERANCE2 || ce[2]<RS_TOLERANCE2){
        return LC_Quadratic();
    }
    ce[0]=1./ce[0];
    ce[2]=1./ce[2];
    ce[5]=-1.;
    LC_Quadratic ret(ce);
    ret.rotate(getAngle());
    ret.move(data.center);
    return ret;
}

/**
 * @brief areaLineIntegral, line integral for contour area calculation by Green's Theorem
 * Contour Area =\oint x dy
 * @return line integral \oint x dy along the entity
 * \oint x dy = Cx y + \frac{1}{4}((a^{2}+b^{2})sin(2a)cos^{2}(t)-ab(2sin^{2}(a)sin(2t)-2t-sin(2t)))
 */
double RS_Ellipse::areaLineIntegral() const
{
    const double a=getMajorRadius();
    const double b=getMinorRadius();
    if(!isArc())
        return M_PI*a*b;
    const double ab=a*b;
    const double r2=a*a+b*b;
    const double& cx=data.center.x;
    const double aE=getAngle();
    const double& a0=data.angle1;
    const double& a1=data.angle2;
    const double fStart=cx*getStartpoint().y+0.25*r2*sin(2.*aE)*cos(a0)*cos(a0)-0.25*ab*(2.*sin(aE)*sin(aE)*sin(2.*a0)-sin(2.*a0));
    const double fEnd=cx*getEndpoint().y+0.25*r2*sin(2.*aE)*cos(a1)*cos(a1)-0.25*ab*(2.*sin(aE)*sin(aE)*sin(2.*a1)-sin(2.*a1));
    return (isReversed()?fStart-fEnd:fEnd-fStart) + 0.5*ab*getAngleLength();
}

/**
 * @return Angle length in rad.
 */
double RS_Ellipse::getAngleLength() const {
    double ret;
    if (isReversed()) {
        ret= RS_Math::correctAngle(data.angle1-data.angle2);
    } else {
        ret= RS_Math::correctAngle(data.angle2-data.angle1);
    }
    if(ret<RS_TOLERANCE_ANGLE) ret=2.*M_PI;
    return ret;
}

/** find the visible part of the arc, and call drawVisible() to draw */
void RS_Ellipse::draw(RS_Painter* painter, RS_GraphicView* view, double& patternOffset) {
    if(isArc()==false){
        RS_Ellipse arc(*this);
        arc.setAngle2(2.*M_PI);
        arc.setReversed(false);
        arc.draw(painter,view,patternOffset);
        return;
    }
    //only draw the visible portion of line
    RS_Vector vpMin(view->toGraph(0,view->getHeight()));
    RS_Vector vpMax(view->toGraph(view->getWidth(),0));
    QPolygonF visualBox(QRectF(vpMin.x,vpMin.y,vpMax.x-vpMin.x, vpMax.y-vpMin.y));

    RS_Vector vpStart(isReversed()?getEndpoint():getStartpoint());
    RS_Vector vpEnd(isReversed()?getStartpoint():getEndpoint());

    QVector<RS_Vector> vertex(0);
    for(unsigned short i=0;i<4;i++){
        const QPointF& vp(visualBox.at(i));
        vertex<<RS_Vector(vp.x(),vp.y());
    }
    /** angles at cross points */
    QVector<double> crossPoints(0);

    double baseAngle=isReversed()?getAngle2():getAngle1();
    for(unsigned short i=0;i<4;i++){
        RS_Line line(NULL,RS_LineData(vertex.at(i),vertex.at((i+1)%4)));
        auto&& vpIts=RS_Information::getIntersection(
                    static_cast<RS_Entity*>(this), &line, true);
//    std::cout<<"vpIts.size()="<<vpIts.size()<<std::endl;
        if( vpIts.size()==0) continue;
        foreach(RS_Vector vp, vpIts.getVector()){
            auto&& ap1=getTangentDirection(vp).angle();
            auto&& ap2=line.getTangentDirection(vp).angle();
            //ignore tangent points, because the arc doesn't cross over
            if( fabs( remainder(ap2 - ap1, M_PI) ) < RS_TOLERANCE_ANGLE) continue;
            crossPoints.push_back(
                        RS_Math::getAngleDifference(baseAngle, getEllipseAngle(vp))
                        );
        }
    }
    if(vpStart.isInWindowOrdered(vpMin, vpMax)) crossPoints.push_back(0.);
    if(vpEnd.isInWindowOrdered(vpMin, vpMax)) crossPoints.push_back(
                RS_Math::getAngleDifference(baseAngle,isReversed()?getAngle1():getAngle2())
                );


    //sorting
    std::sort(crossPoints.begin(),crossPoints.end());
    //draw visible
//    DEBUG_HEADER();
//    std::cout<<"crossPoints.size()="<<crossPoints.size()<<std::endl;
    RS_Ellipse arc(*this);
    arc.setSelected(isSelected());
    arc.setPen(getPen());
    arc.setReversed(false);
    if( crossPoints.size() >= 2) {
        for(int i=0;i<crossPoints.size()-1;i+=2){
            arc.setAngle1(baseAngle+crossPoints[i]);
            arc.setAngle2(baseAngle+crossPoints[i+1]);
            arc.drawVisible(painter,view,patternOffset);
        }
        return;
    }
    //a workaround for buggy equation solver, can be removed, when line-ellipse equation solver is reliable
    if(isVisibleInWindow(view)){
        arc.drawVisible(painter,view,patternOffset);
    }
}

/** directly draw the arc, assuming the whole arc is within visible window */
void RS_Ellipse::drawVisible(RS_Painter* painter, RS_GraphicView* view, double& /*patternOffset*/) {
//    std::cout<<"RS_Ellipse::drawVisible(): begin\n";
//    std::cout<<*this<<std::endl;
    if (painter==NULL || view==NULL) {
        return;
    }

    //visible in grahic view
    if(isVisibleInWindow(view)==false) return;
    double ra(getMajorRadius()*view->getFactor().x);
    double rb(getRatio()*ra);
    if(rb<RS_TOLERANCE) {//ellipse too small
        painter->drawLine(view->toGui(minV),view->toGui(maxV));
        return;
    }
    double mAngle=getAngle();
    RS_Vector cp(view->toGui(getCenter()));
    if ( !isSelected() && (
             getPen().getLineType()==RS2::SolidLine ||
             view->getDrawingMode()==RS2::ModePreview)) {
        painter->drawEllipse(cp,
                             ra, rb,
                             mAngle,
                             getAngle1(), getAngle2(),
                             isReversed());
        return;
    }

    // Pattern:
    const RS_LineTypePattern* pat;
    if (isSelected()) {
        pat = &RS_LineTypePattern::patternSelected;
    } else {
        pat = view->getPattern(getPen().getLineType());
    }

    if (pat==NULL) {
        RS_DEBUG->print(RS_Debug::D_WARNING, "Invalid pattern for Ellipse");
        return;
    }

    // Pen to draw pattern is always solid:
    RS_Pen pen = painter->getPen();
    pen.setLineType(RS2::SolidLine);
    double a1(RS_Math::correctAngle(getAngle1()));
    double a2(RS_Math::correctAngle(getAngle2()));
    if (isReversed()) std::swap(a1,a2);
    if(a2 <a1+RS_TOLERANCE_ANGLE) a2 +=2.*M_PI;
    painter->setPen(pen);
    int i(0),j(0);
    double* ds = new double[pat->num>0?pat->num:0];
    if(pat->num>0){
        double dpmm=static_cast<RS_PainterQt*>(painter)->getDpmm();
        while( i<pat->num){
            ds[i]= dpmm * pat->pattern[i] ;//pattern length
            if(fabs(ds[i])<1.)
                ds[i]=(ds[i]>=0.)?1.:-1.;
            i++;
        }
        j=i;
    }else {
        delete[] ds;
        RS_DEBUG->print(RS_Debug::D_WARNING,"Invalid pattern when drawing ellipse");
        painter->drawEllipse(cp,
                             ra, rb,
                             mAngle,
                             a1,
                             a2,
                             false);
        return;
    }

    double curA(a1);
    double nextA;
    bool notDone(true);

    for(i=0;notDone;i=(i+1)%j) {//draw patterned ellipse

        nextA = curA + fabs(ds[i])/
                RS_Vector(ra*sin(curA),rb*cos(curA)).magnitude();
        if(nextA>a2){
            nextA=a2;
            notDone=false;
        }
        if (ds[i]>0.){
            painter->drawEllipse(cp,
                                 ra, rb,
                                 mAngle,
                                 curA,
                                 nextA,
                                 false);
        }

        curA=nextA;
    }

    delete[] ds;
}



/**
 * Dumps the point's data to stdout.
 */
std::ostream& operator << (std::ostream& os, const RS_Ellipse& a) {
    os << " Ellipse: " << a.data << "\n";
    return os;
}

