#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <vector>
#include <map>

class Peer;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
	Q_OBJECT

public:
	Widget(QWidget *parent = nullptr);
	~Widget();

private slots:
	void on_btn_send_hello_clicked();
	void on_btn_get_peers_clicked();
	void on_btn_add_to_connected_clicked();
	void on_btn_remove_from_connected_clicked();
	void on_btn_add_peer_clicked();
	void on_btn_send_status_clicked();

	void log_it(const QString& string);
	void update_peer_connections_widget();

	void on_cmb_peers_currentIndexChanged(const QString &arg1);

	void on_btn_send_bye_clicked();

	void on_btn_get_alive_peers_clicked();

private:
	void parse_response(const QByteArray& response_raw);
	Peer* get_selected_peer();
	Ui::Widget *ui;
	std::map<std::string, Peer*> peers;
};
#endif // WIDGET_H
