/****************************************************************
Copyright (c) 1998 Sandro Sigala <ssigala@globalnet.it>.
All rights reserved.

Permission to use, copy, modify, and distribute this software
and its documentation for any purpose and without fee is hereby
granted, provided that the above copyright notice appear in all
copies and that both that the copyright notice and this
permission notice and warranty disclaimer appear in supporting
documentation, and that the name of the author not be used in
advertising or publicity pertaining to distribution of the
software without specific, written prior permission.

The author disclaim all warranties with regard to this
software, including all implied warranties of merchantability
and fitness.  In no event shall the author be liable for any
special, indirect or consequential damages or any damages
whatsoever resulting from loss of use, data or profits, whether
in an action of contract, negligence or other tortious action,
arising out of or in connection with the use or performance of
this software.
****************************************************************/

#include "config.h"

#include <kapp.h>
#include <klocale.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "scoredialog.h"
#include <klocale.h>
#include <kconfig.h>

ScoreDialog::ScoreDialog(QWidget *parent, const char *oname)
        : QDialog(parent, oname, true)
{
	setCaption(i18n("High Scores"));

	KConfig *config = kapp->config();
	config->setGroup("High Score");

	int yd = 10;
	QLabel *label;
	label = new QLabel(i18n("Level"), this);
	label->setAutoResize(true);
	label->move(50, yd);
	label = new QLabel(i18n("Score"), this);
	label->setAutoResize(true);
	label->move(100, yd);
	label = new QLabel(i18n("Name"), this);
	label->setAutoResize(true);
	label->move(160, yd);
	yd += 20;

	QString s, name, level, score, num;
	for (int i = 1; i <= 10; ++i) {
		num.setNum(i);
		s = "Pos" + num + "Level";
		level = config->readEntry(s, "0");
		s = "Pos" + num + "Score";
		score = config->readEntry(s, "0");
		s = "Pos" + num + "Name";
		name = config->readEntry(s, i18n("Noname"));
		s = "#" + num;
		label = new QLabel(s, this);
		label->setAutoResize(true);
		label->move(10, yd);
		label = new QLabel(level, this);
		label->setAutoResize(true);
		label->move(50, yd);	
		label = new QLabel(score, this);
		label->setAutoResize(true);
		label->move(100, yd);
		label = new QLabel(name, this);
		label->setAutoResize(true);
		label->move(160, yd);
		yd += 20;
	}

	QPushButton *ok;
	ok = new QPushButton(i18n("OK"), this);
	ok->setGeometry(85, yd+20, 70, 30);
	ok->setDefault(true);
	connect(ok, SIGNAL(clicked()), SLOT(accept()));

	resize(250, 290);
}
