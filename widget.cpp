#include "widget.h"
#include "ui_widget.h"
#include "communication/Request_generated.h"
#include <QInputDialog>

#include "Peer.h"

using namespace Seeder;

Widget::Widget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::Widget)
{
	ui->setupUi(this);
}

Widget::~Widget()
{
	delete ui;
}

void Widget::on_btn_send_hello_clicked()
{
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->send_hello();
}

void Widget::on_btn_get_peers_clicked()
{
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->send_get_elited_peers();
}

void Widget::on_btn_add_to_connected_clicked()
{
	if (ui->lstw_available_nodes->currentItem() == nullptr)
		return;

	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->add_to_connection(ui->lstw_available_nodes->currentItem()->text().toStdString());
	update_peer_connections_widget();
}

void Widget::on_btn_remove_from_connected_clicked()
{
	const std::string selected_connection = ui->lstw_connected_nodes->currentItem()->text().toStdString();

	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->remove_from_connection(selected_connection);
	update_peer_connections_widget();
}

void Widget::on_btn_add_peer_clicked()
{
	const std::string server_ip = ui->led_server_ip->text().toStdString();
	const QString address = QInputDialog::getText(this, "Add Peer", "Enter (fake) IP address of the peer.");
	ui->cmb_peers->addItem(address);
	auto peer = new Peer(address.toStdString(), server_ip, this);
	connect(peer, &Peer::log_it, this, &Widget::log_it);
	connect(peer, &Peer::elited_peers_changed, this, &Widget::update_peer_connections_widget);
	peers.insert(std::make_pair(address.toStdString(), peer));
}

void Widget::log_it(const QString &string)
{
	ui->pte_logs->appendPlainText(string);
}

Peer* Widget::get_selected_peer()
{
	const std::string selected_address = ui->cmb_peers->currentText().toStdString();

	auto it = peers.find(selected_address);
	if (it == peers.end()) {
		log_it(QString("Peer with address %1 not found!").arg(selected_address.c_str()));
		return nullptr;
	}

	return it->second;
}

void Widget::update_peer_connections_widget()
{
	ui->lstw_connected_nodes->clear();

	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	const auto& connections = peer->get_connections();
	for (const auto& connection : connections)
		ui->lstw_connected_nodes->addItem(connection.c_str());

	ui->lstw_available_nodes->clear();

	const auto& elited_peers = peer->get_elited_peers();
	for (const auto& connection : elited_peers)
		ui->lstw_available_nodes->addItem(connection.c_str());
}

void Widget::on_btn_send_status_clicked()
{
	const QDateTime last_alive = ui->dte_last_alive->dateTime();
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->send_status(last_alive);
}

void Widget::on_cmb_peers_currentIndexChanged(const QString &address)
{
	const QDateTime last_alive = ui->dte_last_alive->dateTime();
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	update_peer_connections_widget();
	ui->lbl_connection->setText(QString("%1 connections").arg(address));
	ui->lbl_last_alive->setText(peer->get_last_alive().toString());
}

void Widget::on_btn_send_bye_clicked()
{
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->send_goodbye();
}

void Widget::on_btn_get_alive_peers_clicked()
{
	Peer* peer = get_selected_peer();
	if (peer == nullptr)
		return;

	peer->send_get_alive_peers(ui->dte_alive_since->dateTime().toSecsSinceEpoch());
}
