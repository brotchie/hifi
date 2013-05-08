//
//  main.cpp
//  eve
//
//  Created by Stephen Birarda on 4/22/13.
//  Copyright (c) 2013 High Fidelity, Inc. All rights reserved.
//

#include <sys/time.h>

#include <SharedUtil.h>
#include <AgentTypes.h>
#include <PacketHeaders.h>
#include <AgentList.h>
#include <AvatarData.h>
#include <AudioInjector.h>

const int EVE_AGENT_LISTEN_PORT = 55441;

const float RANDOM_POSITION_MAX_DIMENSION = 10.0f;

const float DATA_SEND_INTERVAL_MSECS = 15;
const float MIN_AUDIO_SEND_INTERVAL_SECS = 10;
const int MIN_ITERATIONS_BETWEEN_AUDIO_SENDS = (MIN_AUDIO_SEND_INTERVAL_SECS * 1000) / DATA_SEND_INTERVAL_MSECS;
const int MAX_AUDIO_SEND_INTERVAL_SECS = 15;
const float MAX_ITERATIONS_BETWEEN_AUDIO_SENDS = (MAX_AUDIO_SEND_INTERVAL_SECS * 1000) / DATA_SEND_INTERVAL_MSECS;

const int ITERATIONS_BEFORE_HAND_GRAB = 100;
const int HAND_GRAB_DURATION_ITERATIONS = 50;
const int HAND_TIMER_SLEEP_ITERATIONS = 50;

const float EVE_PELVIS_HEIGHT = 0.5f;

bool stopReceiveAgentDataThread;
bool injectAudioThreadRunning = false;

int TEMP_AUDIO_LISTEN_PORT = 55439;
UDPSocket audioSocket(TEMP_AUDIO_LISTEN_PORT);

void *receiveAgentData(void *args) {
    sockaddr senderAddress;
    ssize_t bytesReceived;
    unsigned char incomingPacket[MAX_PACKET_SIZE];
    
    AgentList* agentList = AgentList::getInstance();
    Agent* avatarMixer = NULL;
    
    while (!::stopReceiveAgentDataThread) {
        if (agentList->getAgentSocket().receive(&senderAddress, incomingPacket, &bytesReceived)) { 
            switch (incomingPacket[0]) {
                case PACKET_HEADER_BULK_AVATAR_DATA:
                    // this is the positional data for other agents
                    // eve doesn't care about this for now, so let's just update the receive time for the
                    // avatar mixer - this makes sure it won't be killed during silent agent removal
                    avatarMixer = agentList->soloAgentOfType(AGENT_TYPE_AVATAR_MIXER);
                    
                    if (avatarMixer) {
                        avatarMixer->setLastHeardMicrostamp(usecTimestampNow());
                    }
                    
                    break;
                default:
                    // have the agentList handle list of agents from DS, replies from other agents, etc.
                    agentList->processAgentData(&senderAddress, incomingPacket, bytesReceived);
                    break;
            }
        }
    }
    
    pthread_exit(0);
    return NULL;
}

void *injectAudio(void *args) {
    ::injectAudioThreadRunning = true;
    
    AudioInjector* eveAudioInjector = (AudioInjector *)args;
    
    // look for an audio mixer in our agent list
    Agent* audioMixer = AgentList::getInstance()->soloAgentOfType(AGENT_TYPE_AUDIO_MIXER);
    
    if (audioMixer) {
        // until the audio mixer is setup for ping-reply, activate the public socket if it's not active
        if (!audioMixer->getActiveSocket()) {
            audioMixer->activatePublicSocket();
        }
        
        // we have an active audio mixer we can send data to
        eveAudioInjector->injectAudio(&::audioSocket, audioMixer->getActiveSocket());
    }
    
    ::injectAudioThreadRunning = false;
    pthread_exit(0);
    return NULL;
}

