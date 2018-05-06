// Copyright (c) 2011-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <QString>
#include <QtNetwork>
#include <QFile>

#include "masternodecreatedialog.h"
#include "ui_masternodecreatedialog.h"

#include "bitcoinunits.h"
#include "guiconstants.h"
#include "guiutil.h"
#include "optionsmodel.h"
#include "walletmodel.h"
#include "masternode.h"
#include "transactionrecord.h"
#include "transactiontablemodel.h"

#include "addresstablemodel.h"

const std::string MasternodeCreateDialog::MASTERNODE_FILE = "masternode.conf";

MasternodeCreateDialog::MasternodeCreateDialog(QWidget *parent)
    : QDialog(parent)
    , _ui(new Ui::MasternodeCreateDialog)
    , _wallet_model(nullptr)
    , _network_reply(nullptr)
    , _port(0)
    , _current_state(MCDS_STEP_IDLE)
    , _last_state(MCDS_STEP_LAST)
    , _last_error(MCDSE_ERROR_NONE)
    , _should_change_state(true)
    , _connectivity_ok(false)
    , _soft_error(false)

{
    _ui->setupUi(this);

    connect(_ui->btClose, SIGNAL(clicked()), this, SLOT(request_cancel_dialog()));

    // Make all the checkboxes inactive
    _initialize();
    _reset();
}

MasternodeCreateDialog::~MasternodeCreateDialog()
{
    if (_wallet_model)
    {
        disconnect(_wallet_model->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(new_transaction_received(QModelIndex,int,int)));
    }

    delete _ui;
    _ui = nullptr;
}

void MasternodeCreateDialog::setModel(WalletModel *walletModel)
{
    _wallet_model = walletModel;

    // Try to connect!
    if (_wallet_model)
    {
        connect(_wallet_model->getTransactionTableModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(new_transaction_received(QModelIndex,int,int)));
    }
}

void MasternodeCreateDialog::_initialize()
{
    _ui->progressBar->setMaximum(_last_state);

    const CChainParams& chainparams = Params();
    _port = chainparams.GetDefaultPort();
}

void MasternodeCreateDialog::_reset()
{
    // Disable and uncheck
    _ui->checkboxFundsAvailable->setEnabled(false);
    _ui->checkboxFundsConfirmed->setEnabled(false);
    _ui->checkboxIPAddressValid->setEnabled(false);

    _ui->checkboxFundsAvailable->setChecked(false);
    _ui->checkboxFundsConfirmed->setChecked(false);
    _ui->checkboxIPAddressValid->setChecked(false);

    //
    _ui->btContinue->setEnabled(true);

    //
    _ui->errorLabel->setText("");

    // Resetting the automata.
    _set_progress(MCDS_STEP_IDLE);
}

void MasternodeCreateDialog::_set_progress(enum MasternodeCreateDialog::MasternodeCreateDialogSteps state)
{
    _current_state = state;
    _ui->progressBar->setValue(_current_state % (_last_state + 1));
}

void MasternodeCreateDialog::_set_error(enum MasternodeCreateDialogStepError error, const std::string &error_string, bool soft_error)
{
    _last_error = error;

    _last_error_string.clear();
    _last_error_string = error_string;
    _ui->errorLabel->setText(QString::fromStdString(_last_error_string));

    _soft_error = soft_error;
}

void MasternodeCreateDialog::_clear_automaton()
{
    _current_state = MCDS_STEP_IDLE;
    _last_error = MCDSE_ERROR_NONE;
    _last_error_string.clear();
    _connectivity_ok = false;
    _soft_error = false;
}

