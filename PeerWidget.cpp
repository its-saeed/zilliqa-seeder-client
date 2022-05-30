#include "PeerWidget.h"
#include "ui_PeerWidget.h"

PeerWidget::PeerWidget(std::string &&address, const std::string &server_address, QWidget *parent)
: QWidget(parent)
, ui(new Ui::PeerWidget)
, peer(new Peer(std::move(address), server_address, this))
{
	ui->setupUi(this);

	ui->lbl_registered->hide();
	ui->lbl_not_sync->hide();

	ui->lbl_client_name->setText(peer->get_address().c_str());

	connect(peer, &Peer::log_it, this, &PeerWidget::log_it);
	connect(peer, &Peer::elited_peers_changed, this, &PeerWidget::update_elited_peers);
	connect(peer, &Peer::registered, [this]() {
		ui->lbl_registered->show();
		ui->lbl_unregistered->hide();
	});
}

PeerWidget::~PeerWidget()
{
	delete ui;
}

void PeerWidget::on_btn_send_hello_clicked()
{
	peer->send_hello();
}

void PeerWidget::on_btn_send_bye_clicked()
{
	ui->lbl_registered->hide();
	ui->lbl_unregistered->show();
	peer->send_goodbye();
}

void PeerWidget::on_btn_send_status_clicked()
{
	const QDateTime last_alive = ui->dte_last_alive->dateTime();
	ui->lbl_last_alive->setText(last_alive.toString());
	peer->send_status(last_alive);
	ui->lbl_not_sync->hide();
}

void PeerWidget::on_btn_get_peers_clicked()
{
	peer->send_get_elited_peers();
}

void PeerWidget::update_elited_peers()
{
	ui->cmb_available_peers->clear();

	const auto& elited_peers = peer->get_elited_peers();
	for (const auto& connection : elited_peers)
		ui->cmb_available_peers->addItem(connection.c_str());
}

void PeerWidget::on_btn_add_peer_to_connections_clicked()
{
	if (ui->cmb_available_peers->count() == 0)
		return;

	peer->add_to_connection(ui->cmb_available_peers->currentText().toStdString());
	ui->cmb_connections->addItem(ui->cmb_available_peers->currentText());
	ui->lbl_not_sync->show();
	ui->lbl_connections->setText(QString("Connected to %1 peers").arg(ui->cmb_connections->count()));
}

void PeerWidget::on_btn_remove_peers_from_connections_clicked()
{
	if (ui->cmb_connections->count() == 0)
		return;

	const QString connection = ui->cmb_connections->currentText();
	peer->remove_from_connection(connection.toStdString());
	ui->cmb_connections->removeItem(ui->cmb_connections->findText(connection));
	ui->lbl_connections->setText(QString("Connected to %1 peers").arg(ui->cmb_connections->count()));
}
