#include "widget.h"
#include "ui_widget.h"

#include <QInputDialog>
#include <QVBoxLayout>
#include <QMessageBox>

#include "Peer.h"
#include "PeerWidget.h"
#include "communication/Request_generated.h"

using namespace Seeder;

Widget::Widget(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::Widget)
	, peers_widget(new QWidget)
	, peers_layout(new QVBoxLayout(peers_widget))
{
	ui->setupUi(this);
	ui->scrl_peers->setWidget(peers_widget);
}

Widget::~Widget()
{
	delete ui;
}

void Widget::on_btn_add_peer_clicked()
{
	const std::string server_ip = ui->led_server_ip->text().toStdString();
	const QString address = QInputDialog::getText(this, "Add Peer", "Enter (fake) IP address of the peer.");
	if (address.isEmpty())
		return;

	auto peer_widget = new PeerWidget(address.toStdString(), server_ip);
	connect(peer_widget, &PeerWidget::log_it, this, &Widget::log_it);
	peers.insert(std::make_pair(address.toStdString(), peer_widget));
	peers_layout->addWidget(peer_widget);
}

void Widget::log_it(const QString &string)
{
	ui->pte_logs->appendPlainText(string);
}

void Widget::on_btn_get_alive_since_clicked()
{
	if (peers.size() == 0)
	{
		QMessageBox::warning(this, "Alive peers", "Please add at least one peer");
		return;
	}

	Peer* peer = peers.begin()->second->get_peer();
	peer->send_get_alive_peers(ui->dte_alive_since->dateTime().toSecsSinceEpoch());
}
