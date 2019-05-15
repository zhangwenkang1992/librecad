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


#include "rs_linetypepattern.h"



RS_LineTypePattern::RS_LineTypePattern(int count ...) {
    va_list ap;
    int i=0;
    totalLength=0.;
    num = count;

    va_start(ap, count);
    pattern = new double[num];
    for (i=0; i<count; i++) {
        pattern[i] = va_arg(ap, double);
        totalLength+=fabs(pattern[i]);
    }
    va_end(ap);
}

RS_LineTypePattern::~RS_LineTypePattern() {
    delete[] pattern;
}

//define all line patterns in pixels
const RS_LineTypePattern RS_LineTypePattern::patternSolidLine(1, 10.0);

const RS_LineTypePattern RS_LineTypePattern::patternDotLineTiny(2, 0.15, -1.);
const RS_LineTypePattern RS_LineTypePattern::patternDotLine(2, 0.2, -6.2);
const RS_LineTypePattern RS_LineTypePattern::patternDotLine2(2, 0.2, -3.1);
const RS_LineTypePattern RS_LineTypePattern::patternDotLineX2(2, 0.2, -12.4);

const RS_LineTypePattern RS_LineTypePattern::patternDashLineTiny(2, 2., -1.);
const RS_LineTypePattern RS_LineTypePattern::patternDashLine(2, 12.0, -6.0);
const RS_LineTypePattern RS_LineTypePattern::patternDashLine2(2, 6.0, -3.0);
const RS_LineTypePattern RS_LineTypePattern::patternDashLineX2(2, 24.0, -12.0);

const RS_LineTypePattern RS_LineTypePattern::patternDashDotLineTiny(4, 2., -0.7, 0.15, -2.);
const RS_LineTypePattern RS_LineTypePattern::patternDashDotLine(4, 12.0, -5., 0.2, -5.95);
const RS_LineTypePattern RS_LineTypePattern::patternDashDotLine2(4, 6.0, -2., 0.2, -2.);
const RS_LineTypePattern RS_LineTypePattern::patternDashDotLineX2(4, 24.0, -8., 0.2, -8.);

const RS_LineTypePattern RS_LineTypePattern::patternDivideLineTiny(6, 2., -0.7, 0.15, -0.7, 0.15, -0.7);
const RS_LineTypePattern RS_LineTypePattern::patternDivideLine(6, 12.0, -4.9, 0.2, -4.9, 0.2, -4.9);
const RS_LineTypePattern RS_LineTypePattern::patternDivideLine2(6, 6.0, -1.9, 0.2, -1.9, 0.2, -1.9);
const RS_LineTypePattern RS_LineTypePattern::patternDivideLineX2(6, 24.0, -8., 0.2, -8., 0.2, -8.);

const RS_LineTypePattern RS_LineTypePattern::patternCenterLineTiny(4, 5., -1., 1., -1.);
const RS_LineTypePattern RS_LineTypePattern::patternCenterLine(4, 32.0, -6.0, 6.0, -6.0);
const RS_LineTypePattern RS_LineTypePattern::patternCenterLine2(4, 16.0, -3.0, 3.0, -3.0);
const RS_LineTypePattern RS_LineTypePattern::patternCenterLineX2(4, 64.0, -12.0, 12.0, -12.0);

const RS_LineTypePattern RS_LineTypePattern::patternBorderLineTiny(6, 2., -1., 2., -1., 0.15, -1.);
const RS_LineTypePattern RS_LineTypePattern::patternBorderLine(6, 12.0, -4.0, 12.0, -4., 0.2, -4.);
const RS_LineTypePattern RS_LineTypePattern::patternBorderLine2(6, 6.0, -3.0, 6.0, -3., 0.2, -3.);
const RS_LineTypePattern RS_LineTypePattern::patternBorderLineX2(6, 24.0, -8.0, 24.0, -8., 0.2, -8.);

const RS_LineTypePattern RS_LineTypePattern::patternBlockLine(2, 0.5, -0.5);
const RS_LineTypePattern RS_LineTypePattern::patternSelected(2, 1.0, -3.0);