int main(int argc, const char* argv[]) {
    // new seed for random audio sleep times
    srand(time(0));
    
    // create an AgentList instance to handle communication with other agents
    AgentList* agentList = AgentList::createInstance(AGENT_TYPE_AVATAR, EVE_AGENT_LISTEN_PORT);
    
    // start telling the domain server that we are alive
    agentList->startDomainServerCheckInThread();
    
    // start the agent list thread that will kill off agents when they stop talking
    agentList->startSilentAgentRemovalThread();
    
    // start the ping thread that hole punches to create an active connection to other agents
    agentList->startPingUnknownAgentsThread();
    
    pthread_t receiveAgentDataThread;
    pthread_create(&receiveAgentDataThread, NULL, receiveAgentData, NULL);
    
    // create an AvatarData object, "eve"
    AvatarData eve = AvatarData();
    
    // move eve away from the origin
    // pick a random point inside a 10x10 grid
    
    eve.setPosition(glm::vec3(randFloatInRange(0, RANDOM_POSITION_MAX_DIMENSION),
                              EVE_PELVIS_HEIGHT, // this should be the same as the avatar's pelvis standing height
                              randFloatInRange(0, RANDOM_POSITION_MAX_DIMENSION)));
    
    // face any instance of eve down the z-axis
    eve.setBodyYaw(0);
    
    // put her hand out so somebody can shake it
    eve.setHandPosition(glm::vec3(eve.getPosition()[0] - 0.2,
                                  0.5,
                                  eve.getPosition()[2] + 0.1));    
    // read eve's audio data
    AudioInjector eveAudioInjector("/etc/highfidelity/eve/resources/eve.raw");
    
    unsigned char broadcastPacket[MAX_PACKET_SIZE];
    broadcastPacket[0] = PACKET_HEADER_HEAD_DATA;
    
    timeval thisSend;
    double numMicrosecondsSleep = 0;
    
    int numIterationsLeftBeforeAudioSend = 0;
    pthread_t injectAudioThread;
    
    int handStateTimer = 0;

    while (true) {
        // update the thisSend timeval to the current time
        gettimeofday(&thisSend, NULL);
        
        // find the current avatar mixer
        Agent* avatarMixer = agentList->soloAgentOfType(AGENT_TYPE_AVATAR_MIXER);
        
        // make sure we actually have an avatar mixer with an active socket
        if (agentList->getOwnerID() != UNKNOWN_AGENT_ID && avatarMixer && avatarMixer->getActiveSocket() != NULL) {
            unsigned char* packetPosition = broadcastPacket + sizeof(PACKET_HEADER);
            packetPosition += packAgentId(packetPosition, agentList->getOwnerID());
            
            // use the getBroadcastData method in the AvatarData class to populate the broadcastPacket buffer
            packetPosition += eve.getBroadcastData(packetPosition);
            
            // use the UDPSocket instance attached to our agent list to send avatar data to mixer
            agentList->getAgentSocket().send(avatarMixer->getActiveSocket(), broadcastPacket, packetPosition - broadcastPacket);
        }        

        // temporarily disable Eve's audio sending until the file is actually available on EC2 box
        if (numIterationsLeftBeforeAudioSend == 0) {
            if (!::injectAudioThreadRunning) {
                pthread_create(&injectAudioThread, NULL, injectAudio, (void*) &eveAudioInjector);
                
                numIterationsLeftBeforeAudioSend = randIntInRange(MIN_ITERATIONS_BETWEEN_AUDIO_SENDS,
                                                                  MAX_ITERATIONS_BETWEEN_AUDIO_SENDS);
            }
        } else {
            numIterationsLeftBeforeAudioSend--;
        }
        
        // sleep for the correct amount of time to have data send be consistently timed
        if ((numMicrosecondsSleep = (DATA_SEND_INTERVAL_MSECS * 1000) - (usecTimestampNow() - usecTimestamp(&thisSend))) > 0) {
            usleep(numMicrosecondsSleep);
        }   
                                    
        // simulate the effect of pressing and un-pressing the mouse button/pad
        handStateTimer++;
        
        if (handStateTimer == ITERATIONS_BEFORE_HAND_GRAB) {
            eve.setHandState(1);
        } else if (handStateTimer == ITERATIONS_BEFORE_HAND_GRAB + HAND_GRAB_DURATION_ITERATIONS) {
            eve.setHandState(0);
        } else if (handStateTimer >= ITERATIONS_BEFORE_HAND_GRAB + HAND_GRAB_DURATION_ITERATIONS + HAND_TIMER_SLEEP_ITERATIONS) {
            handStateTimer = 0;
        }   
    }
    
    // stop the receive agent data thread
    stopReceiveAgentDataThread = true;
    pthread_join(receiveAgentDataThread, NULL);
    
    // stop the agent list's threads
    agentList->stopDomainServerCheckInThread();
    agentList->stopPingUnknownAgentsThread();
    agentList->stopSilentAgentRemovalThread();
}

