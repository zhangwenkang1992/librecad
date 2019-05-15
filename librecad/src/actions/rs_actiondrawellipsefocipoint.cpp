/****************************************************************************
**
 * Draw ellipse by foci and a point on ellipse

Copyright (C) 2011 Dongxu Li (dongxuli2011@gmail.com)
Copyright (C) 2011 R. van Twisk (librecad@rvt.dds.nl)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**********************************************************************/

#include "rs_actiondrawellipsefocipoint.h"

#include <QAction>
#include "rs_dialogfactory.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_ellipse.h"

/**
 * Constructor.
 *
 */
RS_ActionDrawEllipseFociPoint::RS_ActionDrawEllipseFociPoint(
    RS_EntityContainer& container,
    RS_GraphicView& graphicView)
        :RS_PreviewActionInterface("Draw ellipse by foci and a point",
                           container, graphicView),
          focus1(false),
          focus2(false),
          point(false)
{//nothing

}



QAction* RS_ActionDrawEllipseFociPoint::createGUIAction(RS2::ActionType /*type*/, QObject* /*parent*/) {
    QAction* action;

    action = new QAction(tr("Ellipse &Foci Point"), NULL);
    action->setIcon(QIcon(":/extui/ellipsefocipoint.png"));
    return action;
}

void RS_ActionDrawEllipseFociPoint::init(int status) {
    RS_PreviewActionInterface::init(status);

    if (status==SetFocus1) {
        focus1.valid=false;
    }
}



void RS_ActionDrawEllipseFociPoint::trigger() {
    RS_PreviewActionInterface::trigger();


    RS_EllipseData ed(center,
                      major*d,
                      sqrt(d*d-c*c)/d,
                      0., 0.,false);
    RS_Ellipse* ellipse = new RS_Ellipse(container, ed);
    ellipse->setLayerToActive();
    ellipse->setPenToActive();

    container->addEntity(ellipse);

    // upd. undo list:
    if (document!=NULL) {
        document->startUndoCycle();
        document->addUndoable(ellipse);
        document->endUndoCycle();
    }

//    RS_Vector rz = graphicView->getRelativeZero();
    graphicView->moveRelativeZero(ellipse->getCenter());
    graphicView->redraw(RS2::RedrawDrawing);
    drawSnapper();

    setStatus(SetFocus1);

    RS_DEBUG->print("RS_ActionDrawEllipseFociPoint::trigger():"
                    " entity added: %d", ellipse->getId());
}



void RS_ActionDrawEllipseFociPoint::mouseMoveEvent(QMouseEvent* e) {
    RS_DEBUG->print("RS_ActionDrawEllipseFociPoint::mouseMoveEvent begin");

    RS_Vector mouse = snapPoint(e);

    switch (getStatus()) {


    case SetPoint:
        point=mouse;
        d=0.5*(focus1.distanceTo(point)+focus2.distanceTo(point));
        if (d > c+ RS_TOLERANCE) {
            deletePreview();
            RS_EllipseData ed(center,
                              major*d,
                              sqrt(d*d-c*c)/d,
                              0., 0.,false);
            preview->addEntity(new RS_Ellipse(preview, ed));
            drawPreview();
        }
        break;



    default:
        break;
    }

    RS_DEBUG->print("RS_ActionDrawEllipseFociPoint::mouseMoveEvent end");
}



void RS_ActionDrawEllipseFociPoint::mouseReleaseEvent(QMouseEvent* e) {
    // Proceed to next status
    if (e->button()==Qt::LeftButton) {
        RS_CoordinateEvent ce(snapPoint(e));
        coordinateEvent(&ce);
    }

    // Return to last status:
    else if (e->button()==Qt::RightButton) {
        deletePreview();
        init(getStatus()-1);
    }
}