bool MasternodeCreateDialog::_run_automaton()
{
    switch (_current_state)
    {
    // Clean up and such
    case MCDS_STEP_IDLE:
    {
        _set_error(MCDSE_ERROR_NONE, "");
    }
        break;

    case MCDS_STEP_FUNDS_VALIDITY_CHECK_BEGIN:
    {
        // First check
        CAmount balance = _wallet_model->getBalance();
        bool funds_available = balance > MASTERNODE_REQUIRED_COINS;
        if (!funds_available)
        {
            _set_progress(MCDS_STEP_LAST);
            _set_error(MCDSE_ERROR_NO_FUNDS, "No funds available! Ensure you have at least 1000 XZX.");
        }
        _set_progress(MCDS_STEP_FUNDS_VALIDITY_CHECK_DONE);
        _set_ui_element(MCDUIE_FUNDS_AVAILABLE_CHECKBOX, funds_available);
    }
        break;
    case MCDS_STEP_FUNDS_CONFIRMATION_CHECK_BEGIN:
        _set_ui_element(MCDUIE_FUNDS_CONFIRMED_CHECKBOX, true);
        break;
    case MCDS_STEP_CONNECTIVITY_CHECK_BEGIN:
        _set_ui_element(MCDUIE_CONNECTIVITY_OK_CHECKBOX, false);

        // Block state change until the callback is received.
        _set_change_state(false);
        _check_connectivity();
        _set_progress(MCDS_STEP_CONNECTIVITY_CHECK_PENDING);
        break;

    case MCDS_STEP_CONNECTIVITY_CHECK_PENDING:
    {
        bool request_successful = _wait_for_response();
        if (!request_successful)
        {
            _set_error(MCDSE_ERROR_CONNECTIVITY_SERVER_ERROR, "Unable to verify whether port is open!", true);
        }
    }
        break;

    case MCDS_STEP_CONNECTIVITY_CHECK_DONE:
        // Nothing to be done.
        break;

    default:
        break;
    }

    // Error checking flag
    if (!_is_error_set() && _change_state_valid())
    {
        _set_progress(static_cast<enum MasternodeCreateDialogSteps>(_current_state + 1));
    }

    return !_is_error_set();
}

void MasternodeCreateDialog::_set_ui_element(enum MasternodeCreateDialogUIElement elm, bool state)
{
    switch (elm)
    {
    case MCDUIE_FUNDS_AVAILABLE_CHECKBOX:
        _ui->checkboxFundsAvailable->setChecked(state);
        break;
    case MCDUIE_FUNDS_CONFIRMED_CHECKBOX:
        _ui->checkboxFundsConfirmed->setChecked(state);
        break;
    case MCDUIE_CONNECTIVITY_OK_CHECKBOX:
        _ui->checkboxIPAddressValid->setChecked(state);
        break;

        // Intentional fall through, as they're just control
    case MCDUIE_NONE:
    case MCDUIE_LAST:
    default:
        return;
    }
}

void MasternodeCreateDialog::_set_transient_button_labels(bool state)
{
    if (state)
    {
        _ui->btContinue->setText("Continue");
        _ui->btClose->setText("Close");
    }
    else
    {
        _ui->btContinue->setText("Please wait...");
        _ui->btClose->setText("Cancel");
    }

    _ui->btContinue->setEnabled(state);
}

bool MasternodeCreateDialog::_create_collateral_transaction(SendCoinsRecipient &recipient, const QString& label)
{
    bool valid = true;

    // Preparing the font.
    recipient.inputType = ALL_COINS;
    recipient.fUseInstantSend = false;

    // already unlocked or not encrypted at a*/ll
    valid = _send_collateral(recipient, label);

    return valid;
}

