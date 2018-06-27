/*
    Copyright (C) 2011 Collabora Ltd. <info@collabora.co.uk>
      @author George Kiagiadakis <george.kiagiadakis@collabora.co.uk>
    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 2.1 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <iostream>
#include <QCoreApplication>
#include <QGlib/Error>
#include <QGlib/Connect>
#include <QGst/Bus>
#include <QGst/Init>
#include <QGst/Element>
#include <QGst/ElementFactory>
#include <QGst/Pipeline>
#include <QGst/Message>


class Player : public QCoreApplication
{
public:
    Player(int argc, char **argv);
    ~Player();
private:
    void onBusMessage(const QGst::MessagePtr & message);
private:
    QGst::PipelinePtr pipeline;
    QGst::ElementPtr source, sink;
    QGst::StateChangeReturn ret;
};


Player::Player(int argc, char **argv)
    : QCoreApplication(argc, argv)
{
    /* Initialize GStreamer */
    QGst::init(&argc, &argv);

    /* Create the elements */
    source = QGst::ElementFactory::make("videotestsrc", "source");
    sink = QGst::ElementFactory::make("autovideosink", "sink");

     /* Create the empty pipeline */
    pipeline = QGst::Pipeline::create("test-pipeline");

    if (pipeline.isNull() || source.isNull() || sink.isNull()) {
        std::cerr << "Not all elements could be created." << std::endl;
        std::exit(-1);
    }

    /* Build the pipeline */
    pipeline->add(source);
    pipeline->add(sink);

    if (!source->link(sink)) {
        std::cerr << "Elements could not be linked." << std::endl;
        std::exit(-1);
    }

    /* Modify the source's properties */
    source->setProperty("pattern", 0);

    /* Connects messages "GST_MESSAGE_ERROR" and "GST_MESSAGE_EOS" */
    QGlib::connect(pipeline->bus(), "message::error", this, &Player::onBusMessage);
    QGlib::connect(pipeline->bus(), "message::eos", this, &Player::onBusMessage);
    pipeline->bus()->addSignalWatch();

    /* Start playing */
    ret = pipeline->setState(QGst::StatePlaying);
    if (ret == QGst::StateChangeFailure) {
        std::cerr << "Unable to set the pipeline to the playing state." << std::endl;
        std::exit(-1);
    }
}


Player::~Player()
{
    pipeline->setState(QGst::StateNull);
}


void Player::onBusMessage(const QGst::MessagePtr & message)
{
    /* Parse message */
    switch (message->type()) {
    case QGst::MessageEos:
        qInfo() << "End-Of-Stream reached.";
        quit();
        break;
    case QGst::MessageError:
        qCritical() << "Error received:" << message.staticCast<QGst::ErrorMessage>()->error();
        break;
    default:
        qWarning() << "Unexpected message received.";
        break;
    }
}


int main(int argc, char **argv)
{
    Player p(argc, argv);
    return p.exec();
}