void RS_ActionDrawEllipseFociPoint::coordinateEvent(RS_CoordinateEvent* e) {
    if (e==NULL) {
        return;
    }
    RS_Vector mouse = e->getCoordinate();

    switch (getStatus()) {
    case SetFocus1:
        graphicView->moveRelativeZero(mouse);
        focus1=mouse;
        setStatus(SetFocus2);
        break;

    case SetFocus2:
        c=0.5*focus1.distanceTo(mouse);
        if(c>RS_TOLERANCE){
            graphicView->moveRelativeZero(mouse);
            focus2=mouse;
            center=(focus1+focus2)*0.5;
            major=focus1-center;
            major /= c ;
            setStatus(SetPoint);
        }
        break;
    case SetPoint:
        point=mouse;
        d=0.5*(focus1.distanceTo(point)+focus2.distanceTo(point));
        if (d > c+ RS_TOLERANCE) {
            graphicView->moveRelativeZero(mouse);
            trigger();
        }
        break;


    default:
        break;
    }
}

//fixme, support command line

void RS_ActionDrawEllipseFociPoint::commandEvent(RS_CommandEvent* e) {
    QString cmd = e->getCommand().toLower();

    if (checkCommand("help", cmd)) {
        if (RS_DIALOGFACTORY!=NULL) {
            RS_DIALOGFACTORY->commandMessage(msgAvailableCommands()
                                             + getAvailableCommands().join(", "));
        }
        return;
    }

    switch (getStatus()) {
    //    case SetFocus1: {
    //            bool ok;
    //            double m = RS_Math::eval(c, &ok);
    //            if (ok==true) {
    //                ratio = m / major.magnitude();
    //                if (!isArc) {
    //                    trigger();
    //                } else {
    //                    setStatus(SetAngle1);
    //                }
    //            } else {
    //                if (RS_DIALOGFACTORY!=NULL) {
    //                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
    //                }
    //            }
    //        }
    //        break;

    //    case SetFocus2: {
    //            bool ok;
    //            double a = RS_Math::eval(c, &ok);
    //            if (ok==true) {
    //                angle1 = RS_Math::deg2rad(a);
    //                setStatus(SetAngle2);
    //            } else {
    //                if (RS_DIALOGFACTORY!=NULL) {
    //                    RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
    //                }
    //            }
    //        }
    //        break;

    case SetPoint: {
        bool ok;
        double a = RS_Math::eval(cmd, &ok);
        if (ok) {
            e->accept();
//            std::cout<<"e->isAccepted()="<<e->isAccepted()<<std::endl;
            d=0.5*fabs(a);
            if (d > c + RS_TOLERANCE) {
                trigger();
            }else{
                RS_DIALOGFACTORY->commandMessage(QString::number(fabs(a))+" is smaller than distance between foci");
            }
        } else {
            if (RS_DIALOGFACTORY!=NULL) {
                RS_DIALOGFACTORY->commandMessage(tr("Not a valid expression"));
            }
        }
    }
        break;

    default:
        break;
    }
}


QStringList RS_ActionDrawEllipseFociPoint::getAvailableCommands() {
    QStringList cmd;
    return cmd;
}



void RS_ActionDrawEllipseFociPoint::updateMouseButtonHints() {
    if (RS_DIALOGFACTORY!=NULL) {
        switch (getStatus()) {
        case SetFocus1:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify first focus of ellipse"),
                                                tr("Cancel"));
            break;

        case SetFocus2:
            RS_DIALOGFACTORY->updateMouseWidget(tr("Specify second focus of ellipse"),
                                                tr("Back"));
            break;

        case SetPoint:
            RS_DIALOGFACTORY->updateMouseWidget(
                tr("Specify a point on ellipse or total distance to foci"),
                tr("Back"));
            break;

        default:
            RS_DIALOGFACTORY->updateMouseWidget("", "");
            break;
        }
    }
}



void RS_ActionDrawEllipseFociPoint::updateMouseCursor() {
    graphicView->setMouseCursor(RS2::CadCursor);
}



//void RS_ActionDrawEllipseFociPoint::updateToolBar() {
//    if (RS_DIALOGFACTORY!=NULL) {
//        if (isFinished()) {
//            RS_DIALOGFACTORY->resetToolBar();
//        }
//    }
//}

// EOF