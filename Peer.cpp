#include "Peer.h"

#include <QDataStream>
#include <QHostAddress>
#include <QNetworkDatagram>

#include "communication/Request_generated.h"
using namespace Seeder;

Peer::Peer(std::string &&address, const std::string &server_address, QObject *parent)
: QObject(parent)
, address(std::move(address))
, server_address(server_address)
{
	socket.bind(QHostAddress::LocalHost, 10000 + qrand() % 1000);
	connect(&socket, &QUdpSocket::readyRead,
			this, &Peer::readPendingDatagrams);
}

void Peer::readPendingDatagrams()
{
	QNetworkDatagram datagram = socket.receiveDatagram();
	log_it(QString("Received %1 bytes").arg(datagram.data().length()));
	parse_response(datagram.data());
}

void Peer::parse_response(const QByteArray &response_buffer)
{
	if (response_buffer.size() == 0)
		return;

	auto response = Seeder::GetResponse(response_buffer.data());
	log_it(QString("Got response with id %1, type: %2").arg(response->id()).arg(response->response_type()));

	switch (response->response_type())
	{
	case Seeder::ResponseType::ResponseType_HelloResponse:
	{
		auto hello_response = response->response_as_HelloResponse();
		log_it(QString("HELLO result: %1").arg(hello_result_to_string(hello_response->result())));
//		if (hello_response->result() == Seeder::HelloRequestResult_REGISTERED_SUCCESSFULLY) {
//			const int send_status_interval = hello_response->availability_interval();
//			timer->setInterval(send_status_interval * 1000);
//			timer->start();
//		}
		break;
	}
	case Seeder::ResponseType::ResponseType_GetElitedPeersResponse:
	{
		auto active_peers_response = response->response_as_GetElitedPeersResponse();
		auto active_peers = active_peers_response->active_peers();
		elited_peers.clear();

		log_it("Active Peers: ");
		for (size_t i = 0; i < active_peers->size(); ++i) {
			const std::string peer = active_peers->GetAsString(i)->str();
			if (peer == address)
				continue;

			log_it(QString("%1: %2").arg(i + 1).arg(peer.c_str()));
			elited_peers.push_back(peer);
		}
		emit elited_peers_changed();
		break;
	}

	case Seeder::ResponseType::ResponseType_GetAlivePeersResponse:
	{
		auto alive_peers_response = response->response_as_GetAlivePeersResponse();
		auto alive_peers = alive_peers_response->alive_peers();

		log_it("Alive Peers: ");
		for (size_t i = 0; i < alive_peers->size(); ++i) {
			const std::string peer = alive_peers->GetAsString(i)->str();

			log_it(QString("%1: %2").arg(i + 1).arg(peer.c_str()));
		}
		break;
	}

	default:
		break;
	}
}

QString Peer::hello_result_to_string(HelloRequestResult result)
{
	switch (result) {
	case HelloRequestResult_ALREADY_REGISTERED:
		return "Already registered";
	case HelloRequestResult_REGISTERED_SUCCESSFULLY:
		return "Successfully registered";
	default:
		break;
	}
	return "Unknown";
}

void Peer::write_data(const uint8_t *data, size_t length, uint16_t request_id)
{
	QByteArray byte_array;

	QByteArray temp;
	QDataStream data_stream(&temp, QIODevice::ReadWrite);
	data_stream << quint16(length);
	data_stream.writeRawData((const char*)data, length);
	int bytes_written = socket.writeDatagram(temp, QHostAddress(server_address.c_str()), 9099);
	log_it(QString("%1 bytes written, with ID %2 sent.").arg(bytes_written).arg(request_id));
}