bool MasternodeCreateDialog::_send_collateral(SendCoinsRecipient &collateral, const QString &label)
{
    // prepare transaction for getting txFee earlierWalletModelTransaction
    QList<SendCoinsRecipient> recipients = QList<SendCoinsRecipient>() << collateral;
    WalletModelTransaction currentTransaction(recipients);
    WalletModel::SendCoinsReturn prepareStatus;

    prepareStatus = _wallet_model->prepareTransaction(currentTransaction, NULL);
    CAmount txFee = currentTransaction.getTransactionFee();

    // Format confirmation message
    QStringList formatted;
    Q_FOREACH(const SendCoinsRecipient &rcp, currentTransaction.getRecipients())
    {
        // generate bold amount string
        QString amount = "<b>" + BitcoinUnits::formatHtmlWithUnit(_wallet_model->getOptionsModel()->getDisplayUnit(), rcp.amount);

        // generate monospace address string
        QString address = "<span style='font-family: monospace;'>" + rcp.address;
        address.append("</span>");

        QString recipientElement;

        if (!rcp.paymentRequest.IsInitialized()) // normal payment
        {
            if(rcp.label.length() > 0) // label with address
            {
                recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.label));
                recipientElement.append(QString(" (%1)").arg(address));
            }
            else // just address
            {
                recipientElement = tr("%1 to %2").arg(amount, address);
            }
        }
        else if(!rcp.authenticatedMerchant.isEmpty()) // authenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, GUIUtil::HtmlEscape(rcp.authenticatedMerchant));
        }
        else // unauthenticated payment request
        {
            recipientElement = tr("%1 to %2").arg(amount, address);
        }

        formatted.append(recipientElement);
    }

    QString questionString = "You are ready to proceed with the masternode creation wizard. <br /> <br />"
                             "Confirming the following transaction will take you through the next steps of this process.<br />"
                             "<b>Warning: You will not be able to cancel this wizard after this stage! Proceed with caution.</b><br /><br />";


    questionString.append("Would you like to confirm the transaction below?<br />");
    questionString.append("The funds will be transferred to another address belonging to your account.");
    questionString.append("<br />%1");

    if(txFee > 0)
    {
        // append fee string if a fee is required
        questionString.append("<hr /><span style='color:#aa0000;'>");
        questionString.append(BitcoinUnits::formatHtmlWithUnit(_wallet_model->getOptionsModel()->getDisplayUnit(), txFee));
        questionString.append("</span> ");
        questionString.append(tr("are added as required transaction fee."));

        // append transaction size
        questionString.append(" (" + QString::number((double)currentTransaction.getTransactionSize() / 1000) + " kB)");
    }

    // add total amount in all subdivision units
    questionString.append("<hr /></span>");
    CAmount totalAmount = currentTransaction.getTotalTransactionAmount() + txFee;
    QStringList alternativeUnits;
    Q_FOREACH(BitcoinUnits::Unit u, BitcoinUnits::availableUnits())
    {
        if(u != _wallet_model->getOptionsModel()->getDisplayUnit())
            alternativeUnits.append(BitcoinUnits::formatHtmlWithUnit(u, totalAmount));
    }

    // Show total amount + all alternative units
    questionString.append(tr("The Total Amount is <b>%1</b><br />= %2")
                          .arg(BitcoinUnits::formatHtmlWithUnit(_wallet_model->getOptionsModel()->getDisplayUnit(), totalAmount))
                          .arg(alternativeUnits.join("<br />= ")));

    questionString.append("<hr />");

    QMessageBox msg_box(QMessageBox::Information,
                        tr("Confirm send coins"),
                        questionString.arg(formatted.join("<br />")),
                        QMessageBox::Yes | QMessageBox::Cancel,
                        this);

    // Nightmarish hack to get this running.
    msg_box.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
    msg_box.setStyleSheet("QLabel{min-width:600 px;}");

    if (msg_box.exec() != QMessageBox::Yes)
    {
        // Ok clicked
        return false;
    }

    // now send the prepared transaction
    WalletModel::SendCoinsReturn sendStatus = _wallet_model->sendCoins(currentTransaction);

    if (sendStatus.status == WalletModel::OK)
    {
        accept();
        return true;
    }

    // Boo!
    return false;
}

bool MasternodeCreateDialog::_check_connectivity()
{
    QObject::connect(&_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(connectivity_check_finished(QNetworkReply*)));

    //url parameters
    QString url_string = "https://api.zixx.org/connect/port/" + QString::number(_port);
    QUrl url(url_string);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/json"));

    _network_reply = _network_manager.get(request);
    return true;
}

bool MasternodeCreateDialog::_wait_for_response()
{
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(REQUEST_TIMEOUT);
    timer.start();

    QEventLoop loop;
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(_network_reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    disconnect(_network_reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));

    if(timer.isActive())
    {
        timer.stop();
        return true;
    }

    return false;
}

void MasternodeCreateDialog::request_cancel_dialog()
{
    _set_transient_button_labels(true);
    if (!_is_running() || _is_error_set())
    {
        this->reject();
    }

    _set_error(MCDSE_ERROR_USER_CANCELLED, "User requested cancellation. Aborted!");
}

