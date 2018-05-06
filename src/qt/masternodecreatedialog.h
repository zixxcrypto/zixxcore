// Copyright (c) 2017-2018 The ZIXX Team developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#ifndef ZIXX_QT_MASTERNODECREATEDIALOG_H
#define ZIXX_QT_MASTERNODECREATEDIALOG_H

#include "walletmodel.h"

#include <QDialog>
#include <QList>
#include <QtNetwork>

namespace Ui {
    class MasternodeCreateDialog;
}

QT_BEGIN_NAMESPACE
class QMenu;
QT_END_NAMESPACE

class MasternodeCreateDialog : public QDialog
{
    Q_OBJECT

public:
    const static int            REQUEST_TIMEOUT = 30000;
    const static std::string    MASTERNODE_FILE;

    explicit MasternodeCreateDialog(QWidget *parent = 0);
    ~MasternodeCreateDialog();
    void setModel(WalletModel* walletModel);

    // Internal types
private:
    enum MasternodeCreateDialogSteps
    {
        MCDS_STEP_IDLE = 0,
        MCDS_STEP_FUNDS_VALIDITY_CHECK_BEGIN,
        MCDS_STEP_FUNDS_VALIDITY_CHECK_DONE,
        MCDS_STEP_FUNDS_CONFIRMATION_CHECK_BEGIN,
        MCDS_STEP_FUNDS_CONFIRMATION_CHECK_DONE,
        MCDS_STEP_CONNECTIVITY_CHECK_BEGIN,
        MCDS_STEP_CONNECTIVITY_CHECK_PENDING,
        MCDS_STEP_CONNECTIVITY_CHECK_DONE,
        MCDS_STEP_LAST
    };

    enum MasternodeCreateDialogStepError
    {
        MCDSE_ERROR_NONE = 0,
        MCDSE_ERROR_NO_FUNDS,
        MCDSE_ERROR_FUNDS_NOT_CONFIRMED,
        MCDSE_ERROR_CONNECTIVITY_SERVER_ERROR,
        MCDSE_ERROR_CONNECTIVITY_DATA_TRANSFER_ERROR,
        MCDSE_ERROR_WRITE_FILE_ERROR,

        MCDSE_ERROR_USER_CANCELLED,
        MCDSE_ERROR_LAST
    };


    enum MasternodeCreateDialogUIElement
    {
        MCDUIE_NONE = 0,
        MCDUIE_FUNDS_AVAILABLE_CHECKBOX,
        MCDUIE_FUNDS_CONFIRMED_CHECKBOX,
        MCDUIE_CONNECTIVITY_OK_CHECKBOX,

        MCDUIE_LAST
    };

    struct MasternodeDialogData {
        std::string alias;
        std::string ip_port;
        std::string masternode_key;
        std::string collateral_tx_id;
        std::string rx_id;
        int         output_index;
    };

Q_SIGNALS:
    void close_transaction_popup(int);

private Q_SLOTS:
    void request_cancel_dialog();
    void on_btContinue_clicked();
    void connectivity_check_finished(QNetworkReply* reply);
    void new_transaction_received(const QModelIndex& parent, int start, int end);

private:
    void _initialize();
    void _reset();

    // Setting up the system.
    void _set_progress(enum MasternodeCreateDialogSteps state);
    void _set_error(MasternodeCreateDialogStepError error, const std::string& error_string = "", bool soft_error = false);

    // Automaton
    void _clear_automaton();
    bool _run_automaton();
    void _set_state(enum MasternodeCreateDialogSteps state)     { _current_state = state;       }
    bool _is_error_set()            const                       { return !(_last_error == MCDSE_ERROR_NONE || _soft_error);  }
    bool _should_stop_automaton()   const                       { return (_current_state == MCDS_STEP_LAST) || _is_error_set(); }
    bool _change_state_valid()      const                       { return _should_change_state;  }
    bool _is_running()              const                       { return _current_state != MCDS_STEP_IDLE && _current_state != MCDS_STEP_LAST; }
    void _set_change_state(bool state)                          { _should_change_state = state; }

    void _set_ui_element(enum MasternodeCreateDialogUIElement, bool state);
    void _set_transient_button_labels(bool state);

    // Collateral
    bool _create_collateral_transaction(SendCoinsRecipient &collateral, const QString& label);
    bool _send_collateral(SendCoinsRecipient& collateral, const QString& label);

    // API usage.
    bool _check_connectivity();
    bool _wait_for_response();

    bool _wait_for_transaction();

    void _display_success_message();
    void _display_error_message();

    // file creation
    std::string _masternode_create_key() const;
    bool        _write_masternode_file(const MasternodeDialogData &data);


private:
    QNetworkAccessManager                   _network_manager;
    Ui::MasternodeCreateDialog*             _ui;
    WalletModel*                            _wallet_model;
    QNetworkReply*                          _network_reply;
    uint32_t                                _port;
    MasternodeDialogData                    _masternode_data;
    enum MasternodeCreateDialogSteps        _current_state;
    const enum MasternodeCreateDialogSteps  _last_state;
    enum MasternodeCreateDialogStepError    _last_error;
    std::string                             _last_error_string;
    bool                                    _should_change_state;
    bool                                    _connectivity_ok;
    bool                                    _soft_error;

};

#endif // ZIXX_QT_MASTERNODECREATEDIALOG_H
