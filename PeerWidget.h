#ifndef PEERWIDGET_H
#define PEERWIDGET_H

#include <QWidget>
#include "Peer.h"

namespace Ui {
class PeerWidget;
}

class PeerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PeerWidget(std::string&& address, const std::string& server_address, QWidget *parent = nullptr);
	~PeerWidget();
	Peer* get_peer() noexcept {return peer;}

signals:
	void log_it(const QString& string);

private slots:
	void on_btn_send_hello_clicked();
	void on_btn_send_bye_clicked();
	void on_btn_send_status_clicked();
	void on_btn_get_peers_clicked();
	void update_elited_peers();

	void on_btn_add_peer_to_connections_clicked();

	void on_btn_remove_peers_from_connections_clicked();

private:
	Ui::PeerWidget *ui;
	Peer* peer;
};

#endif // PEERWIDGET_H
