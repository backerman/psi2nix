// Copyright © 2019 Brad Ackerman.
// Licensed under the MIT License (LICENSE.txt in this repository).

#include "psi2nix.hpp"
#include "ui_psi2nix.h"

Psi2Nix::Psi2Nix(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Psi2Nix)
{
    ui->setupUi(this);
}

Psi2Nix::~Psi2Nix()
{
    delete ui;
}

void Psi2Nix::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
