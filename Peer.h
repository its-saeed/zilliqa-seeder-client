#ifndef PEER_H
#define PEER_H

#include "communication/Response_generated.h"

#include <QObject>
#include <QUdpSocket>
#include <vector>

class Peer : public QObject
{
	Q_OBJECT
public:
	Peer(std::string&& address, const std::string& server_address, QObject* parent = nullptr);
	void send_hello();
	void get_peers();
	void send_status();
	void send_goodbye();
	void add_to_connection(const std::string& connection);
	void remove_from_connection(const std::string& connection);
	const std::vector<std::string>& get_connections() const noexcept;
	const std::vector<std::string>& get_elited_peers() const noexcept;

signals:
	void log_it(const QString& log);
	void elited_peers_changed();

private slots:
	void readPendingDatagrams();

private:
	void parse_response(const QByteArray &response_buffer);
	QString hello_result_to_string(Seeder::HelloRequestResult result);
	void write_data(const uint8_t* data, size_t length, uint16_t request_id);

	const std::string address;
	const std::string server_address;
	QUdpSocket socket;
	std::vector<std::string> connections;
	std::vector<std::string> elited_peers;
};

#endif // PEER_H