void Peer::send_hello()
{
	const uint16_t request_id = qrand() & 0xffff;
	flatbuffers::FlatBufferBuilder builder(1024);
	auto address_string = builder.CreateString(address);
	auto req = CreateHelloRequest(builder, address_string);
	RequestBuilder request_builder(builder);
	request_builder.add_id(request_id);
	request_builder.add_request_type(RequestType_HelloRequest);
	request_builder.add_request(req.Union());
	auto orc = request_builder.Finish();
	builder.Finish(orc);
	const uint8_t* buffer = builder.GetBufferPointer();
	const size_t size = builder.GetSize();

	write_data(buffer, size, request_id);
}

void Peer::send_get_elited_peers()
{
	const uint16_t request_id = qrand() & 0xffff;
	flatbuffers::FlatBufferBuilder builder(1024);
	auto req = builder.CreateStruct(GetElitedPeersRequest(5));
	RequestBuilder request_builder(builder);
	request_builder.add_id(request_id);
	request_builder.add_request_type(RequestType_GetElitedPeersRequest);
	request_builder.add_request(req.Union());
	auto orc = request_builder.Finish();
	builder.Finish(orc);
	const uint8_t* buffer = builder.GetBufferPointer();
	const size_t size = builder.GetSize();

	write_data(buffer, size, request_id);
}

void Peer::send_get_alive_peers(time_t since)
{
	const uint16_t request_id = qrand() & 0xffff;
	flatbuffers::FlatBufferBuilder builder(1024);
	auto req = builder.CreateStruct(GetPeersByLastAliveRequest(5, since));
	RequestBuilder request_builder(builder);
	request_builder.add_id(request_id);
	request_builder.add_request_type(RequestType_GetPeersByLastAliveRequest);
	request_builder.add_request(req.Union());
	auto orc = request_builder.Finish();
	builder.Finish(orc);
	const uint8_t* buffer = builder.GetBufferPointer();
	const size_t size = builder.GetSize();

	write_data(buffer, size, request_id);
}

void Peer::send_status(const QDateTime& alive)
{
	this->last_alive = alive;

	const uint16_t request_id = qrand() & 0xffff;
	flatbuffers::FlatBufferBuilder builder(1000);

	std::vector<flatbuffers::Offset<flatbuffers::String>> active_connections;
	for (const auto& connection : connections) {
		active_connections.push_back(builder.CreateString(connection));
	}

	auto address_string = builder.CreateString(address);
	auto request = Seeder::CreatePeerStatusRequest(builder, address_string,
			builder.CreateVector(active_connections), last_alive.toSecsSinceEpoch());

	RequestBuilder request_builder(builder);
	request_builder.add_id(request_id);
	request_builder.add_request_type(RequestType_PeerStatusRequest);
	request_builder.add_request(request.Union());

	auto orc = request_builder.Finish();
	builder.Finish(orc);
	const uint8_t* buffer = builder.GetBufferPointer();
	const size_t size = builder.GetSize();

	write_data(buffer, size, request_id);
}

void Peer::send_goodbye()
{
	const uint16_t request_id = qrand() & 0xffff;
	flatbuffers::FlatBufferBuilder builder(1024);
	auto address_string = builder.CreateString(address);
	auto req = CreateByeRequest(builder, address_string);
	RequestBuilder request_builder(builder);
	request_builder.add_id(request_id);
	request_builder.add_request_type(RequestType_ByeRequest);
	request_builder.add_request(req.Union());
	auto orc = request_builder.Finish();
	builder.Finish(orc);
	const uint8_t* buffer = builder.GetBufferPointer();
	const size_t size = builder.GetSize();

	write_data(buffer, size, request_id);
}

void Peer::add_to_connection(const std::string &connection)
{
	connections.push_back(connection);
}

void Peer::remove_from_connection(const std::string &connection)
{
	std::vector<std::string>::iterator position = std::find(connections.begin(),connections.end(), connection);
	if (position != connections.end())
		connections.erase(position);
}

const std::vector<std::string> &Peer::get_connections() const noexcept
{
	return connections;
}

const std::vector<std::string> &Peer::get_elited_peers() const noexcept
{
	return elited_peers;
}
