#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QUdpSocket>
#include <vector>
#include <map>

class PeerWidget;
class QVBoxLayout;

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
	void on_btn_add_peer_clicked();
	void log_it(const QString& string);
	void on_btn_get_alive_since_clicked();

private:
	Ui::Widget *ui;
	QWidget* peers_widget;
	QVBoxLayout* peers_layout;
	std::map<std::string, PeerWidget*> peers;
};
#endif // WIDGET_H
