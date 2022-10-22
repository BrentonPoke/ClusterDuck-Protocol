#include "../MamaDuck.h"
#include "../MemoryFree.h"


int MamaDuck::setupWithDefaults(std::vector<byte> deviceId, String ssid, String password) {

    int err = Duck::setupWithDefaults(deviceId, ssid, password);
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    err = setupRadio();
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    std::string name(deviceId.begin(),deviceId.end());
    err = setupWifi(name.c_str());

    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    err = setupDns();
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    err = setupWebServer(false);
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    err = setupOTA();
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR setupWithDefaults rc = " + String(err));
        return err;
    }

    duckutils::getTimer().every(CDPCFG_MILLIS_ALIVE, imAlive);

    duckNet->loadChannel();

    return DUCK_ERR_NONE;
}

void MamaDuck::run() {
    Duck::logIfLowMemory();

    duckRadio.serviceInterruptFlags();

    handleOtaUpdate();
    if (DuckRadio::getReceiveFlag()) {
        handleReceivedPacket();
        rxPacket->reset();
    }
    processPortalRequest();
}

void MamaDuck::handleReceivedPacket() {

    std::vector<byte> data;
    bool relay = false;

    loginfo("====> handleReceivedPacket: START");

    int err = duckRadio.readReceivedData(&data);
    if (err != DUCK_ERR_NONE) {
        logerr("ERROR failed to get data from DuckRadio. rc = "+ String(err));
        return;
    }
    logdbg("Got data from radio, prepare for relay. size: "+ String(data.size()));

    relay = rxPacket->prepareForRelaying(&filter, data);
    if (relay) {
        //TODO: this callback is causing an issue, needs to be fixed for mamaduck to get packet data
        //recvDataCallback(rxPacket->getBuffer());
        loginfo("handleReceivedPacket: packet RELAY START");
        // NOTE:
        // Ducks will only handle received message one at a time, so there is a chance the
        // packet being sent below will never be received, especially if the cluster is small
        // there are not many alternative paths to reach other mama ducks that could relay the packet.

        CdpPacket packet = CdpPacket(rxPacket->getBuffer());

        //Check if Duck is desitination for this packet before relaying
        if (duckutils::isEqual(BROADCAST_DUID, packet.dduid)) {
            switch(packet.topic) {
                case reservedTopic::ping:
                    loginfo("ping received");
                    err = sendPong();
                    if (err != DUCK_ERR_NONE) {
                        logerr("ERROR failed to send pong message. rc = " + String(err));
                    }
                    return;
                    break;
                case reservedTopic::ack:
                    handleAck(packet);
                    //relay batch ack
                    err = duckRadio.relayPacket(rxPacket);
                    if (err != DUCK_ERR_NONE) {
                        logerr("====> ERROR handleReceivedPacket failed to relay. rc = " + String(err));
                    } else {
                        loginfo("handleReceivedPacket: packet RELAY DONE");
                    }
                    break;
                case reservedTopic::cmd:
                    loginfo("Command received");
                    handleCommand(packet);

                    err = duckRadio.relayPacket(rxPacket);
                    if (err != DUCK_ERR_NONE) {
                        logerr("====> ERROR handleReceivedPacket failed to relay. rc = " + String(err));
                    } else {
                        loginfo("handleReceivedPacket: packet RELAY DONE");
                    }
                    break;
                case reservedTopic::mbm:
                    packet.timeReceived = millis();
                    duckNet->addToMessageBoardBuffer(packet);
                    break;
                case topics::gchat:
                    packet.timeReceived = millis();
                    duckNet->addToChatBuffer(packet);
                    break;
                default:
                    err = duckRadio.relayPacket(rxPacket);
                    if (err != DUCK_ERR_NONE) {
                        logerr("====> ERROR handleReceivedPacket failed to relay. rc = " + String(err));
                    } else {
                        loginfo("handleReceivedPacket: packet RELAY DONE");
                    }
            }
        } else if(duckutils::isEqual(duid, packet.dduid)) { //Target device check
            std::vector<byte> dataPayload;
            byte num = 1;

            switch(packet.topic) {
                case topics::dcmd:
                    loginfo("Duck command received");
                    handleDuckCommand(packet);
                    break;
                case reservedTopic::cmd:
                    loginfo("Command received");

                    //Start send ack that command was received
                    dataPayload.push_back(num);

                    dataPayload.insert(dataPayload.end(), packet.sduid.begin(), packet.sduid.end());
                    dataPayload.insert(dataPayload.end(), packet.muid.begin(), packet.muid.end());

                    err = txPacket->prepareForSending(&filter, PAPADUCK_DUID,
                                                      DuckType::MAMA, reservedTopic::ack, dataPayload);
                    if (err != DUCK_ERR_NONE) {
                        logerr("ERROR handleReceivedPacket. Failed to prepare ack. Error: " +
                               String(err));
                    }

                    err = duckRadio.sendData(txPacket->getBuffer());
                    if (err == DUCK_ERR_NONE) {
                        filter.bloom_add(packet.muid.data(), MUID_LENGTH);
                    } else {
                        logerr("ERROR handleReceivedPacket. Failed to send ack. Error: " +
                               String(err));
                    }

                    //Handle Command
                    handleCommand(packet);

                    break;
                case reservedTopic::ack:
                    handleAck(packet);
                    break;
                case reservedTopic::mbm:
                    packet.timeReceived = millis();
                    duckNet->addToMessageBoardBuffer(packet);
                    break;
                case topics::gchat:
                    packet.timeReceived = millis();
                    duckNet->addToChatBuffer(packet);
                    break;
                case topics::pchat:{
                    packet.timeReceived = millis();
                    std::string session(packet.sduid.begin(), packet.sduid.end());

                    duckNet->createPrivateHistory(session);
                    duckNet->addToPrivateChatBuffer(packet, session);
                }
                    break;
                default:
                    err = duckRadio.relayPacket(rxPacket);
                    if (err != DUCK_ERR_NONE) {
                        logerr("====> ERROR handleReceivedPacket failed to relay. rc = " + String(err));
                    } else {
                        loginfo("handleReceivedPacket: packet RELAY DONE");
                    }
            }

        } else {
            err = duckRadio.relayPacket(rxPacket);
            if (err != DUCK_ERR_NONE) {
                logerr("====> ERROR handleReceivedPacket failed to relay. rc = " + String(err));
            } else {
                loginfo("handleReceivedPacket: packet RELAY DONE");
            }
        }

    }
}