void MasternodeCreateDialog::on_btContinue_clicked()
{
    bool success = false;
    // Wizard:
    //
    //    - Ask for a masternode name, check it is unique amounst the other alias (optional?)
    //    - generate a new receiving address using that alias, and send 1000 coins to it
    //    - generate a masternode genkey
    //          - masternode genkey
    //    - ask for (or detect automagically?) the valid ip of the windows machine
    //    - show a display of how many blocks have passed since the 1000 coins was sent. Once it hits 15 confirmations, go to the next stage
    //    - populate the masternode.conf file, restart the wallet, and start the node
    //      // "# Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index rewardaddress\n"
    //      // "# Example: mn1 10.0.0.1:44845 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0 ADD_REWARD_ADDRESS\n";

    //    - profit?
    if (!_wallet_model)
    {
        // Bad. Shouldn't happen, but it's still bad.
        return;
    }

    // Clear the FSM
    _clear_automaton();

    // Disable the button.
    _set_transient_button_labels(false);

    bool is_everything_ok = true;
    do
    {
        is_everything_ok = _run_automaton();
    }
    while (is_everything_ok && !_should_stop_automaton());


    // So an error has happened. Give up.
    if (!is_everything_ok)
    {
        _set_transient_button_labels(true);
        return;
    }

    // We now need to run the actual transfer process.
    QString address;
    QString label = QString("mnwiz-%1").arg(QDateTime::currentDateTimeUtc().toString("yyyyMMdd-hhmmss"));

    // Generate new received address
    address = _wallet_model->getAddressTableModel()->addRow(AddressTableModel::Receive, label, "");
    SendCoinsRecipient info(address, label, MASTERNODE_REQUIRED_COINS * COIN, QString("Hot Masternode creation wizard. Reference: (%1)").arg(label));

    success = _create_collateral_transaction(info, label);
    if (!success)
    {
        // Clear the FSM
        _clear_automaton();
        _ui->errorLabel->setText("Error sending funds to created address. Please verify you have enough XZX and try again!");
        return;
    }

    // Transaction is done. We need to generate the masternode key
    _masternode_data.alias = label.toStdString();
    _masternode_data.masternode_key = _masternode_create_key();
    _masternode_data.rx_id = address.toStdString();

    // Waiting until Transaction is confirmed.
    success = _wait_for_transaction();
    if (!success)
    {
        // Clear the FSM
        _clear_automaton();
        _ui->errorLabel->setText("You have cancelled the process of waiting for the transaction. Giving up!");
        return;
    }

    // Write to file.
    success = _write_masternode_file(_masternode_data);
    if (success)
    {
        _ui->errorLabel->setText("Masternode creation finished successfully!");
    }
    else
    {
        _ui->errorLabel->setText("Error writing masternode.conf file! Perhaps the permissions are incorrect?");
    }


    // We can wrap up!
    if (success)
    {
        _display_success_message();
    }
    else
    {
        _display_error_message();
    }

    _set_transient_button_labels(true);
    return;
}

void MasternodeCreateDialog::connectivity_check_finished(QNetworkReply *reply)
{
    QNetworkReply::NetworkError error = reply->error();
    if (error != QNetworkReply::NoError)
    {
        _network_manager.clearAccessCache();
        _set_error(MCDSE_ERROR_CONNECTIVITY_DATA_TRANSFER_ERROR, "Error processing connectivity request. Giving up");
    }
    else
    {
        bool success = true;
        //parse the reply JSON and display result in the UI
        QJsonObject jsonObject = QJsonDocument::fromJson(reply->readAll()).object();

        // Read the IP address connectivity here.
        if (jsonObject.contains("success") && jsonObject["success"].isBool())
        {
            success = jsonObject["success"].toBool(false);
        }

        if (jsonObject.contains("endpoint") && jsonObject["endpoint"].isString())
        {
            _masternode_data.ip_port = jsonObject["endpoint"].toString("").toStdString();
        }

#ifndef _DEBUG_MNCD_FORCE_SUCCESS
        if (!success)
        {
            _set_error(MCDSE_ERROR_CONNECTIVITY_DATA_TRANSFER_ERROR, "Error connecting to local port. Check your firewall!", true);
        }

        _set_change_state(true);
        _set_ui_element(MCDUIE_CONNECTIVITY_OK_CHECKBOX, success);
#else
        _set_change_state(true);
        _set_ui_element(MCDUIE_CONNECTIVITY_OK_CHECKBOX, true);
#endif
    }

    reply->deleteLater();
}

