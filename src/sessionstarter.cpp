/*
 * Copyright 2014-2016 Markus Prasser
 *
 * This file is part of Labcontrol.
 *
 *  Labcontrol is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Labcontrol is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Labcontrol.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>

#include <QDebug>

#include "sessionstarter.h"
#include "ui_sessionstarter.h"
#include "Lib/settings.h"

extern std::unique_ptr< lc::Settings > settings;

lc::SessionStarter::SessionStarter( QWidget *parent ) :
    QWidget{ parent },
    ui{ new Ui::SessionStarter }
{
    ui->setupUi( this );
    this->SetupWidgets();
}

lc::SessionStarter::~SessionStarter() {
    delete ui;
}

void lc::SessionStarter::GetNewDataTargetPath() {
    QFileDialog *file_dialog = new QFileDialog{ this };
    file_dialog->setFileMode( QFileDialog::Directory );
    file_dialog->setDirectory( QDir::homePath() );
    file_dialog->setOption( QFileDialog::ShowDirsOnly, true );
    file_dialog->setOption( QFileDialog::DontUseNativeDialog, true );
    if( file_dialog->exec() ) {
        QString file_name = file_dialog->selectedFiles().at( 0 );
        ui->CBDataTargetPath->addItem( file_name );
        ui->CBDataTargetPath->setCurrentText( file_name );
        ui->CBDataTargetPath->activated( file_name );
    }
    delete file_dialog;
}

void lc::SessionStarter::on_CBDataTargetPath_activated(const QString &arg1) {
    Q_UNUSED( arg1 );

    if ( ui->CBDataTargetPath->currentIndex() == 0 ) {
        emit NewDataTargetPathRequested();
        return;
    }
    ui->CBDataTargetPath->setStyleSheet( "" );
}

void lc::SessionStarter::on_CBReceiptsHeader_activated( const QString &argHeader ) {
    Q_UNUSED( argHeader );
    ui->CBReceiptsHeader->setStyleSheet( "" );
}

void lc::SessionStarter::on_CBzTreeVersion_activated( const QString &argVersion ) {
    Q_UNUSED( argVersion );
    ui->CBzTreeVersion->setStyleSheet( "" );
}

void lc::SessionStarter::on_ChBPrintanonymousreceipts_clicked( bool checked ) {
    // Enable or disable the corresponding widgets
    if ( checked ) {
        ui->LReplaceParticipantName->setEnabled( true );
        ui->CBReplaceParticipantName->setEnabled( true );
        ui->CBReplaceParticipantName->setStyleSheet( "background: cyan" );
    } else {
        ui->LReplaceParticipantName->setEnabled( false );
        ui->CBReplaceParticipantName->setEnabled( false );
        ui->CBReplaceParticipantName->setStyleSheet( "" );
    }
    ui->ChBPrintanonymousreceipts->setStyleSheet( "" );
}

void lc::SessionStarter::on_ChBReceiptsforLocalClients_clicked( bool argChecked ) {
    Q_UNUSED( argChecked );
    ui->ChBReceiptsforLocalClients->setStyleSheet( "" );
}

void lc::SessionStarter::on_PBStartzTree_clicked() {
    emit SessionRequested( ui->CBDataTargetPath->currentText(), ui->SBPort->value(),
                           ui->CBzTreeVersion->currentText(),
                           ui->ChBReceiptsforLocalClients->isChecked(),
                           ui->CBReplaceParticipantName->currentText(),
                           ui->CBReceiptsHeader->currentText() );
    close();
}

void lc::SessionStarter::on_SBPort_editingFinished() {
    ui->SBPort->setStyleSheet( "" );
    settings->SetChosenZTreePort( ui->SBPort->value() );
}

void lc::SessionStarter::SetupWidgets() {
    ui->SBPort->setValue( settings->GetChosenZTreePort() );

    // Fill the 'CBzTreeVersion' combobox with known entries from the lablib class
    ui->CBzTreeVersion->addItem( "NONE" );
    const QStringList &zTreeEntries = settings->installedZTreeVersions;
    if ( !zTreeEntries.isEmpty() ) {
        for ( const auto &zTreeVersionString : zTreeEntries ) {
            ui->CBzTreeVersion->addItem( zTreeVersionString );
        }
        ui->CBzTreeVersion->setCurrentIndex( 0 );
    } else {
        throw lcForbiddenCall{};
    }

    // Fill the 'CBReceipts' combobox with successfully detected LaTeX receipt headers
    if ( !settings->installedLaTeXHeaders.isEmpty() ) {
        if ( ( settings->installedLaTeXHeaders.count() == 1 )
             && ( settings->installedLaTeXHeaders.at(0) == "None found" ) ) {
            ui->GBReceipts->setEnabled( false );
        }
        ui->CBReceiptsHeader->addItems( settings->installedLaTeXHeaders );
        if ( settings->installedLaTeXHeaders.length() - 1 < settings->defaultReceiptIndex ) {
            QMessageBox::information( this, tr( "'default_receipt_index' to high" ),
                                      tr( "'default_receipt_index' was set to big. The combo box containing the receipt templates will default to the first entry." ) );
            qDebug() << "'default_receipt_index' was set to big."
                        " The combo box containing the receipt templates will default to the first entry.";
            ui->CBReceiptsHeader->setCurrentIndex( 0 );
        } else {
            ui->CBReceiptsHeader->setCurrentIndex( settings->defaultReceiptIndex );
        }
    }

    // Fill the 'CBDataTargetPath' combobox with some data target path examples
    ui->CBDataTargetPath->addItem( tr( "Set new path" ) );
    ui->CBDataTargetPath->addItem( QDir::homePath() );
    ui->CBDataTargetPath->addItem( QString{ QDir::homePath() + "/zTreeData" } );
    ui->CBDataTargetPath->setCurrentIndex( 2 );
    connect( this, &SessionStarter::NewDataTargetPathRequested,
             this, &SessionStarter::GetNewDataTargetPath );
}
