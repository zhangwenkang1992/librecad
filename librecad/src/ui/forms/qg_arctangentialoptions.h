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
#ifndef QG_ARCTANGENTIALOPTIONS_H
#define QG_ARCTANGENTIALOPTIONS_H

#include "ui_qg_arctangentialoptions.h"
#include "rs_actiondrawarctangential.h"

class QG_ArcTangentialOptions : public QWidget, public Ui::QG_ArcTangentialOptions
{
    Q_OBJECT

public:
    QG_ArcTangentialOptions(QWidget* parent = 0, Qt::WindowFlags fl = 0);
    ~QG_ArcTangentialOptions();

public slots:
    virtual void setAction( RS_ActionInterface * a, bool update );
    virtual void updateRadius(const QString& s  );
    virtual void updateAngle(const QString& s  );
    virtual void updateByRadius(const bool br);


protected:
    RS_ActionDrawArcTangential* action;

protected slots:
    virtual void languageChange();

private slots:
    void on_leRadius_textEdited();

    void on_leAngle_textEdited();

    void on_rbRadius_clicked(bool checked);

    void on_rbAngle_clicked(bool checked);

private:
    void destroy();

};

#endif // QG_ARCTANGENTIALOPTIONS_H