void MasternodeCreateDialog::new_transaction_received(const QModelIndex &parent, int start, int end)
{
    if (!_wallet_model)
    {
        return;
    }

    TransactionTableModel *ttm = _wallet_model->getTransactionTableModel();
    if (!ttm || ttm->processingQueuedTransactions())
    {
        return;
    }

    bool close = true;
    QModelIndex index = ttm->index(start, 0, parent);

    QString tx_id       = ttm->data(index, TransactionTableModel::TxIDRole).toString();
    int     tx_type     = ttm->data(index, TransactionTableModel::TypeRole).toInt();
    int     output_id   = ttm->data(index, TransactionTableModel::OutputIndexRole).toInt();
    CAmount amount      = ttm->index(start, TransactionTableModel::Credit, parent).data(Qt::EditRole).toULongLong();

    LogPrintf("%d\n", amount);

    // Verifying value
    close = close && (amount == MASTERNODE_REQUIRED_COINS * COIN);

    // Verifying self-payment
    close = close && (tx_type == TransactionRecord::SendToSelf);

    if (close)
    {
        // Storing the output id if OK
        _masternode_data.output_index       = output_id;
        _masternode_data.collateral_tx_id   = tx_id.toStdString();

        Q_EMIT close_transaction_popup(0);
    }
}


bool MasternodeCreateDialog::_wait_for_transaction()
{
    QMessageBox msg_box(QMessageBox::Information,
                        tr("Waiting for confirmation"),
                        "We're <b>almost</b> there! <br />"
                        "Please wait until your transaction is confirmed, so we can create your ZIXX Masternode.<br />"
                        "We promise it won't take too long!<br />"
                        "If you must cancel, the masternode.conf file will have to be created manually",
                        QMessageBox::Cancel,
                        this);

    // Nightmarish hack to get this running.
    msg_box.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
    msg_box.setStyleSheet("QLabel{min-width:600 px;}");

    connect(this, SIGNAL(close_transaction_popup(int)), &msg_box, SLOT(done(int)));
    int response = msg_box.exec();
    disconnect(this, SIGNAL(close_transaction_popup(int)), &msg_box, SLOT(done(int)));

    // Received transaction. Life's good.
    if (response == 0)
    {
        return true;
    }

    // Oh! :(
    return false;
}

void MasternodeCreateDialog::_display_success_message()
{
    QMessageBox msg_box(QMessageBox::Information,
                        tr("Success!"),
                        "Your hot masternode was created <b>successfully</b>.<br />"
                        "Restart your ZIXX wallet and verify its presence on the Masternode tab.",
                        QMessageBox::Ok,
                        this);

    // Nightmarish hack to get this running.
    msg_box.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
    msg_box.setStyleSheet("QLabel{min-width:600 px;}");
    msg_box.exec();
}

void MasternodeCreateDialog::_display_error_message()
{
    QMessageBox msg_box(QMessageBox::Information,
                        tr("Oh no!"),
                        "It seems that your hot Masternode could <b>not</b> be created.<br />"
                        "Your data has not been lost, however. You can still access the required fields and populate the file manually. <br />"
                        "If you need help, don't hesitate to contact the ZIXX team!",
                        QMessageBox::Ok,
                        this);

    // Nightmarish hack to get this running.
    msg_box.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding );
    msg_box.setStyleSheet("QLabel{min-width:600 px;}");
    msg_box.exec();

}

std::string MasternodeCreateDialog::_masternode_create_key() const
{
    CKey secret;
    secret.MakeNewKey(false);

    return CBitcoinSecret(secret).ToString();
}

bool MasternodeCreateDialog::_write_masternode_file(const MasternodeDialogData& data)
{
    boost::filesystem::path pathMasternodeConfigFile = GetMasternodeConfigFile();
    QFile file(QString::fromStdString(pathMasternodeConfigFile.string()));

    if (!file.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Append))
    {
        // Couldn't open. That's bad.
        _set_error(MCDSE_ERROR_WRITE_FILE_ERROR, "Unable to write to masternode.conf file. Cannot proceed!");
        return false;
    }

    // Format: alias IP:port masternodeprivkey collateral_output_txid collateral_output_index rewardaddress\n"
    // Example: mn1 10.0.0.1:44845 93HaYBVUCYjEMeeH1Y4sBGLALQZE1Yc1K64xiqgX37tGBDQL8Xg 2bcd3c84c84f87eaa86e4e56834c92927a07f9e18718810b92e0d0324456a67c 0\n";
    QTextStream out(&file);
    out << "\n"
        << QString::fromStdString(data.alias)               << " "
        << QString::fromStdString(data.ip_port)             << " "
        << QString::fromStdString(data.masternode_key)      << " "
        << QString::fromStdString(data.collateral_tx_id)    << " "
        << QString::number(data.output_index)               << " "
        << "\n";

    file.close();

    out.flush();

    return true;
}