void MamaDuck::handleCommand(const CdpPacket & packet) {
    int err;
    std::vector<byte> dataPayload;
    std::vector<byte> alive {'I','m',' ','a','l','i','v','e'};

    switch(packet.data[0]) {
        case 0:
            //Send health quack
            loginfo("Health request received");
            dataPayload.insert(dataPayload.end(), alive.begin(), alive.end());
            err = txPacket->prepareForSending(&filter, PAPADUCK_DUID,
                                              DuckType::MAMA, topics::health, dataPayload);
            if (err != DUCK_ERR_NONE) {
                logerr("ERROR handleReceivedPacket. Failed to prepare ack. Error: " +
                       String(err));
            }

            err = duckRadio.sendData(txPacket->getBuffer());
            if (err == DUCK_ERR_NONE) {
                CdpPacket healthPacket = CdpPacket(txPacket->getBuffer());
                filter.bloom_add(healthPacket.muid.data(), MUID_LENGTH);
            } else {
                logerr("ERROR handleReceivedPacket. Failed to send ack. Error: " +
                       String(err));
            }

            break;
        case 1:
            //Change wifi status
            if((char)packet.data[1] == '1') {
                loginfo("Command WiFi ON");
                WiFi.mode(WIFI_AP);

            } else if ((char)packet.data[1] == '0') {
                loginfo("Command WiFi OFF");
                WiFi.mode( WIFI_MODE_NULL );
            }

            break;
        default:
            logerr("Command not recognized");
    }

}

void MamaDuck::handleDuckCommand(const CdpPacket & packet) {
    loginfo("Doesn't do anything yet. But Duck Command was received.");
}

void MamaDuck::handleAck(const CdpPacket & packet) {
    //TODO: we're starting over...
    DynamicJsonDocument doc(400);
    DeserializationError err = deserializeMsgPack(doc,packet.data.data());

    switch(err.code()){
        case DeserializationError::Ok: {
            lastMessageAck = false;
            for(int i = 0; i < MAX_MUID_PER_ACK; i++) {
                if (lastMessageMuid == doc["pairs"][i]["muid"].as<std::string>()) {
                    loginfo("Message acked at time: "+ String((int)doc["txTime"]));
                    lastMessageAck = true;
                }
                if(lastMessageAck) {
                    break;
                }
            }
        }
        case DeserializationError::InvalidInput:
            logerr(err.c_str());
            break;
        case DeserializationError::NoMemory:
            logerr(err.c_str());
            break;
        default:
            logerr(err.c_str());
            break;

            // TODO[Rory Olsen: 2021-06-23]: The application may need to know about
            //   acks. I recommend a callback specifically for acks, or
            //   similar.
    }
}

bool MamaDuck::getDetectState() { return duckutils::getDetectState(); }
